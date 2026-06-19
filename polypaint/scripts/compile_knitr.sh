#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  scripts/compile_knitr.sh path/to/report.Rnw

Compiles a knitr/LaTeX report and writes the final PDF to:
  reports/pdf/<report-name>.pdf

Intermediate TeX, aux, log, and latexmk files are written to a temporary
build directory under tmp/ and removed after the build.
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -ne 1 ]]; then
  usage >&2
  exit 2
fi

script_dir="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(CDPATH= cd -- "$script_dir/.." && pwd)"
input_path="$1"

if [[ ! -f "$input_path" ]]; then
  echo "error: input file not found: $input_path" >&2
  exit 1
fi

input_dir="$(CDPATH= cd -- "$(dirname -- "$input_path")" && pwd)"
input_file="$(basename -- "$input_path")"
input_abs="$input_dir/$input_file"
input_ext="${input_file##*.}"
input_ext_lc="$(printf '%s' "$input_ext" | tr '[:upper:]' '[:lower:]')"

case "$input_ext_lc" in
  rnw|rtex|tex)
    ;;
  *)
    echo "error: expected a knitr LaTeX file ending in .Rnw, .Rtex, or .tex" >&2
    exit 1
    ;;
esac

if ! command -v Rscript >/dev/null 2>&1; then
  echo "error: Rscript is not on PATH" >&2
  exit 1
fi

if ! command -v latexmk >/dev/null 2>&1; then
  echo "error: latexmk is not on PATH" >&2
  exit 1
fi

cd "$project_root"
mkdir -p "$project_root/reports/pdf" "$project_root/tmp"

base_name="${input_file%.*}"
safe_base="$(printf '%s' "$base_name" | tr -c 'A-Za-z0-9._-' '_')"
build_dir="$(mktemp -d "$project_root/tmp/knitr-${safe_base}.XXXXXX")"
tex_file="$build_dir/$safe_base.tex"
pdf_file="$build_dir/$safe_base.pdf"
final_pdf="$project_root/reports/pdf/$safe_base.pdf"

cleanup() {
  rm -rf "$build_dir"
  side_output="$input_dir/output"
  if [[ -d "$side_output" ]]; then
    non_junk_file="$(find "$side_output" -type f ! -name '.DS_Store' -print -quit)"
    if [[ -z "$non_junk_file" ]]; then
      rm -rf "$side_output"
    fi
  fi
}
trap cleanup EXIT

Rscript --vanilla -e 'args <- commandArgs(TRUE); if (!requireNamespace("knitr", quietly = TRUE)) stop("R package not installed: knitr", call. = FALSE); knitr::opts_knit$set(root.dir = getwd()); knitr::knit(input = args[[1]], output = args[[2]], quiet = FALSE)' "$input_abs" "$tex_file"

latexmk -pdf -interaction=nonstopmode -halt-on-error -outdir="$build_dir" "$tex_file"

if [[ ! -f "$pdf_file" ]]; then
  echo "error: expected PDF was not created: $pdf_file" >&2
  exit 1
fi

mv "$pdf_file" "$final_pdf"
echo "pdf=$final_pdf"
