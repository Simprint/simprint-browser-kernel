"""
将 overlay/branding/branding.config 中的定制项同步到 Chromium 的 args.gn。
out_dir 由环境 SIMPRINT_OUT_DIR 指定，不来自 branding.config。
"""
import os
import re
from pathlib import Path

GN_KEYS = ("chrome_executable_name", "is_simprint_branded")


def _parse_branding_config(path: Path) -> dict:
    """解析 branding.config：key = value，支持 # 注释。"""
    out = {}
    if not path.is_file():
        return out
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            m = re.match(r"^(\w+)\s*=\s*(.+)$", line)
            if not m:
                continue
            key, value = m.group(1), m.group(2).strip().strip("'\"").strip()
            if value.lower() == "true":
                value = True
            elif value.lower() == "false":
                value = False
            out[key] = value
    return out


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
    config_file = project_root / "branding.config"
    if not config_file.is_file():
        config_file = project_root / "branding.config.example"
    if not config_file.is_file():
        print("  No branding.config or branding.config.example, skip sync_gn_args.")
        return

    cfg = _parse_branding_config(config_file)
    to_set = {k: v for k, v in cfg.items() if k in GN_KEYS}
    if not to_set:
        print("  No branding gn keys in config, skip sync_gn_args.")
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
