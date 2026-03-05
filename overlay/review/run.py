"""
Overlay Review 单元：chrome://review WebUI 页面与窗口增强功能
由 driver 调用：python overlay/review/run.py <phase>
环境变量或 driver/driver.config：SIMPRINT_CHROMIUM_ROOT, SIMPRINT_KERNEL_ROOT
"""
import os
import re
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent
KERNEL_ROOT = PROJECT_ROOT.parent.parent  # overlay/review -> kernel root


def _read_driver_config(key: str) -> str | None:
    """从 driver/driver.config 读取配置。"""
    config_file = KERNEL_ROOT / "driver" / "driver.config"
    if not config_file.is_file():
        return None
    with open(config_file, encoding="utf-8") as f:
        for line in f:
            m = re.match(rf"^\s*{re.escape(key)}\s*=\s*(.+)$", line)
            if m:
                return m.group(1).strip().strip("'\"").strip()
    return None


def _chromium_src() -> Path:
    src = os.environ.get("SIMPRINT_CHROMIUM_ROOT") or _read_driver_config("SIMPRINT_CHROMIUM_ROOT")
    if not src:
        raise SystemExit(
            "Chromium root not set. Set SIMPRINT_CHROMIUM_ROOT or edit driver/driver.config"
        )
    return Path(src).resolve()


def _kernel_root() -> Path:
    root = os.environ.get("SIMPRINT_KERNEL_ROOT") or str(KERNEL_ROOT)
    return Path(root).resolve()


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
        print(f"Applying: overlay/review/{name}")
        r = subprocess.run(
            ["patch", "-p1"],
            cwd=chromium_src,
            stdin=patch_file.open("rb"),
            capture_output=True,
            text=False,
        )
        if r.returncode != 0:
            out = r.stdout.decode("utf-8", errors="replace")
            if out:
                print(out, file=sys.stderr)
            err = r.stderr.decode("utf-8", errors="replace")
            if err:
                print(err, file=sys.stderr)
            raise SystemExit(f"patch failed: {patch_file} (exit {r.returncode})")


def _run_script(name: str, chromium_src: Path, kernel_root: Path) -> None:
    script = PROJECT_ROOT / "scripts" / (name + ".py")
    if not script.is_file():
        return
    import importlib.util
    spec = importlib.util.spec_from_file_location(name, script)
    if spec is None or spec.loader is None:
        return
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    if hasattr(mod, "run"):
        mod.run(chromium_src, kernel_root, PROJECT_ROOT)


def deploy() -> None:
    chromium_src = _chromium_src()
    kernel_root = _kernel_root()
    _run_script("deploy_resources", chromium_src, kernel_root)


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
