"""
Integration 单元：与 Chromium 的接入层（最少 patch + 构建胶水）。
由 driver 调用：python integration/run.py <phase>
apply：打接入用 patch；deploy/build：当前无操作。
"""
import os
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent


def _chromium_src() -> Path:
    src = os.environ.get("SIMPRINT_CHROMIUM_ROOT")
    if not src:
        raise SystemExit("SIMPRINT_CHROMIUM_ROOT not set")
    return Path(src).resolve()


def _read_order(path: Path) -> list[str]:
    if not path.is_file():
        return []
    lines = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.split("#")[0].strip()
            if line:
                lines.append(line)
    return lines


def apply_() -> None:
    chromium_src = _chromium_src()
    patches_dir = PROJECT_ROOT / "patches"
    order_path = PROJECT_ROOT / "apply_order.txt"
    if not patches_dir.is_dir():
        return
    order = _read_order(order_path)
    if not order:
        order = sorted(p.name for p in patches_dir.glob("*.patch"))
    for name in order:
        patch_file = patches_dir / name
        if not patch_file.is_file():
            continue
        print(f"Applying: integration/{name}")
        r = subprocess.run(
            ["patch", "-p1"],
            cwd=chromium_src,
            stdin=patch_file.open("rb"),
            capture_output=True,
            text=False,
        )
        if r.returncode != 0:
            print(r.stderr.decode("utf-8", errors="replace"), file=sys.stderr)
            raise SystemExit(f"patch failed: {patch_file} (exit {r.returncode})")


def deploy() -> None:
    pass


def build() -> None:
    pass


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: run.py <apply|deploy|apply_deploy|build>", file=sys.stderr)
        raise SystemExit(1)
    phase = sys.argv[1].lower()
    if phase == "apply":
        apply_()
    elif phase == "deploy":
        deploy()
    elif phase == "apply_deploy":
        apply_()
        deploy()
    elif phase == "build":
        build()
    else:
        print(f"Unknown phase: {phase}", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()
