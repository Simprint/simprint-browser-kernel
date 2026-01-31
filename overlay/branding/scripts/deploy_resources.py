# Branding deploy: copy icons, tiles, strings, vector_icons so branded build works
# 优先从 overlay/branding/icons 复制；缺失时从 Chromium 对应路径复制
# 路径与文件名从 branding.config 的 branding_path_component 读取
from pathlib import Path
import shutil
import sys
from dataclasses import dataclass

_SCRIPTS_DIR = Path(__file__).resolve().parent
if str(_SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(_SCRIPTS_DIR))
import config_loader

load_config_from_project_root = config_loader.load_config_from_project_root

try:
    import template_util
except ImportError:
    template_util = None

# chrome_unscaled_resources.grd / theme_resources.grd 中 ${branding_path_component}/ 下的 product logo（与品牌名无关）
THEME_PRODUCT_LOGO_FILES = [
    "product_logo_64.png",
    "product_logo_128.png",
    "product_logo_256.png",
    "product_logo_16.png",
    "product_logo_22_mono.png",
    "product_logo_24.png",
    "product_logo_48.png",
    "product_logo.svg",
    "product_logo_animation.svg",
]


@dataclass
class DeployContext:
    """Deploy 阶段共用路径与品牌名，避免各函数重复传参。"""
    chromium_src: Path
    project_root: Path
    component: str
    theme: Path
    kernel_icons: Path
    chromium_win: Path
    dst_win: Path


def _icon_names_and_fallback(component: str):
    """根据 branding_path_component 生成 win 图标名与 Chromium 占位映射。"""
    return (
        [
            f"{component}.ico",
            "app_list.ico",
            "incognito.ico",
            f"{component}_doc.ico",
            f"{component}_pdf.ico",
        ],
        [
            ("chromium.ico", f"{component}.ico"),
            ("app_list.ico", "app_list.ico"),
            ("incognito.ico", "incognito.ico"),
            ("chromium_doc.ico", f"{component}_doc.ico"),
            ("chromium_pdf.ico", f"{component}_pdf.ico"),
        ],
    )


def _deploy_win_icons(ctx: DeployContext, icon_names: list, fallback: list) -> None:
    ctx.dst_win.mkdir(parents=True, exist_ok=True)
    for dst_name in icon_names:
        dst_file = ctx.dst_win / dst_name
        src_file = ctx.kernel_icons / dst_name
        if src_file.is_file():
            shutil.copy2(src_file, dst_file)
            print("  %s (from overlay/branding/icons)" % dst_name)
            continue
        src_name = next((s for s, d in fallback if d == dst_name), None)
        if src_name and (ctx.chromium_win / src_name).is_file():
            shutil.copy2(ctx.chromium_win / src_name, dst_file)
            print("  %s (from chromium/win)" % dst_name)
        else:
            print("  Warning: missing %s (skipped)" % dst_name)


def _deploy_theme_product_logos(ctx: DeployContext) -> None:
    """将 theme/chromium/ 下的 product logo 复制到 theme/{component}/。"""
    src_dir = ctx.theme / "chromium"
    dst_dir = ctx.theme / ctx.component
    dst_dir.mkdir(parents=True, exist_ok=True)
    for name in THEME_PRODUCT_LOGO_FILES:
        src = src_dir / name
        dst = dst_dir / name
        if src.is_file():
            shutil.copy2(src, dst)
            print("  theme/%s/%s (from chromium)" % (ctx.component, name))
        else:
            print("  Warning: missing theme/chromium/%s (skipped)" % name)


def _deploy_theme_scaled_resources(ctx: DeployContext) -> None:
    """将 default_100_percent/chromium、default_200_percent/chromium 整目录复制为 default_*_percent/{component}，供 theme_resources.grd 使用。"""
    for scale_dir in ("default_100_percent", "default_200_percent"):
        src = ctx.theme / scale_dir / "chromium"
        dst = ctx.theme / scale_dir / ctx.component
        if not src.is_dir():
            print("  Warning: theme/%s/chromium not found (skipped)" % scale_dir)
            continue
        for f in src.rglob("*"):
            if f.is_file():
                rel = f.relative_to(src)
                d = dst / rel
                d.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(f, d)
        print("  theme/%s/%s (from chromium)" % (scale_dir, ctx.component))


