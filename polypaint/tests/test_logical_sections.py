import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


from logical_sections import (
    build_native_multispan_manifest,
    build_solve_source_manifest,
    build_source_spans,
)


class TestLogicalSections(unittest.TestCase):
    def _chunk_items(self):
        degree = 2
        n_coeffs = 3
        root_row_bytes = degree * 2 * 4
        coeff_row_bytes = n_coeffs * 2 * 4
        param_row_bytes = 16
        return [
            {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 3,
                "bin_key": "renders/j/chunk_0.bin",
                "coeffs_key": "renders/j/coeffs_0000.bin",
                "params_key": "renders/j/params_0000.bin",
                "bin_size": 3 * root_row_bytes,
                "coeffs_bin_size": 3 * coeff_row_bytes,
                "params_bin_size": 3 * param_row_bytes,
                "params_step_start": 0,
                "params_step_count": 3,
            },
            {
                "chunk_idx": 1,
                "step_start": 3,
                "step_count": 2,
                "bin_key": "renders/j/chunk_1.bin",
                "bin_size": 2 * root_row_bytes,
            },
            {
                "chunk_idx": 2,
                "step_start": 5,
                "step_count": 4,
                "bin_key": "renders/j/chunk_2.bin",
                "coeffs_key": "renders/j/coeffs_0002.bin",
                "params_key": "renders/j/params_0002.bin",
                "bin_size": 4 * root_row_bytes,
                "coeffs_bin_size": 4 * coeff_row_bytes,
                "params_bin_size": 4 * param_row_bytes,
                "params_step_start": 10,
                "params_step_count": 4,
            },
        ]

    def test_build_solve_source_manifest_handles_partial_source_coverage(self):
        manifest = build_solve_source_manifest(
            self._chunk_items(),
            job_id="j",
            degree=2,
            n_coeffs=3,
        )

        self.assertEqual(manifest["v"], 2)
        self.assertEqual(manifest["j"], "j")
        self.assertEqual(manifest["t"], 9)
        self.assertEqual(manifest["s"]["slv"]["r"], 16)
        self.assertEqual(manifest["s"]["cf"]["r"], 24)
        self.assertEqual(manifest["s"]["pm"]["r"], 16)

        slv_segments = manifest["s"]["slv"]["g"]
        cf_source = manifest["s"]["cf"]
        pm_source = manifest["s"]["pm"]
        cf_segments = cf_source["g"]
        pm_segments = pm_source["g"]

        self.assertEqual([row[2] for row in slv_segments], [3, 2, 4])
        self.assertEqual([cf_source["k"][row[0]] for row in cf_segments], ["renders/j/coeffs_0000.bin", "renders/j/coeffs_0002.bin"])
        self.assertEqual([pm_source["k"][row[0]] for row in pm_segments], ["renders/j/params_0000.bin", "renders/j/params_0002.bin"])
        self.assertEqual(pm_segments[1][3], 10)

    def test_build_source_spans_inside_single_segment(self):
        manifest = build_solve_source_manifest(self._chunk_items(), job_id="j", degree=2, n_coeffs=3)

        spans = build_source_spans(
            manifest,
            source_family="slv",
            solve_start=1,
            solve_count=2,
        )

        self.assertEqual(len(spans), 1)
        self.assertEqual(spans[0]["key"], "renders/j/chunk_0.bin")
        self.assertEqual(spans[0]["solve_start"], 1)
        self.assertEqual(spans[0]["solve_count"], 2)
        self.assertEqual(spans[0]["local_solve_start"], 0)
        self.assertEqual(spans[0]["byte_start"], 16)
        self.assertEqual(spans[0]["byte_length"], 32)

    def test_build_source_spans_crosses_three_segments(self):
        manifest = build_solve_source_manifest(self._chunk_items(), job_id="j", degree=2, n_coeffs=3)

        spans = build_source_spans(
            manifest,
            source_family="slv",
            solve_start=2,
            solve_count=5,
        )

        self.assertEqual(len(spans), 3)
        self.assertEqual([span["key"] for span in spans], [
            "renders/j/chunk_0.bin",
            "renders/j/chunk_1.bin",
            "renders/j/chunk_2.bin",
        ])
        self.assertEqual([span["local_solve_start"] for span in spans], [0, 1, 3])
        self.assertEqual([span["byte_start"] for span in spans], [2 * 16, 0, 0])
        self.assertEqual([span["byte_length"] for span in spans], [16, 32, 32])

    def test_build_source_spans_handles_zero_count_and_out_of_range(self):
        manifest = build_solve_source_manifest(self._chunk_items(), job_id="j", degree=2, n_coeffs=3)

        cases = [
            {"solve_start": 0, "solve_count": 0, "label": "zero-count"},
            {"solve_start": 20, "solve_count": 4, "label": "past-end"},
        ]
        for case in cases:
            with self.subTest(case["label"]):
                spans = build_source_spans(
                    manifest,
                    source_family="slv",
                    solve_start=case["solve_start"],
                    solve_count=case["solve_count"],
                )
                self.assertEqual(spans, [])

    def test_build_source_spans_section_start_before_first_segment(self):
        manifest = {
            "version": 1,
            "job_id": "j",
            "total_solves": 8,
            "degree": 1,
            "n_coeffs": 0,
            "sources": {
                "slv": {
                    "row_bytes": 8,
                    "segments": [
                        {
                            "storage_id": "slv_0000",
                            "key": "renders/j/chunk_0.bin",
                            "solve_start": 5,
                            "solve_count": 3,
                            "source_solve_start": 0,
                            "byte_size": 24,
                        }
                    ],
                }
            },
        }

        spans = build_source_spans(
            manifest,
            source_family="slv",
            solve_start=3,
            solve_count=4,
        )

        self.assertEqual(len(spans), 1)
        self.assertEqual(spans[0]["local_solve_start"], 2)
        self.assertEqual(spans[0]["byte_start"], 0)
        self.assertEqual(spans[0]["byte_length"], 16)

    def test_build_native_multispan_manifest_requires_urls_for_all_spans(self):
        manifest = build_solve_source_manifest(self._chunk_items(), job_id="j", degree=2, n_coeffs=3)
        with self.assertRaises(RuntimeError) as ctx:
            build_native_multispan_manifest(
                manifest,
                source_family="slv",
                solve_start=2,
                solve_count=5,
                url_by_key={"renders/j/chunk_0.bin": "https://example.com/chunk_0.bin"},
            )
        self.assertIn("Missing presigned URL", str(ctx.exception))
        self.assertIn("renders/j/chunk_1.bin", str(ctx.exception))

    def test_build_native_multispan_manifest_rejects_empty_span_selection(self):
        manifest = build_solve_source_manifest(self._chunk_items(), job_id="j", degree=2, n_coeffs=3)
        with self.assertRaises(RuntimeError) as ctx:
            build_native_multispan_manifest(
                manifest,
                source_family="slv",
                solve_start=100,
                solve_count=5,
                url_by_key={},
            )
        self.assertIn("produced no spans", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
