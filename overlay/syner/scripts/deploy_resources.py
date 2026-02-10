"""
Syner deploy: 复制新增源文件到 Chromium 对应位置
"""
import shutil
from pathlib import Path


def run(chromium_src: Path, kernel_root: Path, project_root: Path) -> None:
    chromium_src = Path(chromium_src)
    project_root = Path(project_root)
    sources_dir = project_root / "sources"

    if not sources_dir.is_dir():
        print("No sources directory found, skipping deploy.")
        return

    # 复制 simprint/eventbus
    src_eventbus = sources_dir / "simprint" / "eventbus"
    dst_eventbus = chromium_src / "simprint" / "eventbus"
    if src_eventbus.is_dir():
        dst_eventbus.mkdir(parents=True, exist_ok=True)
        for f in src_eventbus.iterdir():
            if f.is_file():
                shutil.copy2(f, dst_eventbus / f.name)
                print(f"  {f.name} -> simprint/eventbus/")

    # 复制 simprint/console_log
    src_console_log = sources_dir / "simprint" / "console_log"
    dst_console_log = chromium_src / "simprint" / "console_log"
    if src_console_log.is_dir():
        dst_console_log.mkdir(parents=True, exist_ok=True)
        for f in src_console_log.iterdir():
            if f.is_file():
                shutil.copy2(f, dst_console_log / f.name)
                print(f"  {f.name} -> simprint/console_log/")

    # 复制 sync_input_* 文件
    src_sync_input = sources_dir / "chrome" / "browser" / "ui" / "views" / "frame"
    dst_sync_input = chromium_src / "chrome" / "browser" / "ui" / "views" / "frame"
    if src_sync_input.is_dir():
        dst_sync_input.mkdir(parents=True, exist_ok=True)
        for f in src_sync_input.glob("sync_input_*"):
            if f.is_file():
                shutil.copy2(f, dst_sync_input / f.name)
                print(f"  {f.name} -> chrome/browser/ui/views/frame/")

    print("Done. Syner sources deployed.")
