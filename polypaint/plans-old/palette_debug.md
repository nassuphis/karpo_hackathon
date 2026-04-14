# Palette Debug

Status: legacy archive. Legacy helper, not the main UI workflow.

This file records the current role of the older palette-debug path.

## Current State

[lambda/handler_palette_debug.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_debug.py) still exists and is still tested, but it is no longer the primary user-facing Palette generation path.

The main UI now uses the asynchronous palette workflow described in [palette-render.md](/Users/nicknassuphis/karpo_hackathon/polypaint/palette-render.md).

That applies to:

- the Palette tab
- the Render tab `Palette` family

## What `handler_palette_debug.py` Still Is

It is a helper for:

- lores-based palette debugging
- direct/debug API use
- legacy tests
- optional persistent palette-variant creation outside the main workflow

It is not the authoritative implementation of the Palette tab anymore.

## Important Distinction

`handler_palette_debug.py` can still generate a full-size image artifact, but its underlying math is still the older lores-based debug path.

The asynchronous palette workflow is the exact full-resolution path.

## Current Recommendation

When documenting or reasoning about the user-facing Palette feature:

- use [palette-render.md](/Users/nicknassuphis/karpo_hackathon/polypaint/palette-render.md)

When documenting or reasoning about the legacy debug Lambda itself:

- use this file

## Tests

Coverage for the legacy debug handler remains in:

- [tests/test_palette_debug_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_debug_handler.py)
