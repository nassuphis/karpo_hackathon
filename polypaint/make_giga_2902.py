#!/usr/bin/env python3
"""Standalone 5K reconstruction of the historical giga_2902 image.

This intentionally does not import the pyroots project. It embeds only the
saved giga_2902 calculation: rjail3, poly_chess5, sort_abs_p, numpy.roots,
the fixed saved view, and the historical rotate/invert raster operations.

The default output is giga_2909.png, as requested. At 5K resolution,
20 million roots preserves the original ratio of 2 billion roots to a
50K-by-50K raster.
"""

from __future__ import annotations

import argparse
import math
import multiprocessing as mp
from multiprocessing import shared_memory
import os
from pathlib import Path
import sys
import time
from textwrap import dedent

# Each process solves many tiny degree-33 eigenproblems. Nested BLAS threads add
# overhead and oversubscribe the machine when 14 Python workers are active.
for _name in (
    "OMP_NUM_THREADS",
    "OPENBLAS_NUM_THREADS",
    "MKL_NUM_THREADS",
    "VECLIB_MAXIMUM_THREADS",
    "NUMEXPR_NUM_THREADS",
):
    os.environ.setdefault(_name, "1")

import numpy as np
import pyvips


DEGREE = 33
DEFAULT_ROOTS = 20_000_000
DEFAULT_RESOLUTION = 5_000
DEFAULT_WORKERS = 14
DEFAULT_SEED = 2_902

VIEW_LOWER_LEFT = complex(-27.25697676487409, -18.351336173567184)
VIEW_UPPER_RIGHT = complex(9.165316633769502, 18.070957225076405)

A = 0.7
ANDY = 0.01
PHI = 0.0
RHO = 0.1
SPEED = 1.0

RJAIL3 = """
TTTTTTTTTTT
TSSSSSSSSST
TSSS   SSST
TS   S   ST      T
TSSS   SSST
TSSSSSSSSST
TTTTTTTTTTT
"""


def layout_to_coordinates(layout: str) -> tuple[np.ndarray, ...]:
    """Match pyroots.polylayout.layout2coord for S/T grid coordinates."""

    lines = dedent(layout).strip("\n").expandtabs(4).splitlines()
    width = max(len(line) for line in lines)
    grid = np.array([list(line.ljust(width)) for line in lines])
    rows, columns = grid.shape
    x = np.arange(columns, dtype=np.float64) - (columns - 1) / 2.0
    y = (rows - 1) / 2.0 - np.arange(rows, dtype=np.float64)
    xx, yy = np.meshgrid(x, y)
    source = grid == "S"
    target = grid == "T"
    return xx[source], yy[source], xx[target], yy[target]


SOURCE_X, SOURCE_Y, TARGET_X, TARGET_Y = layout_to_coordinates(RJAIL3)

if not (
    len(SOURCE_X) == len(SOURCE_Y) == len(TARGET_X) == len(TARGET_Y) == DEGREE
):
    raise RuntimeError("rjail3 must contain exactly 33 source and 33 target cells")


def bimodal_skewed(value: float, u: float) -> float:
    """Scalar form of pyroots.poly.bimodal_skewed."""

    if u < 0.5:
        result = (2.0 * u) ** (1.0 / (1.0 - value)) / 2.0
    else:
        result = 1.0 - (2.0 * (1.0 - u)) ** (1.0 / (1.0 - value)) / 2.0
    return float(np.clip(result, 0.0, 1.0))


