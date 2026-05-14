"""命令行入口：按 driver/order.txt 执行标准浏览器内核流水线。"""
import argparse

from . import runner
from .config import get_chromium_src, get_repo_root


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Simprint 定制流水线：按 driver/order.txt 顺序执行各 overlay 单元的 apply / deploy / build",
        prog="python -m driver",
    )
    parser.add_argument(
        "--out-dir",
        metavar="DIR",
        default=None,
        help="Chromium 输出目录（相对 Chromium 根），如 out/Default、out/Release；用于 deploy 时写入 args.gn 等。默认 out/Default 或环境变量 SIMPRINT_OUT_DIR",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("apply", help="按 driver/order.txt 对每个单元执行 run.py apply")
    sub.add_parser("deploy", help="按 driver/order.txt 对每个单元执行 run.py deploy")
    sub.add_parser("apply-deploy", help="对每个单元依次执行 apply 再 deploy（一次完成）")
    sub.add_parser("build", help="按 driver/order.txt 对每个单元执行 run.py build")
    sub.add_parser("apply-and-prepare", help="先 apply 再 deploy（全单元各跑一遍）")

    args = parser.parse_args()
    repo_root = get_repo_root()
    chromium_src = get_chromium_src()
    out_dir = getattr(args, "out_dir", None)

    if args.command == "apply":
        runner.run_phase(repo_root, chromium_src, "apply", out_dir)
    elif args.command == "deploy":
        runner.run_phase(repo_root, chromium_src, "deploy", out_dir)
    elif args.command == "apply-deploy":
        runner.run_phase(repo_root, chromium_src, "apply_deploy", out_dir)
    elif args.command == "build":
        runner.run_phase(repo_root, chromium_src, "build", out_dir)
    elif args.command == "apply-and-prepare":
        runner.run_phase(repo_root, chromium_src, "apply_deploy", out_dir)
        print("Apply and prepare done. Next: gn gen and build in Chromium.")


if __name__ == "__main__":
    main()
