# Branding deploy: copy icons, tiles, strings, vector_icons so simprint builds without extra steps
# 优先从 overlay/branding/icons 复制；缺失时从 Chromium 对应路径复制
from pathlib import Path
import shutil

# 目标文件名（theme/simprint/win 下）
ICON_NAMES = [
    "simprint.ico",
    "app_list.ico",
    "incognito.ico",
    "simprint_doc.ico",
    "simprint_pdf.ico",
]

# Chromium 占位源：(chromium/win 中的文件名, simprint/win 中的文件名)
CHROMIUM_FALLBACK = [
    ("chromium.ico", "simprint.ico"),
    ("app_list.ico", "app_list.ico"),
    ("incognito.ico", "incognito.ico"),
    ("chromium_doc.ico", "simprint_doc.ico"),
    ("chromium_pdf.ico", "simprint_pdf.ico"),
]


def _deploy_win_icons(theme, kernel_icons, chromium_win, dst_win):
    dst_win.mkdir(parents=True, exist_ok=True)
    for dst_name in ICON_NAMES:
        dst_file = dst_win / dst_name
        src_file = kernel_icons / dst_name
        if src_file.is_file():
            shutil.copy2(src_file, dst_file)
            print("  %s (from overlay/branding/icons)" % dst_name)
            continue
        src_name = next((s for s, d in CHROMIUM_FALLBACK if d == dst_name), None)
        if src_name and (chromium_win / src_name).is_file():
            shutil.copy2(chromium_win / src_name, dst_file)
            print("  %s (from chromium/win)" % dst_name)
        else:
            print("  Warning: missing %s (skipped)" % dst_name)


def _deploy_tiles(theme, kernel_icons, chromium_src):
    kernel_tiles = kernel_icons / "tiles"
    chromium_tiles = theme / "chromium" / "win" / "tiles"
    dst_tiles = theme / "simprint" / "win" / "tiles"
    dst_tiles.mkdir(parents=True, exist_ok=True)
    # 至少部署构建需要的 Logo.png、SmallLogo.png；若 overlay 中有其他 .png 也一并部署
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


def _deploy_components_strings(chromium_src):
    comp = chromium_src / "components"
    src_grd = comp / "components_chromium_strings.grd"
    dst_grd = comp / "components_simprint_strings.grd"
    if not src_grd.is_file():
        return
    shutil.copy2(src_grd, dst_grd)
    print("  components_simprint_strings.grd (from chromium)")


def _deploy_resource_ids_spec(chromium_src):
    """Ensure resource_ids.spec has first id for components_simprint_strings.grd (grit 需要)."""
    spec = chromium_src / "tools" / "gritsettings" / "resource_ids.spec"
    if not spec.is_file():
        return
    text = spec.read_text(encoding="utf-8")
    if "components_simprint_strings.grd" in text:
        return
    # 与 components_chromium_strings 同一起始 id，构建时二选一
    needle = '  "components/components_google_chrome_strings.grd": {\n    "messages": [7020],\n  },'
    block = needle + '\n  "components/components_simprint_strings.grd": {\n    "messages": [7020],\n  },'
    if needle not in text:
        return
    spec.write_text(text.replace(needle, block, 1), encoding="utf-8")
    print("  resource_ids.spec: added components_simprint_strings.grd (messages 7020)")


def _deploy_installer_string_rc_simprint(chromium_src):
    """在 create_installer_string_rc.py 的 MODE_SPECIFIC_STRINGS['IDS_PRODUCT_NAME'] 中加入 simprint（补丁 004 仅含前 3 项，此项由 deploy 补齐）。"""
    path = chromium_src / "chrome" / "installer" / "util" / "prebuild" / "create_installer_string_rc.py"
    if not path.is_file():
        return
    text = path.read_text(encoding="utf-8")
    if "'simprint': [\n      'IDS_PRODUCT_NAME'" in text:
        return
    needle = "    'chromium': [\n      'IDS_PRODUCT_NAME',\n    ],\n  },\n}"
    block = "    'chromium': [\n      'IDS_PRODUCT_NAME',\n    ],\n    'simprint': [\n      'IDS_PRODUCT_NAME',\n    ],\n  },\n}"
    if needle not in text:
        return
    path.write_text(text.replace(needle, block, 1), encoding="utf-8")
    print("  create_installer_string_rc.py: added simprint to IDS_PRODUCT_NAME")


def _deploy_vector_icons(chromium_src):
    vec = chromium_src / "components" / "vector_icons"
    src_dir = vec / "chromium"
    dst_dir = vec / "simprint"
    dst_dir.mkdir(parents=True, exist_ok=True)
    for name in ("product.icon", "product_refresh.icon"):
        src = src_dir / name
        dst = dst_dir / name
        if src.is_file():
            shutil.copy2(src, dst)
            print("  vector_icons/simprint/%s (from chromium)" % name)
        else:
            print("  Warning: missing vector_icons/chromium/%s (skipped)" % name)


def _deploy_chrome_app_strings(chromium_src):
    app = chromium_src / "chrome" / "app"
    resources = app / "resources"
    src_grd = app / "chromium_strings.grd"
    dst_grd = app / "simprint_strings.grd"
    if not src_grd.is_file():
        return
    shutil.copy2(src_grd, dst_grd)
    # Replace chromium_strings_ with simprint_strings_ in .grd so it references our .xtb
    text = dst_grd.read_text(encoding="utf-8")
    if "chromium_strings_" in text:
        text = text.replace("resources/chromium_strings_", "resources/simprint_strings_")
        dst_grd.write_text(text, encoding="utf-8")
    # Copy all chromium_strings_*.xtb to simprint_strings_*.xtb
    count = 0
    for f in resources.glob("chromium_strings_*.xtb"):
        dst = resources / ("simprint_strings_" + f.name.replace("chromium_strings_", "", 1))
        shutil.copy2(f, dst)
        count += 1
    if count:
        print("  simprint_strings.grd + %d simprint_strings_*.xtb (from chromium)" % count)


def run(chromium_src, kernel_root, project_root):
    theme = chromium_src / "chrome" / "app" / "theme"
    kernel_icons = Path(project_root) / "icons"
    chromium_win = theme / "chromium" / "win"
    dst_win = theme / "simprint" / "win"
    branding = theme / "simprint" / "BRANDING"
    if not branding.is_file():
        raise FileNotFoundError("theme/simprint/BRANDING not found. Apply branding patches first.")

    print("Deploying theme/simprint/win icons...")
    _deploy_win_icons(theme, kernel_icons, chromium_win, dst_win)
    print("Deploying theme/simprint/win/tiles...")
    _deploy_tiles(theme, kernel_icons, chromium_src)
    print("Deploying components_simprint_strings.grd...")
    _deploy_components_strings(chromium_src)
    print("Deploying resource_ids.spec (grit first id)...")
    _deploy_resource_ids_spec(chromium_src)
    print("Deploying create_installer_string_rc.py (IDS_PRODUCT_NAME simprint)...")
    _deploy_installer_string_rc_simprint(chromium_src)
    print("Deploying vector_icons/simprint...")
    _deploy_vector_icons(chromium_src)
    print("Deploying chrome/app simprint_strings + .xtb...")
    _deploy_chrome_app_strings(chromium_src)
    print("Done. Branding resources deployed; ready to build.")
