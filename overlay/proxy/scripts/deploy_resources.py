"""
Proxy deploy: copy new source files into the Chromium tree.
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

    for src_file in sources_dir.rglob("*"):
        if not src_file.is_file():
            continue

        rel_path = src_file.relative_to(sources_dir)
        dst_file = chromium_src / rel_path
        dst_file.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src_file, dst_file)
        print(f"  {rel_path.as_posix()} -> {rel_path.parent.as_posix()}/")

    print("Done. Proxy sources deployed.")