def _deploy_tiles(ctx: DeployContext) -> None:
    kernel_tiles = ctx.kernel_icons / "tiles"
    chromium_tiles = ctx.theme / "chromium" / "win" / "tiles"
    dst_tiles = ctx.theme / ctx.component / "win" / "tiles"
    dst_tiles.mkdir(parents=True, exist_ok=True)
    required = ("Logo.png", "SmallLogo.png")
    from_overlay = set(kernel_tiles.glob("*.png")) if kernel_tiles.is_dir() else set()
    names = set(required) | {p.name for p in from_overlay}
    for name in sorted(names):
        src = kernel_tiles / name if kernel_tiles.is_dir() else None
        if src and src.is_file():
            shutil.copy2(src, dst_tiles / name)
            print("  tiles/%s (from overlay/branding/icons/tiles)" % name)
        elif (chromium_tiles / name).is_file():
            shutil.copy2(chromium_tiles / name, dst_tiles / name)
            print("  tiles/%s (from chromium/win/tiles)" % name)
        else:
            print("  Warning: missing tiles/%s (skipped)" % name)


def _deploy_components_strings(ctx: DeployContext) -> None:
    comp = ctx.chromium_src / "components"
    src_grd = comp / "components_chromium_strings.grd"
    dst_grd = comp / ("components_%s_strings.grd" % ctx.component)
    if not src_grd.is_file():
        return
    shutil.copy2(src_grd, dst_grd)
    print("  components_%s_strings.grd (from chromium)" % ctx.component)


def _deploy_resource_ids_spec(ctx: DeployContext) -> None:
    """在 resource_ids.spec 中为 branded GRD 登记 first id（grit 需要）。"""
    spec = ctx.chromium_src / "tools" / "gritsettings" / "resource_ids.spec"
    if not spec.is_file():
        return
    app_grd = "chrome/app/%s_strings.grd" % ctx.component
    comp_grd = "components/components_%s_strings.grd" % ctx.component
    text = spec.read_text(encoding="utf-8")
    changed = False
    if app_grd not in text:
        needle = '  "chrome/app/google_chrome_strings.grd": {\n    "messages": [800],\n  },'
        block = needle + '\n  "%s": {\n    "messages": [800],\n  },' % app_grd
        if needle in text:
            text = text.replace(needle, block, 1)
            changed = True
            print("  resource_ids.spec: added %s (messages 800)" % app_grd)
    if comp_grd not in text:
        needle = '  "components/components_google_chrome_strings.grd": {\n    "messages": [7020],\n  },'
        block = needle + '\n  "%s": {\n    "messages": [7020],\n  },' % comp_grd
        if needle in text:
            text = text.replace(needle, block, 1)
            changed = True
            print("  resource_ids.spec: added %s (messages 7020)" % comp_grd)
    if changed:
        spec.write_text(text, encoding="utf-8")


def _deploy_installer_string_rc_brand(ctx: DeployContext) -> None:
    """在 create_installer_string_rc.py 的 MODE_SPECIFIC_STRINGS['IDS_PRODUCT_NAME'] 中加入品牌名。"""
    path = ctx.chromium_src / "chrome" / "installer" / "util" / "prebuild" / "create_installer_string_rc.py"
    if not path.is_file():
        return
    text = path.read_text(encoding="utf-8")
    if ("'%s': [\n      'IDS_PRODUCT_NAME'" % ctx.component) in text:
        return
    needle = "    'chromium': [\n      'IDS_PRODUCT_NAME',\n    ],\n  },\n}"
    block = "    'chromium': [\n      'IDS_PRODUCT_NAME',\n    ],\n    '%s': [\n      'IDS_PRODUCT_NAME',\n    ],\n  },\n}" % ctx.component
    if needle not in text:
        return
    path.write_text(text.replace(needle, block, 1), encoding="utf-8")
    print("  create_installer_string_rc.py: added %s to IDS_PRODUCT_NAME" % ctx.component)


def _deploy_vector_icons(ctx: DeployContext) -> None:
    vec = ctx.chromium_src / "components" / "vector_icons"
    src_dir = vec / "chromium"
    dst_dir = vec / ctx.component
    dst_dir.mkdir(parents=True, exist_ok=True)
    for name in ("product.icon", "product_refresh.icon"):
        src = src_dir / name
        dst = dst_dir / name
        if src.is_file():
            shutil.copy2(src, dst)
            print("  vector_icons/%s/%s (from chromium)" % (ctx.component, name))
        else:
            print("  Warning: missing vector_icons/chromium/%s (skipped)" % name)


