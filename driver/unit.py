"""Runtime for standard legacy overlay units.

Keeping this compatibility runner centralized prevents every capability from
carrying a private copy of patch/config/deploy behavior while it is migrated
behind the Core/Port boundary.
"""

from __future__ import annotations

import importlib.util
import os
import re
import sys
from pathlib import Path

from .patching import apply_patch


def _read_driver_config(kernel_root: Path, key: str) -> str | None:
    config_file = kernel_root / "driver" / "driver.config"
    if not config_file.is_file():
        return None
    with config_file.open(encoding="utf-8") as stream:
        for line in stream:
            match = re.match(rf"^\s*{re.escape(key)}\s*=\s*(.+)$", line)
            if match:
                return match.group(1).strip().strip("'\"").strip()
    return None


def _chromium_src(kernel_root: Path) -> Path:
    source = os.environ.get("SIMPRINT_CHROMIUM_ROOT") or _read_driver_config(
        kernel_root, "SIMPRINT_CHROMIUM_ROOT"
    )
    if not source:
        raise SystemExit(
            "Chromium root not set. Set SIMPRINT_CHROMIUM_ROOT or edit "
            "driver/driver.config"
        )
    path = Path(source).resolve()
    if not path.is_dir():
        raise SystemExit(f"Chromium root is not a directory: {path}")
    return path


def _read_order(path: Path) -> list[str]:
    if not path.is_file():
        return []
    result: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        value = line.split("#", 1)[0].strip()
        if value:
            result.append(value)
    return result


def _run_deploy_script(
    project_root: Path, chromium_src: Path, kernel_root: Path
) -> None:
    script = project_root / "scripts" / "deploy_resources.py"
    if not script.is_file():
        return
    spec = importlib.util.spec_from_file_location(
        f"simprint_deploy_{project_root.name}", script
    )
    if spec is None or spec.loader is None:
        raise SystemExit(f"Cannot load deploy script: {script}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    if hasattr(module, "run"):
        module.run(chromium_src, kernel_root, project_root)


def run_standard_unit(project_root: Path, phase: str) -> None:
    project_root = project_root.resolve()
    kernel_root = project_root.parent.parent
    chromium_src = _chromium_src(kernel_root)

    def apply() -> None:
        patches_dir = project_root / "patches"
        order = _read_order(project_root / "apply_order.txt")
        if not order and patches_dir.is_dir():
            order = sorted(path.name for path in patches_dir.glob("*.patch"))
        for name in order:
            patch_file = patches_dir / name
            if patch_file.is_file():
                apply_patch(
                    chromium_src,
                    patch_file,
                    f"overlay/{project_root.name}/{name}",
                )

    def deploy() -> None:
        configured_root = os.environ.get("SIMPRINT_KERNEL_ROOT")
        active_kernel_root = (
            Path(configured_root).resolve() if configured_root else kernel_root
        )
        _run_deploy_script(project_root, chromium_src, active_kernel_root)

    if phase == "apply":
        apply()
    elif phase == "deploy":
        deploy()
    elif phase == "apply_deploy":
        apply()
        deploy()
    elif phase == "build":
        return
    else:
        raise SystemExit(f"Unknown phase: {phase}")


def main(project_root: Path) -> None:
    if len(sys.argv) < 2:
        raise SystemExit("Usage: run.py <apply|deploy|apply_deploy|build>")
    run_standard_unit(project_root, sys.argv[1].lower())
