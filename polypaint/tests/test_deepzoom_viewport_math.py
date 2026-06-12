import json
import math
import subprocess
import unittest
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
INDEX_HTML = ROOT / "index.html"

_DEEPZOOM_JS_FUNCTIONS = [
    "_formatDeepZoomViewportNumber",
    "_clampDeepZoomViewportCoord",
    "_rotateDeepZoomPoint",
    "_deepZoomCameraRectFromImageRect",
    "_deepZoomRenderBoundsFromCameraRect",
    "_computeDeepZoomVisibleBounds",
]


def _extract_function(source, name):
    marker = f"function {name}("
    start = source.find(marker)
    if start < 0:
        raise RuntimeError(f"could not find {name} in index.html")
    brace = source.find("{", start)
    if brace < 0:
        raise RuntimeError(f"could not find opening brace for {name}")
    depth = 0
    for idx in range(brace, len(source)):
        ch = source[idx]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[start:idx + 1]
    raise RuntimeError(f"could not find closing brace for {name}")


def _frontend_source():
    """index.html plus its js/ parts in script-tag order (what the browser runs)."""
    html = INDEX_HTML.read_text(encoding="utf-8")
    parts = re.findall(r'<script src="js/([^"?]+\.js)"></script>', html)
    return html + "\n" + "\n".join(
        (ROOT / "js" / name).read_text(encoding="utf-8") for name in parts
    )


def _deepzoom_js_source():
    source = _frontend_source()
    return "\n\n".join(_extract_function(source, name) for name in _DEEPZOOM_JS_FUNCTIONS)


def _encode_js_payload(value):
    if isinstance(value, float):
        if math.isnan(value):
            return {"__js_num__": "NaN"}
        if math.isinf(value):
            return {"__js_num__": "Infinity" if value > 0 else "-Infinity"}
        return value
    if isinstance(value, dict):
        return {key: _encode_js_payload(item) for key, item in value.items()}
    if isinstance(value, list):
        return [_encode_js_payload(item) for item in value]
    return value


def _run_deepzoom_js(op, **payload):
    js = _deepzoom_js_source()
    script = f"""
function reviveJsNumbers(value) {{
  if (Array.isArray(value)) return value.map(reviveJsNumbers);
  if (value && typeof value === 'object') {{
    if (value.__js_num__ === 'NaN') return NaN;
    if (value.__js_num__ === 'Infinity') return Infinity;
    if (value.__js_num__ === '-Infinity') return -Infinity;
    const out = {{}};
    for (const [key, item] of Object.entries(value)) out[key] = reviveJsNumbers(item);
    return out;
  }}
  return value;
}}
const payload = reviveJsNumbers(JSON.parse(require('fs').readFileSync(0, 'utf8')));
{js}

let result = null;
switch (payload.op) {{
  case 'format':
    result = _formatDeepZoomViewportNumber(payload.value);
    break;
  case 'camera':
    result = _deepZoomCameraRectFromImageRect(payload.meta, payload.imageRect);
    break;
  case 'render':
    result = _deepZoomRenderBoundsFromCameraRect(payload.meta, payload.cameraRect);
    break;
  case 'visible':
    result = _computeDeepZoomVisibleBounds(
      {{
        viewport: {{
          getBounds: () => payload.bounds,
          viewportToImageRectangle: () => payload.imageRect,
        }},
      }},
      payload.meta
    );
    break;
  default:
    throw new Error(`unknown op: ${{payload.op}}`);
}}
process.stdout.write(JSON.stringify(result));
"""
    proc = subprocess.run(
        ["node", "-e", script],
        input=json.dumps(_encode_js_payload({"op": op, **payload})),
        text=True,
        capture_output=True,
        cwd=ROOT,
        check=False,
    )
    if proc.returncode != 0:
        raise AssertionError(
            "deepzoom js invocation failed\n"
            f"stdout:\n{proc.stdout}\n"
            f"stderr:\n{proc.stderr}"
        )
    return json.loads(proc.stdout)


