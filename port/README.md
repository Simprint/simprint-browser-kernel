# Chromium port

This directory describes the Chromium integration represented by the current
Git branch. A different Chromium milestone belongs on a different branch, not
in a sibling version directory.

`manifest.toml` records:

- the exact Chromium baseline;
- the Core API understood by this port;
- product capabilities and their semantic dependencies;
- whether a capability still relies on the ordered legacy patch pipeline.

Run `python -m driver validate` before publishing a port branch. The validator
also reports overlapping upstream targets so migration work can focus on the
most fragile integration points first.
