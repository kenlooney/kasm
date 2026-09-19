# Introduction

This is the development journal for **Kasm** — Ken's Assembler — a small C11
assembler for a limited x86-64 (and now emerging x86-16) instruction set.

The [README](https://github.com/kenlooney/kasm/blob/main/README.md) is Kasm's
user manual: it documents current behavior, supported syntax, and how to build
and run it *today*. This book is different. It is a **living journal**, written
in chronological order, that tells the story of how Kasm grew — one small,
deliberate version at a time — from a bare lexer to an assembler that emits
Windows COFF objects a linker can consume.

Every chapter here corresponds to one or more version bumps in
`CMakeLists.txt`. Where the README states facts, this book explains *why* those
facts came to be: what problem the version solved, what constraints shaped the
design, and what was deliberately left for later.

## Why write this down

Kasm was built in short, fast, test-gated milestones — usually one capability
per version, each with a permanent regression test before moving on. That
discipline is easy to lose sight of once the codebase is large. This journal
exists to preserve the reasoning trail: not just *what* Kasm can do, but the
order it learned to do it in, and the reviews and fixes along the way.

## How to read this book

The chapters are in the same order the assembler grew. If you only want to
know what Kasm can do right now, read the README instead. If you want to
understand *why* jumps require an explicit `near`/`short` keyword, why data
directives skip decoding, or why 16-bit mode only understands `[bx]`, read on
— each of those decisions gets a chapter.

## Source of truth

This book is generated from the project's own Git history and version table,
not from memory. Dates are taken from annotated tags where they exist; some
early versions were bundled into later tagged builds and are dated
approximately from surrounding commits.
