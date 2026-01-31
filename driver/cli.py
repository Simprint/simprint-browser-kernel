"""命令行入口：apply、deploy、build、apply-and-prepare，可选 --project。"""
import argparse

from . import runner
from .config import get_chromium_src, get_repo_root


def _parse_project_list(s: str | None) -> list[str] | None:
    if s is None or s.strip() == "":
        return None
    return [p.strip() for p in s.split(",") if p.strip()]


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Simprint 定制流水线：按 driver/order.txt 执行 integration、overlay、projects 的 apply / deploy / build",
        prog="python -m driver",
    )
    parser.add_argument(
        "--project",
        "-p",
        metavar="NAME",
        help="仅处理指定单元（逗号分隔），如 integration,branding,fingerprint",
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
    project_filter = _parse_project_list(args.project)
    repo_root = get_repo_root()
    chromium_src = get_chromium_src()
    out_dir = getattr(args, "out_dir", None)

    if args.command == "apply":
        runner.run_phase(repo_root, chromium_src, "apply", project_filter, out_dir)
    elif args.command == "deploy":
        runner.run_phase(repo_root, chromium_src, "deploy", project_filter, out_dir)
    elif args.command == "apply-deploy":
        runner.run_phase(repo_root, chromium_src, "apply_deploy", project_filter, out_dir)
    elif args.command == "build":
        runner.run_phase(repo_root, chromium_src, "build", project_filter, out_dir)
    elif args.command == "apply-and-prepare":
        runner.run_phase(repo_root, chromium_src, "apply", project_filter, out_dir)
        runner.run_phase(repo_root, chromium_src, "deploy", project_filter, out_dir)
        print("Apply and prepare done. Next: gn gen and build in Chromium.")


if __name__ == "__main__":
    main()
