import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
_GENERATED = False


def ensure_generated_coeff_catalog():
    global _GENERATED
    if _GENERATED:
        return

    subprocess.run(
        ["cc", "-O2", "-pthread", "-o", str(LAMBDA_DIR / "sweep_test"), str(LAMBDA_DIR / "sweep_cli.c"), "-lm"],
        cwd=ROOT,
        check=True,
    )
    subprocess.run([sys.executable, str(LAMBDA_DIR / "gen_parity_results.py")], cwd=ROOT, check=True)
    subprocess.run([sys.executable, str(LAMBDA_DIR / "gen_catalog.py")], cwd=LAMBDA_DIR, check=True)
    _GENERATED = True
