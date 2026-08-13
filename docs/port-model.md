# Chromium branch, Core, and Port model

One Git branch represents one exact Chromium baseline. Do not copy milestones
into sibling directories. The branch-local `port/manifest.toml` records the
baseline, Core API, capabilities, semantic dependencies, and migration state.

Shared policy, protocol, configuration, and algorithms belong behind the
versioned API in `core/`. Chromium types, process lifecycle, Mojo plumbing, and
invasive subsystem integration belong to the current branch's Port. A Port may
be large; the invariant is that Core never depends on Chromium.

Capabilities marked `legacy-patch` still rely on historical text order. Use:

```bash
uv run python -m driver validate
uv run python -m driver plan --features fingerprint
```

The validator checks the exact patch inventory, normalized patch syntax, safe
paths, Core include boundaries, dependency cycles, driver order, and overlapping
upstream targets. The planner distinguishes semantic dependencies from modules
that are included only because a legacy patch expects their earlier text.

See the [Chinese maintenance guide](./port-model.zh-CN.md) for the full branch,
migration, Core roll, and CI workflow.
