<div align="center">
  <h1>Simprint Browser Kernel</h1>
  <p>Patch pipeline, overlay modules, and release preparation layer for Simprint Chromium builds.</p>
  <p>
    <img alt="Python 3.11+" src="https://img.shields.io/badge/python-3.11%2B-3776ab?style=flat-square&labelColor=0f172a" />
    <img alt="Package Manager uv" src="https://img.shields.io/badge/package%20manager-uv-22c55e?style=flat-square&labelColor=0f172a" />
    <img alt="Base Chromium" src="https://img.shields.io/badge/base-Chromium-2563eb?style=flat-square&labelColor=0f172a" />
  </p>
  <p>
    <strong>English</strong> | <a href="./README.zh-CN.md">简体中文</a>
  </p>
</div>

---

## Introduction

Simprint Browser Kernel is the browser-kernel layer used to customize Chromium for Simprint. It does not ship Chromium source code itself. Instead, it provides the patch flow, overlay modules, resource deployment logic, and release preparation steps that are applied onto a separately managed Chromium source tree.

This repository exists to keep Chromium-facing customization isolated from the desktop client and runtime repositories. By moving branding, patch orchestration, and browser-side feature overlays into a dedicated repository, Simprint can evolve its browser kernel more explicitly and maintain upstream upgrades with clearer boundaries.

## Why Simprint Browser Kernel?

Maintaining a Chromium-based product usually becomes difficult when product changes are spread across ad hoc patches, local scripts, and unrelated application repositories. That makes upgrades harder, review more fragile, and build behavior less reproducible.

Simprint Browser Kernel is intended to make that layer more disciplined. It keeps the browser customization path explicit, treats Chromium as an external source dependency, and provides a small driver-based workflow for applying overlays in a repeatable order.

## Features

- **External Chromium workflow**: Keep Chromium source code outside this repository and apply Simprint-specific changes onto a separately managed source tree.
- **Driver-based pipeline**: Use `python -m driver` commands to run `apply`, `deploy`, `build`, or combined preparation flows in a fixed order.
- **Overlay modules**: Organize browser customizations into units such as `branding`, `ntp`, `syner`, `auth`, `fingerprint`, `review`, `account`, `proxy`, and `cookie`.
- **Patch and resource deployment**: Support both source patching and follow-up deployment work such as copying assets or syncing build arguments.
- **Template-ready customization**: Use repository-managed scripts and configuration files instead of scattering browser branding logic into one-off manual edits.
- **Structured customization flow**: Keep browser-kernel changes organized through explicit driver and overlay boundaries.

## Quick Start

### Prerequisites

- Python 3.11+
- `uv`
- A separate Chromium source tree managed outside this repository

### Run locally

```bash
uv sync
cp driver/driver.config.example driver/driver.config
uv run python -m driver apply-and-prepare
```

After preparation, build Chromium in your external source tree with your usual `gn` and `autoninja` workflow.

## Version Baseline

The current browser-kernel content is adapted against and validated on Chromium `144.0.7559.118`.

If you apply these overlays to a different Chromium revision, patch conflicts, source drift, or build breakage may need to be resolved separately.

## Status

Simprint Browser Kernel is being prepared as part of the broader Simprint open-source refactoring effort.

The repository is already used to manage Simprint-specific Chromium customization, but its structure, documentation, and release-facing conventions are still being cleaned up for long-term external collaboration.

## Contributing

Issues and pull requests are welcome.

High-value contribution areas currently include:

- Patch organization and upgrade safety improvements
- Build and release workflow cleanup
- Overlay module documentation
- Chromium upgrade validation and regression checks
- Tooling that reduces manual browser-kernel maintenance work

## License

This project is licensed under the GNU Affero General Public License v3.0 (AGPLv3).

If you want to use Simprint Browser Kernel in a way that does not comply with the AGPLv3 obligations, including distributing modified versions or providing modified versions as a closed-source service, please contact us for a commercial license.
