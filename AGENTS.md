# Agent instructions for this repository

These instructions apply to any AI coding agent (GitHub Copilot, Claude, or
similar) making changes in this repository. They exist so that Kasm's
version number and its [dev journal](devlog/src/introduction.md) stay
accurate without a human having to remember to update them by hand.

## 1. Every code change bumps the version

Kasm's version lives in `project(kasm VERSION <major>.<minor>.<patch> ...)`
in [CMakeLists.txt](CMakeLists.txt). Treat it as
`<breaking>.<feature>.<patch>`:

| Change you made | Bump | Example |
| --- | --- | --- |
| Removed/renamed a CLI flag, changed an existing instruction's syntax or encoding, changed on-disk output formats, or otherwise broke something a user could have depended on | **breaking** (first number). Reset feature and patch to `0`. | `0.34.1` → `1.0.0` |
| Added a new instruction, directive, CLI flag, output format, or other capability that did not exist before, without breaking existing behavior | **feature** (second number). Reset patch to `0`. | `0.34.1` → `0.35.0` |
| Bug fix, test-only change, refactor, or documentation-only change with no behavior change | **patch** (third number) | `0.34.1` → `0.34.2` |

Rules:

- **Every commit that changes `src/`, `include/`, `tests/`, `examples/`, or
  `CMakeLists.txt` behavior bumps at least the patch number.** Pure
  documentation/journal edits (README, `devlog/`, comments) do not require a
  version bump on their own unless they accompany a code change.
- Only ever increment. Never decrement a version number or reuse one that
  has already been tagged (check `git tag` if unsure).
- When in doubt between two categories, prefer the larger bump — it is
  cheaper to over-signal a change than to under-signal a breaking one.
- Update the version in exactly one place: `CMakeLists.txt`. Nothing else
  hardcodes it except generated release artifacts and the dev journal
  entries described below, which reference specific past versions and
  should not be edited retroactively.

## 2. Every feature or breaking change gets a dev journal entry

The [dev journal](devlog/) is a living, chronological record of how Kasm
grew, built with [mdbook](https://rust-lang.github.io/mdBook/). When you
make a **feature** or **breaking** change (not a patch-only fix, unless the
fix is itself noteworthy), add a chapter for it:

1. Create `devlog/src/<next-number>-<short-slug>.md`, using the next
   available number after the highest-numbered chapter in
   `devlog/src/SUMMARY.md`. Follow the tone and structure of existing
   chapters (see `devlog/src/14-coff-objects.md` or
   `devlog/src/15-launching-the-journal.md` for recent examples): explain
   what changed, why, and what it does or doesn't cover yet. Link to real
   source files, tests, and commands where relevant, using
   `https://github.com/kenlooney/kasm/blob/main/<path>` for anything a
   reference page or journal entry needs to cite.
2. Add the new chapter to `devlog/src/SUMMARY.md`, in the `# Development
   Journal` part, immediately before the "Retrospective" and "Version
   Reference" entries.
3. Add a row to the table in `devlog/src/appendix-version-table.md`
   summarizing the new version's capability.
4. If the change adds or modifies user-facing behavior (a new instruction,
   directive, CLI flag, or similar), also update the relevant chapter under
   `devlog/src/reference/` (or add a new one, linked from
   `devlog/src/SUMMARY.md`'s `# Reference` part and from the "Where to go"
   table in [README.md](README.md)) so the reference docs reflect current
   behavior, not just the historical narrative.
5. Rebuild the book locally to confirm it still compiles before finishing:

   ```powershell
   cd devlog
   mdbook build
   ```

## 3. Keep README.md short

README.md is intentionally a landing page, not documentation. Do not add
detailed feature documentation there — add or update a `devlog/src/reference/`
chapter instead, and link to it from README's "Where to go" table if it's a
new topic.
