"""命令行入口：按 driver/order.txt 执行标准浏览器内核流水线。"""
import argparse
import json

from . import runner
from .config import get_chromium_src, get_repo_root
from .contract import load_manifest, resolve_capabilities, validate_port


def _print_validation(repo_root, as_json: bool) -> int:
    report = validate_port(repo_root)
    if as_json:
        print(
            json.dumps(
                {
                    "ok": report.ok,
                    "patch_count": report.patch_count,
                    "errors": report.errors,
                    "warnings": report.warnings,
                    "overlapping_targets": report.overlapping_targets,
                },
                ensure_ascii=False,
                indent=2,
            )
        )
    else:
        status = "PASS" if report.ok else "FAIL"
        print(f"Port contract: {status}")
        print(f"Parsed legacy patches: {report.patch_count}")
        print(f"Overlapping upstream targets: {len(report.overlapping_targets)}")
        for warning in report.warnings:
            print(f"WARNING: {warning}")
        for error in report.errors:
            print(f"ERROR: {error}")
    return 0 if report.ok else 1


def _print_plan(repo_root, feature_values: list[str], as_json: bool) -> int:
    manifest = load_manifest(repo_root)
    requested = [
        value.strip()
        for group in feature_values
        for value in group.split(",")
        if value.strip()
    ]
    selected = resolve_capabilities(manifest, requested)
    selected_ids = {item.id for item in selected}
    selected_overlays = {item.overlay for item in selected}
    legacy_positions = [
        index
        for index, item in enumerate(manifest.capabilities)
        if item.overlay in selected_overlays and item.state == "legacy-patch"
    ]
    legacy_prefix = (
        manifest.capabilities[: max(legacy_positions) + 1] if legacy_positions else ()
    )
    required_by_text_order = [
        item.id for item in legacy_prefix if item.id not in selected_ids
    ]
    plan = {
        "chromium_version": manifest.chromium_version,
        "core_api": manifest.core_api,
        "requested": requested,
        "semantic_closure": [item.id for item in selected],
        "legacy_execution_prefix": [item.id for item in legacy_prefix],
        "blocked_by_text_order": required_by_text_order,
        "independently_composable": not required_by_text_order,
    }
    if as_json:
        print(json.dumps(plan, ensure_ascii=False, indent=2))
    else:
        print(f"Chromium port: {manifest.chromium_version}")
        print(f"Core API: v{manifest.core_api}")
        print(f"Requested: {', '.join(requested)}")
        print(
            "Semantic closure: "
            + ", ".join(item.id for item in selected)
        )
        if required_by_text_order:
            print(
                "Legacy text-order blockers: "
                + ", ".join(required_by_text_order)
            )
            print("This selection is not independently composable yet.")
        else:
            print("This selection is independently composable.")
    return 0


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
    validate_parser = sub.add_parser(
        "validate", help="校验当前分支的 Chromium port、Core 边界与 legacy patch 清单"
    )
    validate_parser.add_argument("--json", action="store_true", help="输出 JSON 报告")
    plan_parser = sub.add_parser(
        "plan", help="解析能力依赖，并显示仍受 legacy patch 文本顺序约束的模块"
    )
    plan_parser.add_argument(
        "--features",
        action="append",
        required=True,
        metavar="ID[,ID...]",
        help="能力 ID；可重复或用逗号分隔",
    )
    plan_parser.add_argument("--json", action="store_true", help="输出 JSON 计划")

    args = parser.parse_args()
    repo_root = get_repo_root()
    if args.command == "validate":
        raise SystemExit(_print_validation(repo_root, args.json))
    if args.command == "plan":
        try:
            code = _print_plan(repo_root, args.features, args.json)
        except ValueError as exc:
            parser.error(str(exc))
        raise SystemExit(code)

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
