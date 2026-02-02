"""
NTP deploy: 复制新增源文件到 Chromium 对应位置
"""
import shutil
from pathlib import Path


def run(chromium_src: Path, kernel_root: Path, project_root: Path) -> None:
    chromium_src = Path(chromium_src)
    project_root = Path(project_root)

    # 复制 simprint_ip C++ 源文件
    src_ip_dir = project_root / "simprint_ip"
    dst_ip_dir = chromium_src / "chrome" / "browser" / "ui" / "webui" / "new_tab_page" / "simprint_ip"
    if src_ip_dir.is_dir():
        dst_ip_dir.mkdir(parents=True, exist_ok=True)
        for f in src_ip_dir.iterdir():
            if f.is_file():
                shutil.copy2(f, dst_ip_dir / f.name)
                print(f"  {f.name} -> chrome/browser/ui/webui/new_tab_page/simprint_ip/")

    # 复制前端资源
    resources_dir = project_root / "resources"
    dst_ntp_dir = chromium_src / "chrome" / "browser" / "resources" / "new_tab_page"
    if resources_dir.is_dir():
        for f in resources_dir.iterdir():
            if f.is_file():
                shutil.copy2(f, dst_ntp_dir / f.name)
                print(f"  {f.name} -> chrome/browser/resources/new_tab_page/")

    print("Done. NTP resources deployed.")
