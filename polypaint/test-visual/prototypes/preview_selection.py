"""Generate side-by-side C vs Python comparison PNGs for selected functions."""
import ast, json, os, subprocess, textwrap, numpy as np
from PIL import Image

LAMBDA_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "lambda"))
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
IMG_SIZE = 500
EXTENT = 2.0
N1, N2 = 100, 100

def load_poly_funcs(path, names):
    with open(path, "r") as f:
        source = f.read()
    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in names:
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.splitlines()[start:end]
            func_source = "\n".join(lines)
            module_code = textwrap.dedent("""\
                import math, cmath
                import numpy as np
                try:
                    from scipy.special import sph_harm
                except ImportError:
                    sph_harm = None
                pi = math.pi
            """) + "\n" + func_source
            ns = {}
            exec(compile(module_code, f"<{node.name}>", "exec"), ns)
            funcs[node.name] = ns[node.name]
    return funcs

def run_c_pipeline(func_name):
    coeffs_file = f"/tmp/{func_name}_coeffs.bin"
    roots_file = f"/tmp/{func_name}_roots.bin"
    spec = json.dumps({
        "mode": "coeffgen", "function": func_name,
        "n1": N1, "n2": N2, "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"], "times": 1, "dither_pass": 0,
    })
    result = subprocess.run([SWEEP, coeffs_file], input=spec, capture_output=True, text=True, timeout=30)
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed: {result.stderr[:200]}")
    meta1 = json.loads(result.stdout)
    spec2 = json.dumps({
        "mode": "solve", "coeffs_file": coeffs_file,
        "n_coeffs": meta1["n_coeffs"], "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1, "match_roots": False,
    })
    result2 = subprocess.run([SWEEP, roots_file], input=spec2, capture_output=True, text=True, timeout=60)
    if result2.returncode != 0:
        raise RuntimeError(f"solve failed: {result2.stderr[:200]}")
    meta2 = json.loads(result2.stdout)
    degree = meta2["degree"]
    n_t = meta2["n_t"]
    with open(roots_file, "rb") as f:
        data = f.read()
    expected = n_t * degree * 2 * 4
    floats = np.frombuffer(data[:expected], dtype=np.float32)
    return floats.reshape(n_t, degree, 2)

def run_python_pipeline(func):
    all_roots_re, all_roots_im = [], []
    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                coeffs = func(t1, t2)
            except Exception:
                continue
            if coeffs is None or len(coeffs) < 2:
                continue
            coeffs = coeffs[::-1]
            first_nz = next((k for k in range(len(coeffs)) if abs(coeffs[k]) > 1e-15), None)
            if first_nz is None:
                continue
            coeffs = coeffs[first_nz:]
            if len(coeffs) < 2:
                continue
            try:
                r = np.roots(coeffs)
            except Exception:
                continue
            all_roots_re.extend(r.real)
            all_roots_im.extend(r.imag)
    n = len(all_roots_re)
    if n == 0:
        return np.zeros((0, 1, 2), dtype=np.float32)
    roots = np.zeros((n, 1, 2), dtype=np.float32)
    roots[:, 0, 0] = np.array(all_roots_re, dtype=np.float32)
    roots[:, 0, 1] = np.array(all_roots_im, dtype=np.float32)
    return roots

def rasterize_color(roots_array, img_size=IMG_SIZE, extent=EXTENT):
    """Rasterize roots to RGB: brighter = more roots at that pixel."""
    counts = np.zeros((img_size, img_size), dtype=np.int32)
    re_vals = roots_array[:, :, 0].ravel()
    im_vals = roots_array[:, :, 1].ravel()
    px = ((re_vals + extent) / (2 * extent) * img_size).astype(np.int32)
    py = ((im_vals + extent) / (2 * extent) * img_size).astype(np.int32)
    mask = (px >= 0) & (px < img_size) & (py >= 0) & (py < img_size)
    for x, y in zip(px[mask], py[mask]):
        counts[y, x] += 1
    # Normalize to 0-255
    if counts.max() > 0:
        norm = np.minimum(counts * (255.0 / max(counts.max() * 0.3, 1)), 255).astype(np.uint8)
    else:
        norm = np.zeros((img_size, img_size), dtype=np.uint8)
    return norm

