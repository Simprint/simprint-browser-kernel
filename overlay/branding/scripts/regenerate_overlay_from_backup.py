#!/usr/bin/env python3
"""
从 Chromium 源码（通过 SIMPRINT_CHROMIUM_ROOT）生成 overlay strings。

步骤：
1. 清空 backup，从 Chromium 复制 chromium_strings.grd 和 chromium_strings_*.xtb
2. 从 backup 生成 strings.grd.j2（替换路径和产品名）
3. 清空 resources/，从 backup 生成 strings_*.xtb.j2（替换产品名）

用法：在 kernel 根目录执行
  uv run python overlay/branding/scripts/regenerate_overlay_from_backup.py
"""

import sys
from pathlib import Path

_SCRIPTS_DIR = Path(__file__).resolve().parent
_BRANDING_DIR = _SCRIPTS_DIR.parent
_STRINGS_DIR = _BRANDING_DIR / "strings"
_BACKUP_DIR = _STRINGS_DIR / "backup"
_BACKUP_RES = _BACKUP_DIR / "resources"
_OVERLAY_RES = _STRINGS_DIR / "resources"

# 导入 driver/config.py
_KERNEL_ROOT = _BRANDING_DIR.parent.parent
sys.path.insert(0, str(_KERNEL_ROOT / "driver"))
import config as driver_config


def step1_sync_backup() -> bool:
    """清空 backup，从 Chromium 复制 chromium_strings.grd 和 chromium_strings_*.xtb。"""
    chromium_src = driver_config.get_chromium_src()
    app_dir = chromium_src / "chrome" / "app"
    grd = app_dir / "chromium_strings.grd"
    res_dir = app_dir / "resources"

    if not grd.is_file():
        print(f"错误: 未找到 {grd}", file=sys.stderr)
        return False

    # 清空 backup
    if (_BACKUP_DIR / "chromium_strings.grd").exists():
        (_BACKUP_DIR / "chromium_strings.grd").unlink()
    if _BACKUP_RES.is_dir():
        for f in _BACKUP_RES.glob("chromium_strings_*.xtb"):
            f.unlink()
    _BACKUP_RES.mkdir(parents=True, exist_ok=True)

    # 复制 grd
    (_BACKUP_DIR / "chromium_strings.grd").write_bytes(grd.read_bytes())

    # 复制 xtb
    count = 0
    for f in res_dir.glob("chromium_strings_*.xtb"):
        (_BACKUP_RES / f.name).write_bytes(f.read_bytes())
        count += 1

    print(f"步骤1: backup 已从 {chromium_src} 复制 chromium_strings.grd 和 {count} 个 .xtb")
    return True


def step2_generate_grd_j2() -> bool:
    """从 backup/chromium_strings.grd 生成 strings.grd.j2。"""
    src = _BACKUP_DIR / "chromium_strings.grd"
    if not src.is_file():
        print(f"错误: {src} 不存在", file=sys.stderr)
        return False

    content = src.read_text(encoding="utf-8")
    # 替换1: 翻译文件路径
    content = content.replace("chromium_strings_", "${ branding_path_component }_strings_")
    # 替换2: 产品名
    content = content.replace("Chromium", "${ product_display_name }")

    dst = _STRINGS_DIR / "strings.grd.j2"
    dst.write_text(content, encoding="utf-8")
    print("步骤2: strings.grd.j2 已生成")
    return True


def step3_generate_xtb_j2() -> bool:
    """清空 resources/strings_*.xtb.j2，从 backup 生成新的。"""
    if not _BACKUP_RES.is_dir():
        print(f"错误: {_BACKUP_RES} 不存在", file=sys.stderr)
        return False

    # 清空
    for f in _OVERLAY_RES.glob("strings_*.xtb.j2"):
        f.unlink()
    _OVERLAY_RES.mkdir(parents=True, exist_ok=True)

    count = 0
    for f in sorted(_BACKUP_RES.glob("chromium_strings_*.xtb")):
        # chromium_strings_zh-CN.xtb -> strings_zh-CN.xtb.j2
        suffix = f.name.replace("chromium_strings_", "").replace(".xtb", "")
        content = f.read_text(encoding="utf-8")
        # 替换: 产品名
        content = content.replace("Chromium", "${ product_display_name }")
        dst = _OVERLAY_RES / f"strings_{suffix}.xtb.j2"
        dst.write_text(content, encoding="utf-8")
        count += 1

    print(f"步骤3: resources/strings_*.xtb.j2 已生成 {count} 个")
    return True


def main() -> int:
    if not step1_sync_backup():
        return 1
    if not step2_generate_grd_j2():
        return 1
    if not step3_generate_xtb_j2():
        return 1
    print("完成")
    return 0


if __name__ == "__main__":
    sys.exit(main())
