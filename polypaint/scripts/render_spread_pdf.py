#!/usr/bin/env python3
"""Fetch a Color artifact from AWS and build a local ColorSpread PDF."""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "lambda"))

import boto3  # noqa: E402

from color_artifact_meta import load_color_artifact_head  # noqa: E402
from handler_pdf_artifact import _title_from, build_pdf_report_model  # noqa: E402
from shared import BUCKET  # noqa: E402
from spread_pdf import PDF_IMAGE_MAX_PX, PDF_PALETTE_MAX_PX, build_color_spread_pdf, prepare_pdf_image  # noqa: E402


def _write_json(path: Path, data) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True, default=str) + "\n")


def _download(s3, bucket: str, key: str, path: Path) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as fh:
        s3.download_fileobj(bucket, key, fh)
    return path


def _read_s3_json(s3, bucket: str, key: str):
    obj = s3.get_object(Bucket=bucket, Key=key)
    return json.loads(obj["Body"].read())


def _find_job_from_status(ddb, artifact_id: str, table_name: str) -> str:
    paginator = ddb.get_paginator("scan")
    job_ids: set[str] = set()
    for page in paginator.paginate(
        TableName=table_name,
        ProjectionExpression="job_id, task_id, result_data, error_msg",
    ):
        for item in page.get("Items", []) or []:
            blob = " ".join(
                str(item.get(name, {}).get("S", ""))
                for name in ("job_id", "task_id", "result_data", "error_msg")
            )
            if artifact_id not in blob:
                continue
            job_id = str(item.get("job_id", {}).get("S", "")).strip()
            if job_id:
                job_ids.add(job_id)
    if not job_ids:
        raise RuntimeError(
            f"Could not find job for {artifact_id!r} in DynamoDB table {table_name!r}. "
            "Pass the compute job id as the second argument."
        )
    compute_jobs = sorted(job_id for job_id in job_ids if job_id.startswith("compute_"))
    if len(compute_jobs) == 1:
        return compute_jobs[0]
    if len(compute_jobs) > 1:
        raise RuntimeError(
            f"Found multiple compute jobs for {artifact_id!r}: {', '.join(compute_jobs)}. "
            "Pass the desired compute job id as the second argument."
        )
    return sorted(job_ids)[0]


def _render_preview(pdf_path: Path) -> list[str]:
    pdftoppm = shutil.which("pdftoppm")
    if not pdftoppm:
        return []
    prefix = pdf_path.with_suffix("")
    subprocess.run(
        [pdftoppm, "-png", "-r", "120", str(pdf_path), str(prefix) + "_page"],
        check=True,
    )
    return sorted(str(path) for path in pdf_path.parent.glob(prefix.name + "_page-*.png"))


def _remove_paths(paths: list[Path]) -> list[str]:
    removed: list[str] = []
    for path in paths:
        try:
            if path.exists():
                path.unlink()
                removed.append(str(path))
        except IsADirectoryError:
            continue
    return removed


def _preview_paths_for(pdf_path: Path) -> list[Path]:
    prefix = pdf_path.with_suffix("")
    return sorted(pdf_path.parent.glob(prefix.name + "_page-*.png"))


