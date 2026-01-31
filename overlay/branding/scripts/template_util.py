"""
模板工具：解析 key=value 配置，用 Jinja2 渲染模板。
供 overlay 单元在 apply 时使用：读 config → 渲染 .patch.j2 → 再应用。
"""
from pathlib import Path

from jinja2 import Environment, BaseLoader, select_autoescape


def load_config(path: Path) -> dict:
    """解析 key = value 配置文件，支持 # 注释。"""
    out = {}
    if not path.is_file():
        return out
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            if "=" not in line:
                continue
            key, _, value = line.partition("=")
            key = key.strip()
            value = value.strip().strip("'\"").strip()
            if value.lower() == "true":
                value = True
            elif value.lower() == "false":
                value = False
            out[key] = value
    return out


def render_template(content: str, config: dict) -> str:
    """用 config 渲染 Jinja2 模板内容。"""
    env = Environment(
        loader=BaseLoader(),
        autoescape=select_autoescape(default=False),
        variable_start_string="{{",
        variable_end_string="}}",
    )
    t = env.from_string(content)
    return t.render(**config)
