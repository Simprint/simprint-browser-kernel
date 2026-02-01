"""
008: variations generate_ui_string_overrider 按 (hash, name) 去重。
若目标文件已包含该逻辑则跳过，避免上游已有修复时重复插入。
由 overlay/branding/run.py apply 阶段执行；需环境变量 SIMPRINT_CHROMIUM_ROOT。
"""
import os
import sys
from pathlib import Path

# 要插入的代码块（在 resources = list(set(resources)) 与 # The default 之间）
DEDUPE_BLOCK = """
  # Deduplicate by (hash, name): the same name may appear in multiple branded
  # headers (e.g. chrome branded_strings and components_branded_strings) with
  # different indices when using a third-party brand like Simprint. Keep the
  # first occurrence to avoid HashCollisionError.
  seen = {}
  for r in resources:
    key = (r.hash, r.name)
    if key not in seen:
      seen[key] = r
  resources = list(seen.values())

"""

# 用于检测“已应用”的特征字符串
MARKER = "Deduplicate by (hash, name)"
ANCHOR_AFTER = "  resources = list(set(resources))\n\n"
ANCHOR_BEFORE = "  # The default |Resource| order makes |resources| sorted by the hash, then"


def main() -> None:
    chromium_src = os.environ.get("SIMPRINT_CHROMIUM_ROOT")
    if not chromium_src:
        print("SIMPRINT_CHROMIUM_ROOT not set", file=sys.stderr)
        sys.exit(1)
    target = Path(chromium_src).resolve() / "components" / "variations" / "service" / "generate_ui_string_overrider.py"
    if not target.is_file():
        print(f"Target not found: {target}", file=sys.stderr)
        sys.exit(1)

    content = target.read_text(encoding="utf-8")
    if MARKER in content:
        print("  Already applied (008-variations-dedupe-by-name), skip.")
        return

    # 在 ANCHOR_AFTER 之后、ANCHOR_BEFORE 之前插入 DEDUPE_BLOCK
    needle = ANCHOR_AFTER + ANCHOR_BEFORE
    replacement = ANCHOR_AFTER + DEDUPE_BLOCK.lstrip("\n") + ANCHOR_BEFORE
    if needle not in content:
        print("  Could not find insertion point (anchor text changed?), skip.", file=sys.stderr)
        sys.exit(1)
    new_content = content.replace(needle, replacement, 1)
    target.write_text(new_content, encoding="utf-8")
    print("  Applied 008-variations-dedupe-by-name (dedupe by hash,name).")


if __name__ == "__main__":
    main()