def _deploy_strings_from_templates(ctx: DeployContext, config: dict) -> None:
    """从 overlay/branding/strings 的 .j2 模板渲染并写入 chrome/app/{component}_strings.*。"""
    if template_util is None:
        return
    strings_dir = ctx.project_root / "strings"
    grd_j2 = strings_dir / "strings.grd.j2"
    if not grd_j2.is_file():
        return
    app = ctx.chromium_src / "chrome" / "app"
    resources_dir = app / "resources"
    resources_dir.mkdir(parents=True, exist_ok=True)
    content = grd_j2.read_text(encoding="utf-8")
    rendered = template_util.render_template_for_strings(content, config)
    dst_grd = app / ("%s_strings.grd" % ctx.component)
    dst_grd.write_text(rendered, encoding="utf-8")
    count = 0
    resources_j2 = strings_dir / "resources"
    for f in sorted(resources_j2.glob("strings_*.xtb.j2")):
        # strings_zh-CN.xtb.j2 -> stem "strings_zh-CN.xtb" -> suffix "zh-CN"
        suffix = f.stem.replace("strings_", "", 1).replace(".xtb", "", 1)
        content = f.read_text(encoding="utf-8")
        rendered = template_util.render_template_for_strings(content, config)
        dst = resources_dir / ("%s_strings_%s.xtb" % (ctx.component, suffix))
        dst.write_text(rendered, encoding="utf-8")
        count += 1
    print("  %s_strings.grd + %d %s_strings_*.xtb (from overlay/branding/strings templates)" % (ctx.component, count, ctx.component))


def _deploy_chrome_app_strings(ctx: DeployContext) -> None:
    app = ctx.chromium_src / "chrome" / "app"
    resources = app / "resources"
    src_grd = app / "chromium_strings.grd"
    dst_grd = app / ("%s_strings.grd" % ctx.component)
    if not src_grd.is_file():
        return
    shutil.copy2(src_grd, dst_grd)
    prefix = "%s_strings_" % ctx.component
    text = dst_grd.read_text(encoding="utf-8")
    if "chromium_strings_" in text:
        text = text.replace("resources/chromium_strings_", "resources/" + prefix)
        dst_grd.write_text(text, encoding="utf-8")
    count = 0
    for f in resources.glob("chromium_strings_*.xtb"):
        dst = resources / (prefix + f.name.replace("chromium_strings_", "", 1))
        shutil.copy2(f, dst)
        count += 1
    if count:
        print("  %s_strings.grd + %d %s_strings_*.xtb (from chromium)" % (ctx.component, count, ctx.component))


def run(chromium_src, kernel_root, project_root):
    config = load_config_from_project_root(project_root)
    component = config.get("branding_path_component", "simprint")
    icon_names, fallback = _icon_names_and_fallback(component)

    theme = Path(chromium_src) / "chrome" / "app" / "theme"
    kernel_icons = Path(project_root) / "icons"
    chromium_win = theme / "chromium" / "win"
    dst_win = theme / component / "win"
    branding_file = theme / component / "BRANDING"
    if not branding_file.is_file():
        raise FileNotFoundError("theme/%s/BRANDING not found. Apply branding patches first." % component)

    ctx = DeployContext(
        chromium_src=Path(chromium_src),
        project_root=Path(project_root),
        component=component,
        theme=theme,
        kernel_icons=kernel_icons,
        chromium_win=chromium_win,
        dst_win=dst_win,
    )

    print("Deploying theme/%s/win icons..." % component)
    _deploy_win_icons(ctx, icon_names, fallback)
    print("Deploying theme/%s/ product logos (chrome_unscaled_resources)..." % component)
    _deploy_theme_product_logos(ctx)
    print("Deploying theme/default_*_percent/%s (theme_resources scaled)..." % component)
    _deploy_theme_scaled_resources(ctx)
    print("Deploying theme/%s/win/tiles..." % component)
    _deploy_tiles(ctx)
    print("Deploying components_%s_strings.grd..." % component)
    _deploy_components_strings(ctx)
    print("Deploying resource_ids.spec (grit first id)...")
    _deploy_resource_ids_spec(ctx)
    print("Deploying create_installer_string_rc.py (IDS_PRODUCT_NAME %s)..." % component)
    _deploy_installer_string_rc_brand(ctx)
    print("Deploying vector_icons/%s..." % component)
    _deploy_vector_icons(ctx)
    print("Deploying chrome/app %s_strings + .xtb..." % component)
    strings_j2 = project_root / "strings" / "strings.grd.j2"
    if strings_j2.is_file() and template_util is not None:
        _deploy_strings_from_templates(ctx, config)
    else:
        _deploy_chrome_app_strings(ctx)
    print("Done. Branding resources deployed; ready to build.")
