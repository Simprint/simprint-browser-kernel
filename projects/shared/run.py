"""
Shared 公共库区统一入口：apply / deploy / build。
由 driver 调用：python projects/shared/run.py <phase>
预留：将来用于通讯、认证、同步、RPA 等跨版本复用的静态/动态库；当前 apply/deploy/build 均为 no-op。
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
    # 预留：暂不实现静态/动态库编译
    pass


if __name__ == "__main__":
    main()
