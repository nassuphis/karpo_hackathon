import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
DEPLOY = (ROOT / "deploy.sh").read_text()


class TestDeployPackaging(unittest.TestCase):
    def test_render_plan_zip_includes_palette_modules(self):
        self.assertIn(
            'cp lambda/handler_render_plan.py lambda/shared.py \\\n'
            '   lambda/palette_names.py lambda/tri_palette_names_generated.py "$PLAN_DIR/"',
            DEPLOY,
        )

    def test_palette_render_plan_zip_includes_palette_modules(self):
        self.assertIn(
            'cp lambda/handler_palette_render_plan.py lambda/shared.py \\\n'
            '   lambda/palette_names.py lambda/tri_palette_names_generated.py "$PAL_PLAN_DIR/"',
            DEPLOY,
        )

    def test_palette_debug_zip_includes_palette_modules(self):
        self.assertIn(
            'cp lambda/handler_palette_debug.py lambda/shared.py \\\n'
            '   lambda/palette_names.py lambda/tri_palette_names_generated.py "$PD_DIR/"',
            DEPLOY,
        )


if __name__ == "__main__":
    unittest.main()
