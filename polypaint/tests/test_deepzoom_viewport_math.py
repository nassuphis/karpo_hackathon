import math
import unittest


def _format_viewport_number(value):
    num = float(value)
    if num == 0:
        return "0"
    mag = abs(num)
    return f"{num:.6f}" if 1e-4 <= mag < 1e6 else f"{num:.6e}"


def _map_visible_bounds(*, width, height, min_re, max_re, min_im, max_im, x, y, w, h):
    x0 = max(0.0, min(float(width), float(x)))
    x1 = max(0.0, min(float(width), x0 + float(w)))
    y0 = max(0.0, min(float(height), float(y)))
    y1 = max(0.0, min(float(height), y0 + float(h)))
    view_min_re = min_re + (x0 / width) * (max_re - min_re)
    view_max_re = min_re + (x1 / width) * (max_re - min_re)
    view_max_im = max_im - (y0 / height) * (max_im - min_im)
    view_min_im = max_im - (y1 / height) * (max_im - min_im)
    return {
        "min_re": view_min_re,
        "max_re": view_max_re,
        "min_im": view_min_im,
        "max_im": view_max_im,
    }


class TestDeepZoomViewportMath(unittest.TestCase):
    def test_square_viewport_maps_linearly(self):
        visible = _map_visible_bounds(
            width=4096,
            height=4096,
            min_re=-2.0,
            max_re=2.0,
            min_im=-2.0,
            max_im=2.0,
            x=1024,
            y=1024,
            w=2048,
            h=2048,
        )
        self.assertAlmostEqual(visible["min_re"], -1.0)
        self.assertAlmostEqual(visible["max_re"], 1.0)
        self.assertAlmostEqual(visible["min_im"], -1.0)
        self.assertAlmostEqual(visible["max_im"], 1.0)

    def test_wide_viewport_uses_independent_x_and_y_spans(self):
        visible = _map_visible_bounds(
            width=4000,
            height=4000,
            min_re=-4.0,
            max_re=8.0,
            min_im=-1.0,
            max_im=2.0,
            x=1000,
            y=500,
            w=2000,
            h=1000,
        )
        self.assertAlmostEqual(visible["min_re"], -1.0)
        self.assertAlmostEqual(visible["max_re"], 5.0)
        self.assertAlmostEqual(visible["max_im"], 1.625)
        self.assertAlmostEqual(visible["min_im"], 0.875)

    def test_tall_viewport_preserves_y_axis_inversion(self):
        visible = _map_visible_bounds(
            width=3000,
            height=6000,
            min_re=-1.0,
            max_re=2.0,
            min_im=-6.0,
            max_im=6.0,
            x=0,
            y=1500,
            w=1500,
            h=3000,
        )
        self.assertAlmostEqual(visible["min_re"], -1.0)
        self.assertAlmostEqual(visible["max_re"], 0.5)
        self.assertAlmostEqual(visible["max_im"], 3.0)
        self.assertAlmostEqual(visible["min_im"], -3.0)

    def test_asymmetric_viewport_clamps_to_image_bounds(self):
        visible = _map_visible_bounds(
            width=1000,
            height=1000,
            min_re=-3.5,
            max_re=1.25,
            min_im=-0.75,
            max_im=2.0,
            x=-100,
            y=750,
            w=500,
            h=500,
        )
        self.assertAlmostEqual(visible["min_re"], -3.5)
        self.assertAlmostEqual(visible["max_re"], -1.125)
        self.assertAlmostEqual(visible["max_im"], -0.0625)
        self.assertAlmostEqual(visible["min_im"], -0.75)

    def test_adaptive_number_formatting(self):
        self.assertEqual(_format_viewport_number(0), "0")
        self.assertEqual(_format_viewport_number(0.125), "0.125000")
        self.assertEqual(_format_viewport_number(-3.5), "-3.500000")
        self.assertEqual(_format_viewport_number(1.234567e-8), "1.234567e-08")


if __name__ == "__main__":
    unittest.main()
