"""按 driver/order.txt 顺序调用 integration、overlay、projects 下各单元的 run.py <phase>。"""
import os
import subprocess
import sys
from pathlib import Path


def _read_order(repo_root: Path) -> list[str]:
    order_file = repo_root / "driver" / "order.txt"
    if not order_file.is_file():
        return []
    lines = []
    with open(order_file, encoding="utf-8") as f:
        for line in f:
            line = line.split("#")[0].strip()
            if line:
                lines.append(line)
    return lines


def _unit_dir(repo_root: Path, name: str) -> Path | None:
    """根据 order.txt 中的名称解析实际目录：integration、overlay/<name>、projects/<name>。"""
    if name == "integration":
        d = repo_root / "integration"
        return d if d.is_dir() else None
    overlay_d = repo_root / "overlay" / name
    if overlay_d.is_dir():
        return overlay_d
    projects_d = repo_root / "projects" / name
    return projects_d if projects_d.is_dir() else None


def run_phase(
    repo_root: Path,
    chromium_src: Path,
    phase: str,
    project_filter: list[str] | None = None,
    out_dir: str | None = None,
) -> None:
    """
    对 order.txt 中列出的每个单元执行 run.py <phase>。
    设置环境变量 SIMPRINT_CHROMIUM_ROOT, SIMPRINT_KERNEL_ROOT（仓库根）, SIMPRINT_OUT_DIR。
    """
    order = _read_order(repo_root)
    if not order:
        print("No units in driver/order.txt", file=sys.stderr)
        return
    if project_filter:
        order = [p for p in order if p in project_filter]
        if not order:
            print("No matching units for filter.", file=sys.stderr)
            return

    env = {
        **os.environ,
        "SIMPRINT_CHROMIUM_ROOT": str(chromium_src),
        "SIMPRINT_KERNEL_ROOT": str(repo_root),
        "SIMPRINT_OUT_DIR": (out_dir or os.environ.get("SIMPRINT_OUT_DIR") or "out/Default").strip("/"),
    }
    for name in order:
        unit_dir = _unit_dir(repo_root, name)
        if unit_dir is None:
            continue
        run_py = unit_dir / "run.py"
        if not run_py.is_file():
            continue
        print(f"[{name}] run.py {phase}")
        r = subprocess.run(
            [sys.executable, str(run_py), phase],
            cwd=str(repo_root),
            env=env,
        )
        if r.returncode != 0:
            raise SystemExit(f"Unit {name} {phase} failed (exit {r.returncode})")
