# 0.34.0 — Launching This Journal

Version 0.34.0 is documentation-only: no encoder, decoder, or semantic
behavior changed. It marks the point where this journal itself became part
of the project.

## What actually changed

- Added `devlog/`, an [mdbook](https://rust-lang.github.io/mdBook/) project
  containing this journal, sourced from the project's own Git history and
  README version table rather than written from memory after the fact.
- Trimmed the README's long "Version history" table and its surrounding
  historical narrative paragraphs, replacing them with a short pointer to
  this journal. The README's per-feature reference sections (16-bit forms,
  data directives, COFF output, and so on) were left in place — they
  document current behavior and usage, which is the README's job, not the
  journal's.
- Added a `publish-devlog` job to the CI workflow: on every push to `main`,
  after the existing build/test/package job finishes, it builds this book
  with `mdbook build devlog` and deploys it to GitHub Pages. It runs
  independently of (not before) the release job that publishes the ZIP
  archives — both depend only on the same upstream build job.

## Why bump the version for a docs-only change

Kasm's version history, going back to 0.1.0, has always advanced the version
number for every milestone worth recording — including a few, like 0.24.1
and 0.25.1, that were pure fixes rather than new features. Treating "the
project's own development record moved from ad hoc README prose to a
maintained, published journal" as a milestone worth a version bump keeps
that convention intact, and gives this chapter a natural place in the
timeline rather than leaving it undated.

No test changes accompany this version, because no testable behavior
changed. The existing test suite continues to be the record of what Kasm
does; this journal is the record of how it got here.
