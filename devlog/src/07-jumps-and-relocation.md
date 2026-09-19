# 0.10.0–0.12.0 — Short Jumps, Relocation, and Absolute Jumps

## 0.10.0 — Two jump forms, one explicit keyword

`bc36448 — Update project version to 0.10.0, implement short jump
functionality` added `jmp short label;` alongside the existing near jump,
using a signed 8-bit displacement instead of a wider relative one. This
version also **removed** the earlier, more permissive `jmp answer;` syntax in
favor of requiring an explicit `jmp near answer;` or `jmp short answer;` —
a rare instance of the README calling out a breaking syntax change directly:
the ambiguity between two encodings for the same mnemonic was resolved by
making the choice explicit at the syntax level rather than picking one
automatically based on displacement size.

## 0.11.0 — Someone has to patch the address at load time

Not every address is known at assembly time — a runner loads Kasm's raw
image into memory at an address chosen by the OS, not by Kasm. `db287f4 —
Update project version to 0.11.0, implement load-time relocation, and
enhance output handling` introduced `include/relocate.h`: a helper that
patches an image slot to a runtime address after the image has been loaded,
plus relocation metadata emitted into the generated C header. Both the Linux
and Windows runners were updated to apply these patches before executing the
image — this is the first time the runner examples had to do more than "copy
bytes and call them."

## 0.12.0 — `jmp abs`, verified on WSL2

`d0fd215 — Update project version to 0.12.0, implement absolute jump
functionality` added `jmp abs label;` through a full-width address slot: the
target address is stored as data in the image, patched at load time via the
0.11.0 relocation mechanism, and jumped to indirectly. This was verified by
actually executing a relocated jump under WSL2 — not just diffing hex — a
reminder that relocation bugs are exactly the kind of thing that look correct
in an encoded-bytes test but fail the moment the image runs at a different
address.

**Note on numbering:** 0.13.0 does not appear in Kasm's history. The
project's own version table marks 0.14.0 explicitly as "0.13.0 skipped" —
this journal preserves that gap rather than renumbering around it, since the
gap is part of the accurate record.

With relative, short, and absolute jumps all in place, Kasm had every jump
form it needed except *conditional* ones — which meant no `if`/`while`-style
control flow was possible yet. That's the next milestone.