def polynomial_roots(rng: np.random.RandomState) -> tuple[np.ndarray, bool]:
    """Generate and solve one historical poly_chess5 polynomial."""

    # polyflow.sample() drew these xfrm inputs before poly_chess5 discarded them.
    # Preserve the same random-consumption pattern even though the values are unused.
    rng.random_sample(2)
    t = float(rng.random_sample())
    blend_uniform = float(rng.random_sample())

    delta_source = RHO * np.exp(1j * 2.0 * math.pi * t)
    delta_target = RHO * np.exp(1j * 2.0 * math.pi * (t * SPEED + PHI))

    source_roots = (SOURCE_X + delta_source) + 1j * (SOURCE_Y + delta_source)
    target_roots = (TARGET_X + delta_target) + 1j * (TARGET_Y + delta_target)
    source_coefficients = np.poly(source_roots)
    target_coefficients = np.poly(target_roots)

    blend = bimodal_skewed(A, blend_uniform)
    coefficients = (
        target_coefficients * blend + source_coefficients * (1.0 - blend)
    )

    # zfrm.sort_abs_p: this is addition, not a 0.99/0.01 interpolation.
    coefficients = coefficients + ANDY * coefficients[
        np.argsort(np.abs(coefficients))
    ]

    try:
        return np.roots(coefficients), False
    except Exception:
        # Match solve.solve's historical failure policy.
        return np.zeros(DEGREE, dtype=np.complex128), True


def roots_to_pixels(
    roots: np.ndarray,
    resolution: int,
) -> tuple[np.ndarray, np.ndarray]:
    """Clip and map roots exactly as pyroots.polyview.pixels did."""

    lower = VIEW_LOWER_LEFT
    upper = VIEW_UPPER_RIGHT
    finite = np.isfinite(roots.real) & np.isfinite(roots.imag)
    inside = (
        finite
        & (roots.real >= lower.real)
        & (roots.real <= upper.real)
        & (roots.imag >= lower.imag)
        & (roots.imag <= upper.imag)
    )
    clipped = roots[inside]
    x = (
        (resolution - 1)
        * (clipped.real - lower.real)
        / (upper.real - lower.real)
    ).astype(np.intp)
    y = (
        (resolution - 1)
        - (resolution - 1)
        * (clipped.imag - lower.imag)
        / (upper.imag - lower.imag)
    ).astype(np.intp)
    return x, y