class TestDeepZoomViewportMath(unittest.TestCase):
    def test_square_viewport_maps_linearly(self):
        visible = _run_deepzoom_js(
            "camera",
            meta={
                "width": 4096,
                "height": 4096,
                "minRe": -2.0,
                "maxRe": 2.0,
                "minIm": -2.0,
                "maxIm": 2.0,
            },
            imageRect={
                "x": 1024,
                "y": 1024,
                "width": 2048,
                "height": 2048,
            },
        )
        self.assertAlmostEqual(visible["min_re"], -1.0)
        self.assertAlmostEqual(visible["max_re"], 1.0)
        self.assertAlmostEqual(visible["min_im"], -1.0)
        self.assertAlmostEqual(visible["max_im"], 1.0)

    def test_wide_viewport_uses_independent_x_and_y_spans(self):
        visible = _run_deepzoom_js(
            "camera",
            meta={
                "width": 4000,
                "height": 4000,
                "minRe": -4.0,
                "maxRe": 8.0,
                "minIm": -1.0,
                "maxIm": 2.0,
            },
            imageRect={
                "x": 1000,
                "y": 500,
                "width": 2000,
                "height": 1000,
            },
        )
        self.assertAlmostEqual(visible["min_re"], -1.0)
        self.assertAlmostEqual(visible["max_re"], 5.0)
        self.assertAlmostEqual(visible["max_im"], 1.625)
        self.assertAlmostEqual(visible["min_im"], 0.875)

    def test_tall_viewport_preserves_y_axis_inversion(self):
        visible = _run_deepzoom_js(
            "camera",
            meta={
                "width": 3000,
                "height": 6000,
                "minRe": -1.0,
                "maxRe": 2.0,
                "minIm": -6.0,
                "maxIm": 6.0,
            },
            imageRect={
                "x": 0,
                "y": 1500,
                "width": 1500,
                "height": 3000,
            },
        )
        self.assertAlmostEqual(visible["min_re"], -1.0)
        self.assertAlmostEqual(visible["max_re"], 0.5)
        self.assertAlmostEqual(visible["max_im"], 3.0)
        self.assertAlmostEqual(visible["min_im"], -3.0)

    def test_asymmetric_viewport_clamps_to_image_bounds(self):
        visible = _run_deepzoom_js(
            "camera",
            meta={
                "width": 1000,
                "height": 1000,
                "minRe": -3.5,
                "maxRe": 1.25,
                "minIm": -0.75,
                "maxIm": 2.0,
            },
            imageRect={
                "x": -100,
                "y": 750,
                "width": 500,
                "height": 500,
            },
        )
        self.assertAlmostEqual(visible["min_re"], -3.5)
        self.assertAlmostEqual(visible["max_re"], -1.125)
        self.assertAlmostEqual(visible["max_im"], -0.0625)
        self.assertAlmostEqual(visible["min_im"], -0.75)

    def test_rotated_viewport_converts_crop_back_into_render_bounds(self):
        visible = _run_deepzoom_js(
            "visible",
            meta={
                "width": 8192,
                "height": 8192,
                "minRe": -2.5,
                "maxRe": 2.5,
                "minIm": -2.5,
                "maxIm": 2.5,
                "rotation": -(math.pi / 2.0),
                "hasViewport": True,
            },
            bounds={"x": 0, "y": 0, "width": 1, "height": 1},
            imageRect={
                "x": 4096,
                "y": 0,
                "width": 4096,
                "height": 4096,
            },
        )
        self.assertAlmostEqual(visible["min_re"], -2.5)
        self.assertAlmostEqual(visible["max_re"], 0.0)
        self.assertAlmostEqual(visible["min_im"], 0.0)
        self.assertAlmostEqual(visible["max_im"], 2.5)

    def test_full_image_preserves_original_bounds_even_with_rotation(self):
        visible = _run_deepzoom_js(
            "visible",
            meta={
                "width": 4096,
                "height": 4096,
                "minRe": -2.0,
                "maxRe": 2.0,
                "minIm": -2.0,
                "maxIm": 2.0,
                "rotation": 0.25,
                "hasViewport": True,
            },
            bounds={"x": 0, "y": 0, "width": 1, "height": 1},
            imageRect={
                "x": 0,
                "y": 0,
                "width": 4096,
                "height": 4096,
            },
        )
        self.assertAlmostEqual(visible["min_re"], -2.0)
        self.assertAlmostEqual(visible["max_re"], 2.0)
        self.assertAlmostEqual(visible["min_im"], -2.0)
        self.assertAlmostEqual(visible["max_im"], 2.0)

    def test_compute_returns_null_without_valid_viewport_contract(self):
        visible = _run_deepzoom_js(
            "visible",
            meta={
                "width": 4096,
                "height": 4096,
                "minRe": -2.0,
                "maxRe": 2.0,
                "minIm": -2.0,
                "maxIm": 2.0,
                "rotation": 0.0,
                "hasViewport": False,
            },
            bounds={"x": 0, "y": 0, "width": 1, "height": 1},
            imageRect={
                "x": 0,
                "y": 0,
                "width": 4096,
                "height": 4096,
            },
        )
        self.assertIsNone(visible)

    def test_adaptive_number_formatting_comes_from_js(self):
        self.assertEqual(_run_deepzoom_js("format", value=0), "0")
        self.assertEqual(_run_deepzoom_js("format", value=0.125), "0.125000")
        self.assertEqual(_run_deepzoom_js("format", value=-3.5), "-3.500000")
        self.assertEqual(_run_deepzoom_js("format", value=1.234567e-8), "1.234567e-8")
        self.assertEqual(_run_deepzoom_js("format", value=float("inf")), "?")


if __name__ == "__main__":
    unittest.main()
