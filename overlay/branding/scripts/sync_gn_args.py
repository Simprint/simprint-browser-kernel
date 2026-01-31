"""
将 overlay/branding/branding.config 中的定制项同步到 Chromium 的 args.gn。
out_dir 由环境 SIMPRINT_OUT_DIR 指定，不来自 branding.config。
"""
import os
import re
import sys
from pathlib import Path

_SCRIPTS_DIR = Path(__file__).resolve().parent
if str(_SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(_SCRIPTS_DIR))
import config_loader

load_config_from_project_root = config_loader.load_config_from_project_root

GN_KEYS = ("chrome_executable_name", "is_simprint_branded")


def _format_gn_value(value) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    return '"%s"' % str(value).replace("\\", "\\\\").replace('"', '\\"')


def _read_args_gn(path: Path) -> list[str]:
    if not path.is_file():
        return []
    with open(path, encoding="utf-8") as f:
        return f.readlines()


def _write_args_gn(path: Path, lines: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.writelines(lines)


def run(chromium_src: Path, kernel_root: Path, project_root: Path) -> None:
    """读取 branding.config，合并到 Chromium args.gn。out_dir 来自环境 SIMPRINT_OUT_DIR，默认 out/Default。"""
    cfg = load_config_from_project_root(project_root)
    to_set = {k: v for k, v in cfg.items() if k in GN_KEYS}
    if not to_set:
        print("  No branding.config or no gn keys in config, skip sync_gn_args.")
        return

    out_dir = os.environ.get("SIMPRINT_OUT_DIR", "out/Default").strip("/")
    args_gn_path = chromium_src / out_dir / "args.gn"
    existing = _read_args_gn(args_gn_path)

    key_pattern = re.compile(r"^\s*(" + "|".join(re.escape(k) for k in GN_KEYS) + r")\s*=")
    new_lines = []
    for line in existing:
        if key_pattern.match(line.split("#")[0]):
            continue
        new_lines.append(line)

    for k in GN_KEYS:
        if k not in to_set:
            continue
        new_lines.append("%s = %s\n" % (k, _format_gn_value(to_set[k])))

    _write_args_gn(args_gn_path, new_lines)
    print("  Synced branding.config -> %s" % args_gn_path)
