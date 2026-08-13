"""Unified-diff helpers shared by the legacy port and its validator."""

from __future__ import annotations

import subprocess
import sys
import re
from pathlib import Path


HUNK_HEADER = re.compile(
    r"^@@ -\d+(?:,(\d+))? \+\d+(?:,(\d+))? @@"
)


def normalize_unified_diff(text: str) -> str:
    """Normalize historical patch whitespace without changing its meaning.

    Some repository patches contain CRLF and blank context lines without the
    required leading space. GNU patch has historically been permissive here,
    while git apply and other tooling reject the same input. Normalization
    gives both the runtime and CI one deterministic representation.
    """

    lines = text.replace("\r\n", "\n").replace("\r", "\n").splitlines()
    in_hunk = False
    old_remaining = 0
    new_remaining = 0
    normalized: list[str] = []
    for line in lines:
        if line.startswith("diff --git ") or line.startswith("--- "):
            in_hunk = False
        header = HUNK_HEADER.match(line)
        if header:
            in_hunk = True
            old_remaining = int(header.group(1) or "1")
            new_remaining = int(header.group(2) or "1")
            normalized.append(line)
            continue
        if in_hunk and not line and old_remaining > 0 and new_remaining > 0:
            line = " "
        normalized.append(line)
        if not in_hunk or not line or line.startswith("\\"):
            continue
        if line[0] in {" ", "-"}:
            old_remaining -= 1
        if line[0] in {" ", "+"}:
            new_remaining -= 1
        if old_remaining <= 0 and new_remaining <= 0:
            in_hunk = False
    return "\n".join(normalized) + "\n"


def read_normalized_patch(path: Path) -> bytes:
    text = path.read_text(encoding="utf-8", errors="strict")
    return normalize_unified_diff(text).encode("utf-8")


def apply_patch(chromium_src: Path, patch_file: Path, label: str) -> None:
    print(f"Applying: {label}")
    result = subprocess.run(
        ["patch", "--batch", "--forward", "-p1"],
        cwd=chromium_src,
        input=read_normalized_patch(patch_file),
        capture_output=True,
    )
    if result.returncode == 0:
        return
    stdout = result.stdout.decode("utf-8", errors="replace")
    stderr = result.stderr.decode("utf-8", errors="replace")
    if stdout:
        print(stdout, file=sys.stderr)
    if stderr:
        print(stderr, file=sys.stderr)
    raise SystemExit(f"patch failed: {patch_file} (exit {result.returncode})")
