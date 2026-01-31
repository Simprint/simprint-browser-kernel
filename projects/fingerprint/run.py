"""
Fingerprint 单元统一入口：apply / deploy / build。
由 driver 调用：python projects/fingerprint/run.py <phase>
当前为占位，apply/deploy/build 均为 no-op。
"""
import sys


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: run.py <apply|deploy|build>", file=sys.stderr)
        raise SystemExit(1)
    phase = sys.argv[1].lower()
    if phase not in ("apply", "deploy", "build"):
        print(f"Unknown phase: {phase}", file=sys.stderr)
        raise SystemExit(1)
    # 占位：暂无补丁与部署逻辑
    pass


if __name__ == "__main__":
    main()
