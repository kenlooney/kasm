# 0.7.0–0.9.0 — Labels, Layout, and Near Jumps

This trio of versions is where Kasm stopped being a straight-line
instruction emitter and started being able to describe *programs* with
control flow.

## 0.7.0 — Labels that don't cost bytes

`8f4f3b3 — Update project version to 0.7.0, implement label definitions, and
enhance Windows execution support` parsed label definitions — including
consecutive labels pointing at the same location — without emitting any
bytes for them. A label is purely a name bound to a position; it has zero
size in the encoded image.

## 0.8.0 — A layout pass, finally

Parsing labels is easy. Knowing *where* they point requires knowing the size
of everything before them, which requires a dedicated pass. `e643517 —
Update project version to 0.8.0, implement layout functionality, and add
label handling` introduced that pass: it walks the statement array once,
assigns byte offsets to instructions and labels, rejects duplicate label
names, and provides an offset lookup used by everything that follows —
jumps, relocations, and (much later) data directives and location-aware `$`/
`$$` expressions.

Introducing a distinct layout stage — separate from parsing and separate
from encoding — is arguably the single most consequential structural
decision in Kasm's history. Every forward-reference feature added after this
point (near/short/absolute jumps, conditional branches, TIMES/FILL counts,
`$`/`$$`) depends on layout running to completion *before* encoding starts.

## 0.9.0 — Near jumps and forward references

`d98a002 — Update project version to 0.9.0, implement near jump
functionality, and enhance parser and encoder` added `jmp` with a signed
relative displacement, resolved against the label table built during
layout. This is the first feature that genuinely required the layout pass:
a forward jump's displacement can't be computed until the size of every
instruction between the jump and its target is known.

At this point Kasm could express loops and forward branches, but only in one
jump form, and only as a raw relative displacement. The next chapter adds a
second jump form, a relocation mechanism, and jumps through memory rather
than through an immediate displacement.
