"""
Branding 配置解析：key=value，支持 # 注释与 true/false。
供 deploy_resources、sync_gn_args、template_util 共用，避免重复实现。
"""
from pathlib import Path


def load_config(path: Path) -> dict:
    """解析单个配置文件，支持 # 注释，value 支持 true/false 转为 bool。"""
    out = {}
    if not path.is_file():
        return out
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line or "=" not in line:
                continue
            key, _, value = line.partition("=")
            key = key.strip()
            value = value.strip().strip("'\"").strip()
            if str(value).lower() == "true":
                value = True
            elif str(value).lower() == "false":
                value = False
            out[key] = value
    return out


def load_config_from_project_root(project_root: Path) -> dict:
    """在 project_root 下查找 branding.config 或 branding.config.example，解析并返回。"""
    for name in ("branding.config", "branding.config.example"):
        p = Path(project_root) / name
        if p.is_file():
            return load_config(p)
    return {}