def make_comparison(name, c_roots, py_roots, overlap_pct):
    """Create side-by-side image: C (cyan) | Py (magenta), with overlap info."""
    c_img = rasterize_color(c_roots)
    py_img = rasterize_color(py_roots)

    # Create RGB images - C in cyan, Py in magenta
    h, w = c_img.shape
    gap = 10
    canvas = np.zeros((h + 30, w * 2 + gap, 3), dtype=np.uint8)
    canvas[:, :, :] = 20  # dark background

    # C image (cyan tint)
    canvas[:h, :w, 1] = c_img  # green
    canvas[:h, :w, 2] = c_img  # blue

    # Py image (magenta tint)
    canvas[:h, w + gap:w * 2 + gap, 0] = py_img  # red
    canvas[:h, w + gap:w * 2 + gap, 2] = py_img  # blue

    img = Image.fromarray(canvas)
    # Add label bar at bottom
    from PIL import ImageDraw, ImageFont
    draw = ImageDraw.Draw(img)
    try:
        font = ImageFont.truetype("/System/Library/Fonts/Menlo.ttc", 14)
    except Exception:
        font = ImageFont.load_default()
    label = f"{name}  |  C (cyan) vs Python (magenta)  |  Overlap: {overlap_pct:.1f}%"
    draw.text((10, h + 8), label, fill=(200, 200, 200), font=font)

    out_path = f"/tmp/{name}_compare.png"
    img.save(out_path)
    return out_path

# Selection of interesting cases
SELECTION = [
    # (func_name, source_file, description)
    ("poly_303", "poly400.py", "Perfect 100% - linspace worked"),
    ("poly_401", "poly500.py", "Perfect 100% - new transpiler"),
    ("poly_337", "poly400.py", "99.1% - great match"),
    ("poly_360", "poly400.py", "62.4% - float32 precision loss"),
    ("poly_489", "poly500.py", "59.8% - near miss"),
    ("poly_341", "poly400.py", "32.8% - structural mismatch"),
    ("poly_408", "poly500.py", "44.6% - dynamic range fail"),
    ("poly_345", "poly400.py", "0% - phantom: C=1, Py=103859"),
]

def main():
    # Load all needed functions
    py400_names = {s[0] for s in SELECTION if s[1] == "poly400.py"}
    py500_names = {s[0] for s in SELECTION if s[1] == "poly500.py"}

    py_funcs = {}
    if py400_names:
        py_funcs.update(load_poly_funcs(os.path.join(LAMBDA_DIR, "poly400.py"), py400_names))
    if py500_names:
        py_funcs.update(load_poly_funcs(os.path.join(LAMBDA_DIR, "poly500.py"), py500_names))

    paths = []
    for name, src, desc in SELECTION:
        print(f"Generating {name} ({desc})...")
        try:
            c_roots = run_c_pipeline(name)
            py_roots = run_python_pipeline(py_funcs[name])

            c_bool = np.any(c_roots != 0, axis=(1, 2)) if c_roots.size else np.array([])
            py_bool = np.any(py_roots != 0, axis=(1, 2)) if py_roots.size else np.array([])

            # Compute overlap
            c_rast = rasterize_color(c_roots) > 0
            py_rast = rasterize_color(py_roots) > 0
            intersection = np.sum(c_rast & py_rast)
            union = np.sum(c_rast | py_rast)
            overlap = 100.0 * intersection / union if union > 0 else 0.0

            path = make_comparison(name, c_roots, py_roots, overlap)
            paths.append(path)
            print(f"  -> {path}")
        except Exception as e:
            print(f"  ERROR: {e}")

    # Create a combined grid image
    if paths:
        imgs = [Image.open(p) for p in paths]
        # Stack vertically in pairs of 2
        cols = 2
        rows = (len(imgs) + cols - 1) // cols
        w = max(im.width for im in imgs)
        h = max(im.height for im in imgs)
        grid = Image.new("RGB", (w * cols + 4, h * rows + 4), (10, 10, 10))
        for i, im in enumerate(imgs):
            r, c = divmod(i, cols)
            grid.paste(im, (c * (w + 2), r * (h + 2)))
        grid_path = "/tmp/transpiler_preview.png"
        grid.save(grid_path)
        print(f"\nGrid saved to {grid_path}")

if __name__ == "__main__":
    main()
