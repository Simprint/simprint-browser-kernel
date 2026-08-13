from __future__ import annotations

import unittest
from pathlib import Path

from driver.contract import (
    Capability,
    PortManifest,
    _patch_targets_from_text,
    load_manifest,
    resolve_capabilities,
    validate_port,
)
from driver.patching import normalize_unified_diff


REPO_ROOT = Path(__file__).resolve().parents[1]


class PortManifestTests(unittest.TestCase):
    def test_repository_manifest_matches_driver_and_patches(self) -> None:
        report = validate_port(REPO_ROOT)
        self.assertEqual([], report.errors)
        self.assertGreater(report.patch_count, 100)

    def test_manifest_represents_a_single_chromium_branch(self) -> None:
        manifest = load_manifest(REPO_ROOT)
        self.assertEqual(144, manifest.chromium_milestone)
        self.assertEqual(1, manifest.core_api)
        self.assertFalse((REPO_ROOT / "port" / "chromium-144").exists())

    def test_dependency_resolution_is_topological(self) -> None:
        manifest = load_manifest(REPO_ROOT)
        selected = resolve_capabilities(manifest, ["launch-cookies"])
        self.assertEqual(
            ["eventbus-input-sync", "account-import", "launch-cookies"],
            [item.id for item in selected],
        )

    def test_unknown_capability_fails(self) -> None:
        manifest = load_manifest(REPO_ROOT)
        with self.assertRaisesRegex(ValueError, "unknown capability"):
            resolve_capabilities(manifest, ["does-not-exist"])

    def test_dependency_cycle_fails(self) -> None:
        manifest = PortManifest(
            schema_version=1,
            chromium_version="144.0.0.0",
            chromium_milestone=144,
            core_api=1,
            capabilities=(
                Capability("a", "a", "port", ("b",)),
                Capability("b", "b", "port", ("a",)),
            ),
        )
        with self.assertRaisesRegex(ValueError, "dependency cycle"):
            resolve_capabilities(manifest, ["a"])


class PatchSafetyTests(unittest.TestCase):
    def test_normalizes_crlf_and_blank_hunk_context(self) -> None:
        raw = (
            "diff --git a/a.cc b/a.cc\r\n"
            "--- a/a.cc\r\n"
            "+++ b/a.cc\r\n"
            "@@ -1,2 +1,2 @@\r\n"
            " old\r\n"
            "\r\n"
        )
        normalized = normalize_unified_diff(raw)
        self.assertEqual(
            "diff --git a/a.cc b/a.cc\n"
            "--- a/a.cc\n"
            "+++ b/a.cc\n"
            "@@ -1,2 +1,2 @@\n"
            " old\n"
            " \n",
            normalized,
        )

    def test_rejects_parent_traversal(self) -> None:
        _, errors = _patch_targets_from_text(
            "diff --git a/../outside.cc b/../outside.cc\n", "unsafe.patch"
        )
        self.assertTrue(any("unsafe patch target" in item for item in errors))

    def test_collects_valid_target(self) -> None:
        targets, errors = _patch_targets_from_text(
            "diff --git a/chrome/browser/example.cc "
            "b/chrome/browser/example.cc\n",
            "valid.patch",
        )
        self.assertEqual(["chrome/browser/example.cc"], targets)
        self.assertEqual([], errors)


if __name__ == "__main__":
    unittest.main()
