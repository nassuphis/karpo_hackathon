#!/usr/bin/env bash
# Convert all WebM files in snaps/ to high-quality MP4, then delete the originals.
# Skips files that already have a corresponding .mp4.
# Usage: ./convert-videos.sh

set -euo pipefail

SNAPS_DIR="$(cd "$(dirname "$0")" && pwd)/snaps"

if ! command -v ffmpeg &>/dev/null; then
    echo "Error: ffmpeg not found. Install it first." >&2
    exit 1
fi

count=0
for webm in "$SNAPS_DIR"/*.webm; do
    [ -f "$webm" ] || continue
    mp4="${webm%.webm}.mp4"
    if [ -f "$mp4" ]; then
        echo "skip (mp4 exists): $(basename "$webm")"
        continue
    fi
    echo "converting: $(basename "$webm")"
    ffmpeg -y -i "$webm" -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p -movflags +faststart "$mp4" </dev/null 2>/dev/null
    rm "$webm"
    echo "  -> $(basename "$mp4") ($(du -h "$mp4" | cut -f1))"
    count=$((count + 1))
done

if [ "$count" -eq 0 ]; then
    echo "No unconverted WebM files found."
else
    echo "Converted $count file(s)."
fi