def build_local_pdf(
    *,
    artifact_id: str,
    job_id: str,
    out_dir: Path,
    bucket: str,
    render_preview: bool,
    keep_work: bool,
) -> dict:
    s3 = boto3.client("s3")
    color_head = load_color_artifact_head(s3, bucket, job_id, artifact_id)
    image_key = color_head["image_key"]
    src_meta = dict(color_head.get("metadata") or {})
    calc_key = f"renders/{job_id}/calc.json"
    calc = _read_s3_json(s3, bucket, calc_key)

    out_dir.mkdir(parents=True, exist_ok=True)
    base = out_dir / f"{job_id}_{artifact_id}"
    image_ext = "." + image_key.rsplit(".", 1)[-1].lower()
    image_path = base.with_suffix(image_ext)
    prepared_image_path = base.with_name(base.name + "_prepared.jpg")
    pdf_path = base.with_suffix(".pdf")
    color_meta_path = base.with_name(base.name + "_color_meta.json")
    calc_path = base.with_name(base.name + "_calc.json")
    summary_path = base.with_name(base.name + "_summary.json")

    if not keep_work:
        _remove_paths(_preview_paths_for(pdf_path))

    _download(s3, bucket, image_key, image_path)
    _write_json(color_meta_path, src_meta)
    _write_json(calc_path, calc)

    palette_key = str(src_meta.get("associated_palette_image_key") or "").strip()
    palette_path = None
    prepared_palette_path = None
    if palette_key:
        palette_ext = "." + palette_key.rsplit(".", 1)[-1].lower()
        palette_path = base.with_name(base.name + "_palette").with_suffix(palette_ext)
        prepared_palette_path = base.with_name(base.name + "_palette_prepared.jpg")
        _download(s3, bucket, palette_key, palette_path)

    image_max_px = int(os.environ.get("PDF_IMAGE_MAX_PX", PDF_IMAGE_MAX_PX))
    palette_max_px = int(os.environ.get("PDF_PALETTE_MAX_PX", PDF_PALETTE_MAX_PX))
    source_image = prepare_pdf_image(
        image_path, prepared_image_path, max_px=image_max_px,
        quality=92, image_format="jpeg")
    palette_image = None
    if palette_path and prepared_palette_path:
        palette_image = prepare_pdf_image(
            palette_path, prepared_palette_path, max_px=palette_max_px,
            quality=92, image_format="jpeg")

    report = build_pdf_report_model(job_id, calc, src_meta, artifact_id)
    result = build_color_spread_pdf(
        prepared_image_path,
        pdf_path,
        report.get("title") or _title_from(calc, src_meta),
        report=report,
        palette_image_path=prepared_palette_path if prepared_palette_path else None,
    )
    preview_paths = _render_preview(pdf_path) if render_preview else []
    intermediate_paths = [
        image_path,
        prepared_image_path,
        color_meta_path,
        calc_path,
    ]
    if palette_path:
        intermediate_paths.append(palette_path)
    if prepared_palette_path:
        intermediate_paths.append(prepared_palette_path)

    summary = {
        "bucket": bucket,
        "job_id": job_id,
        "artifact_id": artifact_id,
        "image_key": image_key,
        "calc_key": calc_key,
        "palette_key": palette_key,
        "image_path": str(image_path),
        "prepared_image_path": str(prepared_image_path),
        "palette_path": str(palette_path) if palette_path else "",
        "prepared_palette_path": str(prepared_palette_path) if prepared_palette_path else "",
        "pdf_path": str(pdf_path),
        "preview_paths": preview_paths,
        "page_count": result.get("page_count") if isinstance(result, dict) else None,
        "source_image": source_image,
        "palette_image": palette_image,
        "report": report,
    }
    if keep_work:
        _write_json(summary_path, summary)
        summary["summary_path"] = str(summary_path)
        summary["deleted_paths"] = []
    else:
        deleted_paths = _remove_paths(intermediate_paths + [summary_path])
        summary["summary_path"] = ""
        summary["deleted_paths"] = deleted_paths
    return summary


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("artifact_id", help="Color artifact id, for example color_run_...")
    parser.add_argument("job_id", nargs="?", help="Optional compute job id. If omitted, DynamoDB is scanned.")
    parser.add_argument("--bucket", default=os.environ.get("BUCKET", BUCKET), help="S3 bucket name.")
    parser.add_argument(
        "--jobs-table",
        default=os.environ.get("JOBS_TABLE", "polypaint-jobs"),
        help="DynamoDB jobs/status table used to resolve the job id.",
    )
    parser.add_argument("--out-dir", default=str(ROOT / "test-pdf"), help="Output directory.")
    parser.add_argument("--render-preview", action="store_true", help="Also render PNG page previews with pdftoppm.")
    parser.add_argument(
        "--keep-work",
        action="store_true",
        help="Keep downloaded images, prepared images, and JSON sidecars for debugging.",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    job_id = str(args.job_id or "").strip()
    if not job_id:
        job_id = _find_job_from_status(boto3.client("dynamodb"), args.artifact_id, args.jobs_table)
    summary = build_local_pdf(
        artifact_id=args.artifact_id,
        job_id=job_id,
        out_dir=Path(args.out_dir),
        bucket=args.bucket,
        render_preview=bool(args.render_preview),
        keep_work=bool(args.keep_work),
    )
    print(json.dumps(summary, indent=2, sort_keys=True, default=str))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
