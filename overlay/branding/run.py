"""
Overlay branding 单元：产品名、图标、主题等对 Chromium 源码的修改。
由 driver 调用：python overlay/branding/run.py <phase>
环境变量：SIMPRINT_CHROMIUM_ROOT, SIMPRINT_KERNEL_ROOT
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


def _kernel_root() -> Path:
    root = os.environ.get("SIMPRINT_KERNEL_ROOT")
    if not root:
        raise SystemExit("SIMPRINT_KERNEL_ROOT not set")
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


def _load_template_util():
    """动态加载 scripts/template_util.py，避免顶层 import jinja2 影响无模板场景。"""
    import importlib.util
    path = PROJECT_ROOT / "scripts" / "template_util.py"
    if not path.is_file():
        return None
    spec = importlib.util.spec_from_file_location("template_util", path)
    if spec is None or spec.loader is None:
        return None
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _load_branding_config() -> dict:
    """读 branding.config 或 branding.config.example。"""
    for name in ("branding.config", "branding.config.example"):
        p = PROJECT_ROOT / name
        if p.is_file():
            mod = _load_template_util()
            if mod is not None:
                return mod.load_config(p)
            # 无 template_util 时简单解析 key=value
            out = {}
            with open(p, encoding="utf-8") as f:
                for line in f:
                    line = line.split("#")[0].strip()
                    if not line or "=" not in line:
                        continue
                    k, _, v = line.partition("=")
                    out[k.strip()] = v.strip().strip("'\"").strip()
            return out
    return {}


def apply_() -> None:
    chromium_src = _chromium_src()
    patches_dir = PROJECT_ROOT / "patches"
    order_path = PROJECT_ROOT / "apply_order.txt"
    if not patches_dir.is_dir():
        return
    order = _read_order(order_path)
    if not order:
        order = sorted(
            p.name
            for p in list(patches_dir.glob("*.patch")) + list(patches_dir.glob("*.patch.j2")) + list(patches_dir.glob("*.py"))
        )
    config = _load_branding_config()
    template_util = _load_template_util()

    for name in order:
        patch_file = patches_dir / name
        if not patch_file.is_file():
            continue
        print(f"Applying: overlay/branding/{name}")
        if name.endswith(".py"):
            r = subprocess.run(
                [sys.executable, str(patch_file)],
                cwd=str(PROJECT_ROOT),
                env={**os.environ, "SIMPRINT_CHROMIUM_ROOT": str(chromium_src)},
            )
            if r.returncode != 0:
                raise SystemExit(f"script failed: {patch_file} (exit {r.returncode})")
            continue
        if name.endswith(".patch.j2"):
            if template_util is None:
                raise SystemExit("Template .patch.j2 requires jinja2 (uv sync).")
            raw = patch_file.read_text(encoding="utf-8").replace("\r\n", "\n").replace("\r", "\n")
            rendered = template_util.render_template(raw, config)
            # Normalize context lines: unified diff requires one leading space; fix "  X" -> " X"
            lines = rendered.split("\n")
            out_lines = []
            for line in lines:
                if len(line) >= 2 and line[0] == " " and line[1] == " " and line[2:3] and line[2] != " " and line[2] != "+" and line[2] != "-":
                    line = " " + line[2:]
                out_lines.append(line)
            rendered = "\n".join(out_lines)
            if not rendered.endswith("\n"):
                rendered += "\n"
            r = subprocess.run(
                ["patch", "-p1"],
                cwd=chromium_src,
                input=rendered.encode("utf-8"),
                capture_output=True,
            )
        else:
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
    _run_script("sync_gn_args", chromium_src, kernel_root)


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
