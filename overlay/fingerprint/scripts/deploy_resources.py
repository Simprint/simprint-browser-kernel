"""
部署 fingerprint 模块的资源文件到 Chromium 源码树
"""
import shutil
from pathlib import Path


def run(chromium_src: Path, kernel_root: Path, project_root: Path) -> None:
    """
    部署资源文件
    - 复制 simprint/config 和 simprint/fingerprint 源文件
    - 删除 simprint/console_log 目录
    """
    sources_dir = project_root / "sources" / "simprint"
    if not sources_dir.is_dir():
        return

    simprint_dir = chromium_src / "simprint"

    # 删除 console_log 目录（如果存在）
    console_log_dir = simprint_dir / "console_log"
    if console_log_dir.exists():
        print(f"Removing: simprint/console_log/")
        shutil.rmtree(console_log_dir)

    # 创建目标目录
    config_dir = simprint_dir / "config"
    fingerprint_dir = simprint_dir / "fingerprint"
    config_dir.mkdir(parents=True, exist_ok=True)
    fingerprint_dir.mkdir(parents=True, exist_ok=True)

    # 部署 config 文件
    for src_file in sources_dir.glob("config_*"):
        target_name = src_file.name.replace("config_", "", 1)
        target_file = config_dir / target_name
        print(f"Deploying: simprint/config/{target_name}")
        shutil.copy2(src_file, target_file)

    # 部署 fingerprint 文件
    for src_file in sources_dir.glob("fingerprint_*"):
        target_name = src_file.name.replace("fingerprint_", "", 1)
        target_file = fingerprint_dir / target_name
        print(f"Deploying: simprint/fingerprint/{target_name}")
        shutil.copy2(src_file, target_file)
