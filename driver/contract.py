"""Validate the branch-local Chromium port contract."""

from __future__ import annotations

import re
import subprocess
import tomllib
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Iterable

from .patching import normalize_unified_diff


PATCH_HEADER = re.compile(r"^diff --git a/(\S+) b/(\S+)$", re.MULTILINE)
FORBIDDEN_CORE_PREFIXES = (
    "chrome/",
    "content/",
    "net/",
    "services/",
    "third_party/blink/",
    "ui/",
)


@dataclass(frozen=True)
class Capability:
    id: str
    overlay: str
    state: str
    requires: tuple[str, ...]


@dataclass(frozen=True)
class PortManifest:
    schema_version: int
    chromium_version: str
    chromium_milestone: int
    core_api: int
    capabilities: tuple[Capability, ...]

    @property
    def overlay_order(self) -> tuple[str, ...]:
        return tuple(item.overlay for item in self.capabilities)


@dataclass
class ValidationReport:
    errors: list[str]
    warnings: list[str]
    patch_count: int
    overlapping_targets: dict[str, tuple[str, ...]]

    @property
    def ok(self) -> bool:
        return not self.errors


def _clean_lines(path: Path) -> list[str]:
    if not path.is_file():
        return []
    result: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        value = raw.split("#", 1)[0].strip()
        if value:
            result.append(value)
    return result


def load_manifest(repo_root: Path) -> PortManifest:
    manifest_path = repo_root / "port" / "manifest.toml"
    with manifest_path.open("rb") as stream:
        raw = tomllib.load(stream)

    port = raw["port"]
    capabilities = tuple(
        Capability(
            id=item["id"],
            overlay=item["overlay"],
            state=item["state"],
            requires=tuple(item.get("requires", [])),
        )
        for item in raw.get("capabilities", [])
    )
    return PortManifest(
        schema_version=raw["schema_version"],
        chromium_version=port["chromium_version"],
        chromium_milestone=port["chromium_milestone"],
        core_api=port["core_api"],
        capabilities=capabilities,
    )


def resolve_capabilities(
    manifest: PortManifest, requested: Iterable[str]
) -> tuple[Capability, ...]:
    by_id = {item.id: item for item in manifest.capabilities}
    selected: set[str] = set()
    visiting: set[str] = set()

    def visit(capability_id: str) -> None:
        if capability_id in selected:
            return
        if capability_id in visiting:
            raise ValueError(f"capability dependency cycle at {capability_id}")
        try:
            capability = by_id[capability_id]
        except KeyError as exc:
            raise ValueError(f"unknown capability: {capability_id}") from exc
        visiting.add(capability_id)
        for dependency in capability.requires:
            visit(dependency)
        visiting.remove(capability_id)
        selected.add(capability_id)

    for capability_id in requested:
        visit(capability_id)
    return tuple(item for item in manifest.capabilities if item.id in selected)


def _patch_targets_from_text(
    text: str, source_name: str
) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    targets: list[str] = []
    for source, target in PATCH_HEADER.findall(text):
        if source != target:
            errors.append(
                f"{source_name}: rename patches are not supported by the legacy runner: "
                f"{source} -> {target}"
            )
        path = PurePosixPath(target)
        if path.is_absolute() or ".." in path.parts:
            errors.append(f"{source_name}: unsafe patch target: {target}")
        targets.append(target)
    if not targets:
        errors.append(f"{source_name}: no unified diff headers found")
    return targets, errors


def _patch_targets(patch_file: Path) -> tuple[list[str], list[str]]:
    text = patch_file.read_text(encoding="utf-8", errors="replace")
    return _patch_targets_from_text(text, str(patch_file))


