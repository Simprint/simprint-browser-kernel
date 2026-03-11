# Simprint Proxy Overlay

## Purpose

This module migrates the proxy feature set from:

- `simprint-browser/src`
- commit `4fb8aadb1ceab45e405905887476e2e5be65ee46`

into the kernel repository structure.

It contains only the proxy-related delta from that commit.

## Scope

- Launch-time proxy application from `LaunchConfig`
- Runtime proxy refresh from `kProxySet`
- Chromium fixed proxy rules
- HTTP / HTTPS proxy authentication
- SOCKS5 username/password authentication
- Runtime cache and connection cleanup needed for proxy switching
- Proxy-related error-page mapping

## Structure

```text
overlay/proxy/
├── apply_order.txt
├── README.md
├── run.py
├── patches/
├── scripts/
│   └── deploy_resources.py
└── sources/
    ├── net/socket/
    └── simprint/
```

## Conventions

- New files are placed in `sources/` and deployed into Chromium by script.
- Existing Chromium file modifications are stored in `patches/`.
- This module is intended to run after the existing kernel modules have already
  been applied, because the source commit was produced on top of that fully
  migrated state.

## Expected Order

Recommended `driver/order.txt` position:

```text
branding
ntp
syner
auth
fingerprint
review
account
proxy
```

## Notes

- The module depends on the existing EventBus / LaunchConfig foundation from
  `syner` and `auth`.
- The module should stay limited to the proxy delta from the target commit and
  should not absorb unrelated browser changes.
