# Branding deploy: copy icons to theme/simprint/win
# 优先从 overlay/branding/icons 复制；缺失时从 Chromium theme/chromium/win 复制
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


def run(chromium_src, kernel_root, project_root):
    theme = chromium_src / "chrome" / "app" / "theme"
    kernel_icons = Path(project_root) / "icons"
    chromium_win = theme / "chromium" / "win"
    dst_win = theme / "simprint" / "win"
    branding = theme / "simprint" / "BRANDING"
    if not branding.is_file():
        raise FileNotFoundError("theme/simprint/BRANDING not found. Apply branding patches first.")
    dst_win.mkdir(parents=True, exist_ok=True)

    for dst_name in ICON_NAMES:
        dst_file = dst_win / dst_name
        src_file = kernel_icons / dst_name
        if src_file.is_file():
            shutil.copy2(src_file, dst_file)
            print("  %s (from overlay/branding/icons)" % dst_name)
            continue
        # 回退到 Chromium theme/chromium/win
        src_name = next((s for s, d in CHROMIUM_FALLBACK if d == dst_name), None)
        if src_name and (chromium_win / src_name).is_file():
            shutil.copy2(chromium_win / src_name, dst_file)
            print("  %s (from chromium/win)" % dst_name)
        else:
            print("  Warning: missing %s (skipped)" % dst_name)

    print("Done. Icons deployed to %s" % dst_win)