def _validate_patch_syntax(repo_root: Path, patch_file: Path) -> str | None:
    relative = patch_file.relative_to(repo_root)
    patch_text = normalize_unified_diff(
        patch_file.read_text(encoding="utf-8", errors="strict")
    )
    result = subprocess.run(
        ["git", "apply", "--recount", "--numstat", "-"],
        cwd=repo_root,
        input=patch_text,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if result.returncode == 0:
        return None
    detail = (result.stderr or result.stdout).strip()
    return f"{relative}: git apply could not parse patch: {detail}"


def _validate_core_boundary(repo_root: Path) -> list[str]:
    errors: list[str] = []
    core_root = repo_root / "core"
    if not core_root.is_dir():
        return ["core directory is missing"]
    for path in core_root.rglob("*"):
        if not path.is_file() or path.suffix not in {".h", ".cc", ".cpp", ".cxx"}:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for line_number, line in enumerate(text.splitlines(), start=1):
            stripped = line.strip()
            if not stripped.startswith("#include"):
                continue
            if any(f'"{prefix}' in stripped for prefix in FORBIDDEN_CORE_PREFIXES):
                relative = path.relative_to(repo_root)
                errors.append(
                    f"{relative}:{line_number}: Core must not include Chromium header: "
                    f"{stripped}"
                )
    return errors


def validate_port(repo_root: Path, check_patch_syntax: bool = True) -> ValidationReport:
    errors: list[str] = []
    warnings: list[str] = []
    manifest = load_manifest(repo_root)

    if manifest.schema_version != 1:
        errors.append(f"unsupported manifest schema: {manifest.schema_version}")
    if manifest.core_api != 1:
        errors.append(f"this driver supports Core API 1, got {manifest.core_api}")
    if manifest.chromium_milestone != int(manifest.chromium_version.split(".", 1)[0]):
        errors.append("chromium_milestone does not match chromium_version")

    ids = [item.id for item in manifest.capabilities]
    overlays = [item.overlay for item in manifest.capabilities]
    if len(ids) != len(set(ids)):
        errors.append("capability IDs must be unique")
    if len(overlays) != len(set(overlays)):
        errors.append("each overlay must map to exactly one capability")

    known_ids = set(ids)
    for capability in manifest.capabilities:
        unknown = set(capability.requires) - known_ids
        if unknown:
            errors.append(
                f"{capability.id}: unknown dependencies: {', '.join(sorted(unknown))}"
            )
    try:
        resolve_capabilities(manifest, ids)
    except ValueError as exc:
        errors.append(str(exc))

    driver_order = tuple(_clean_lines(repo_root / "driver" / "order.txt"))
    if driver_order != manifest.overlay_order:
        errors.append(
            "driver/order.txt must exactly match capability order in port/manifest.toml"
        )

    target_owners: dict[str, set[str]] = defaultdict(set)
    patch_count = 0
    for capability in manifest.capabilities:
        overlay_root = repo_root / "overlay" / capability.overlay
        if not overlay_root.is_dir():
            errors.append(f"{capability.id}: missing overlay/{capability.overlay}")
            continue
        if not (overlay_root / "run.py").is_file():
            errors.append(f"{capability.id}: overlay is missing run.py")

        patches_dir = overlay_root / "patches"
        ordered_names = _clean_lines(overlay_root / "apply_order.txt")
        actual_names = sorted(
            path.name
            for path in patches_dir.glob("*")
            if path.is_file() and path.name != ".gitkeep"
        ) if patches_dir.is_dir() else []
        if sorted(ordered_names) != actual_names:
            missing = sorted(set(ordered_names) - set(actual_names))
            unordered = sorted(set(actual_names) - set(ordered_names))
            if missing:
                errors.append(
                    f"{capability.id}: apply_order references missing files: "
                    f"{', '.join(missing)}"
                )
            if unordered:
                errors.append(
                    f"{capability.id}: patch files missing from apply_order: "
                    f"{', '.join(unordered)}"
                )

        for name in ordered_names:
            patch_file = patches_dir / name
            if not patch_file.is_file() or patch_file.suffix != ".patch":
                continue
            patch_count += 1
            targets, patch_errors = _patch_targets(patch_file)
            errors.extend(patch_errors)
            for target in targets:
                target_owners[target].add(capability.id)
            if check_patch_syntax:
                syntax_error = _validate_patch_syntax(repo_root, patch_file)
                if syntax_error:
                    errors.append(syntax_error)

    errors.extend(_validate_core_boundary(repo_root))
    overlaps = {
        target: tuple(sorted(owners))
        for target, owners in sorted(target_owners.items())
        if len(owners) > 1
    }
    for target, owners in overlaps.items():
        warnings.append(
            f"legacy overlap: {target} is modified by {', '.join(owners)}"
        )

    return ValidationReport(
        errors=errors,
        warnings=warnings,
        patch_count=patch_count,
        overlapping_targets=overlaps,
    )
