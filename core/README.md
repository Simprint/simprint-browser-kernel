# Simprint Core boundary

`core/` is reserved for code that can be shared by every supported Chromium
port. Code in this directory must not include Chromium Browser, Content, Blink,
Net, Services, or UI headers.

The public boundary is versioned under `core/include/simprint/kernel_api/vN`.
Compatible Core updates may be rolled into every supported Chromium branch
without rewriting its port. A breaking boundary requires a new API directory;
old Chromium branches may remain pinned to the previous API.

The current repository is still migrating from the ordered overlay pipeline.
Until a capability moves behind this boundary, its state remains
`legacy-patch` in `port/manifest.toml`.
