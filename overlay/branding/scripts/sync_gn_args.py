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


def _merge_and_write_args_gn(args_gn_path: Path, to_set: dict, key_pattern) -> None:
    """合并 to_set 到已有 args.gn，写回文件。"""
    existing = _read_args_gn(args_gn_path)
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


def run(chromium_src: Path, kernel_root: Path, project_root: Path) -> None:
    """读取 branding.config，合并到 Chromium args.gn。
    会同时写入 out/Default 与 out/Release，确保无论用哪个构建目录，VisualElements 等均使用 theme/simprint/（is_simprint_branded）。
    """
    cfg = load_config_from_project_root(project_root)
    to_set = {k: v for k, v in cfg.items() if k in GN_KEYS}
    if not to_set:
        print("  No branding.config or no gn keys in config, skip sync_gn_args.")
        return

    key_pattern = re.compile(r"^\s*(" + "|".join(re.escape(k) for k in GN_KEYS) + r")\s*=")
    out_dirs = ["out/Default", "out/Release"]
    env_out = os.environ.get("SIMPRINT_OUT_DIR", "").strip().strip("/")
    if env_out and env_out not in out_dirs:
        out_dirs.append(env_out)
    written = []
    for out_dir in out_dirs:
        args_gn_path = chromium_src / out_dir / "args.gn"
        _merge_and_write_args_gn(args_gn_path, to_set, key_pattern)
        written.append(out_dir)
    print("  Synced branding.config -> args.gn (%s)" % ", ".join(written))
