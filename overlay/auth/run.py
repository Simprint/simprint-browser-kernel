"""Compatibility entry point for the auth Chromium port unit."""

import sys
from pathlib import Path


KERNEL_ROOT = Path(__file__).resolve().parents[2]
if str(KERNEL_ROOT) not in sys.path:
    sys.path.insert(0, str(KERNEL_ROOT))

from driver.unit import main  # noqa: E402


if __name__ == "__main__":
    main(Path(__file__).resolve().parent)
