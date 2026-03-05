"""
Account deploy: 本模块只修改现有文件，无需部署新源文件
"""
from pathlib import Path


def run(chromium_src: Path, kernel_root: Path, project_root: Path) -> None:
    """
    Account 模块只通过 patch 修改现有文件，不需要部署新的源文件
    """
    print("Account module: No source files to deploy (patch-only module)")
