"""
模板工具：解析 key=value 配置，用 Jinja2 渲染模板。
供 overlay 单元在 apply 时使用：读 config → 渲染 .patch.j2 → 再应用。
"""
import sys
from pathlib import Path

from jinja2 import Environment, BaseLoader, select_autoescape

_SCRIPTS_DIR = Path(__file__).resolve().parent
if str(_SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(_SCRIPTS_DIR))
import config_loader

load_config = config_loader.load_config


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