def worker(task: tuple[int, str, int, int, int]) -> dict[str, int | float]:
    worker_id, shm_name, resolution, samples, base_seed = task
    started = time.perf_counter()
    rng = np.random.RandomState((base_seed + worker_id * 1_000_003) % (2**32))
    shm = shared_memory.SharedMemory(name=shm_name)
    occupancy = np.ndarray(
        (resolution, resolution),
        dtype=np.uint8,
        buffer=shm.buf,
    )
    clipped_roots = 0
    solver_failures = 0
    progress_step = max(1, samples // 20)

    try:
        for sample_index in range(samples):
            roots, failed = polynomial_roots(rng)
            solver_failures += int(failed)
            x, y = roots_to_pixels(roots, resolution)
            clipped_roots += len(x)

            # All writers store the same value. Unlike the historical uint8
            # increment, this idempotent occupancy write cannot overflow.
            occupancy[y, x] = 1

            completed = sample_index + 1
            if worker_id == 0 and (
                completed == samples
                or (completed % progress_step == 0 and completed < samples)
            ):
                percent = 100.0 * completed / samples
                elapsed = time.perf_counter() - started
                print(
                    f"worker 0: {percent:5.1f}% ({completed:,}/{samples:,}) "
                    f"in {elapsed:.1f}s",
                    flush=True,
                )
    finally:
        shm.close()

    return {
        "worker_id": worker_id,
        "samples": samples,
        "roots": samples * DEGREE,
        "clipped_roots": clipped_roots,
        "solver_failures": solver_failures,
        "elapsed_seconds": time.perf_counter() - started,
    }


def render(
    *,
    output: Path,
    resolution: int,
    requested_roots: int,
    workers: int,
    seed: int,
    preview_size: int,
    force: bool,
) -> None:
    if resolution < 100:
        raise ValueError("resolution must be at least 100")
    if requested_roots < DEGREE * workers:
        raise ValueError("root count is too small for the requested worker count")
    if workers < 1:
        raise ValueError("workers must be positive")
    if preview_size < 0:
        raise ValueError("preview size must be zero or positive")
    if output.exists() and not force:
        raise FileExistsError(f"refusing to overwrite {output}; pass --force to replace it")

    preview = output.with_name(f"{output.stem}_preview.png")
    if preview_size > 0 and preview.exists() and not force:
        raise FileExistsError(
            f"refusing to overwrite {preview}; pass --force to replace it"
        )

    output.parent.mkdir(parents=True, exist_ok=True)
    samples_per_worker = requested_roots // DEGREE // workers
    actual_samples = samples_per_worker * workers
    actual_roots = actual_samples * DEGREE
    histogram_bytes = resolution * resolution

    print("giga_2902 standalone reconstruction", flush=True)
    print(f"output:          {output}", flush=True)
    print(f"resolution:      {resolution:,} x {resolution:,}", flush=True)
    print(f"workers:         {workers}", flush=True)
    print(f"samples/worker:  {samples_per_worker:,}", flush=True)
    print(f"actual samples:  {actual_samples:,}", flush=True)
    print(f"requested roots: {requested_roots:,}", flush=True)
    print(f"actual roots:    {actual_roots:,}", flush=True)
    print(f"shared map:      {histogram_bytes / 1_000_000:.1f} MB", flush=True)
    print(f"seed:            {seed}", flush=True)

    shm = shared_memory.SharedMemory(create=True, size=histogram_bytes)
    occupancy = np.ndarray(
        (resolution, resolution),
        dtype=np.uint8,
        buffer=shm.buf,
    )
    occupancy.fill(0)
    tasks = [
        (worker_id, shm.name, resolution, samples_per_worker, seed)
        for worker_id in range(workers)
    ]
    started = time.perf_counter()

    try:
        context = mp.get_context("spawn")
        with context.Pool(processes=workers) as pool:
            stats = pool.map(worker, tasks)

        occupied_pixels = int(np.count_nonzero(occupancy))
        image_bytes = np.multiply(occupancy, 255, dtype=np.uint8).tobytes()
    finally:
        shm.close()
        shm.unlink()

    image = pyvips.Image.new_from_memory(
        image_bytes,
        resolution,
        resolution,
        1,
        "uchar",
    )
    image = image.rotate(90)
    image = image.invert()
    image.write_to_file(str(output))

    if preview_size > 0:
        scale = preview_size / image.width
        image.resize(scale, vscale=preview_size / image.height).write_to_file(
            str(preview)
        )

    elapsed = time.perf_counter() - started
    clipped_roots = sum(int(item["clipped_roots"]) for item in stats)
    solver_failures = sum(int(item["solver_failures"]) for item in stats)
    slowest_worker = max(float(item["elapsed_seconds"]) for item in stats)
    occupancy_percent = 100.0 * occupied_pixels / (resolution * resolution)

    print(f"occupied pixels: {occupied_pixels:,} ({occupancy_percent:.3f}%)", flush=True)
    print(f"roots in view:   {clipped_roots:,}", flush=True)
    print(f"solve failures:  {solver_failures:,}", flush=True)
    print(f"slowest worker:  {slowest_worker:.1f}s", flush=True)
    print(f"total elapsed:   {elapsed:.1f}s", flush=True)
    print(f"wrote:           {output}", flush=True)
    if preview_size > 0:
        print(f"wrote:           {preview}", flush=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a standalone, reduced-resolution giga_2902 reconstruction."
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("giga_2909.png"),
        help="output PNG (default: giga_2909.png)",
    )
    parser.add_argument(
        "--resolution",
        type=int,
        default=DEFAULT_RESOLUTION,
        help=f"square raster size (default: {DEFAULT_RESOLUTION})",
    )
    parser.add_argument(
        "--roots",
        type=int,
        default=DEFAULT_ROOTS,
        help=f"requested root count (default: {DEFAULT_ROOTS})",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=DEFAULT_WORKERS,
        help=f"worker processes (default: {DEFAULT_WORKERS})",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=DEFAULT_SEED,
        help=f"deterministic base seed (default: {DEFAULT_SEED})",
    )
    parser.add_argument(
        "--preview-size",
        type=int,
        default=1_000,
        help="also write <stem>_preview.png at this size; 0 disables it",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="allow replacing an existing output and preview",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        render(
            output=args.output,
            resolution=args.resolution,
            requested_roots=args.roots,
            workers=args.workers,
            seed=args.seed,
            preview_size=args.preview_size,
            force=args.force,
        )
    except (FileExistsError, RuntimeError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    mp.freeze_support()
    raise SystemExit(main())
