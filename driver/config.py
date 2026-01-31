"""解析 driver 配置：Chromium 根目录、仓库根目录。"""
import os
import re
from pathlib import Path


def _repo_root() -> Path:
    """仓库根目录（本包所在目录的上一级）。"""
    return Path(__file__).resolve().parent.parent


def get_repo_root() -> Path:
    return _repo_root()


def get_chromium_src() -> Path:
    """从环境变量或 driver/driver.config 读取 Chromium 根目录。"""
    root = _repo_root()
    driver_dir = root / "driver"
    src = os.environ.get("SIMPRINT_CHROMIUM_ROOT")
    config_file = driver_dir / "driver.config"
    if not src and config_file.is_file():
        with open(config_file, encoding="utf-8") as f:
            for line in f:
                m = re.match(r"^\s*SIMPRINT_CHROMIUM_ROOT\s*=\s*(.+)$", line)
                if m:
                    src = m.group(1).strip().strip("'\"").strip()
                    break
    if not src:
        raise SystemExit(
            "Chromium root not set. Set SIMPRINT_CHROMIUM_ROOT or copy driver/driver.config.example to driver/driver.config"
        )
    path = Path(src).resolve()
    if not path.is_dir():
        raise SystemExit(f"Chromium root is not a directory: {path}")
    return path
