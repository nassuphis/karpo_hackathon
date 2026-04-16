import os
import sys
import unittest
from unittest.mock import patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestComputeFusedSizing(unittest.TestCase):

    def _estimate(self, *, memory_mb, tmp_mb, timeout_sec):
        import compute_fused as mod

        with patch.dict(os.environ, {
            "FUSED_WORKER_MEMORY_MB": str(memory_mb),
            "FUSED_WORKER_TMP_MB": str(tmp_mb),
            "FUSED_WORKER_TIMEOUT_SEC": str(timeout_sec),
        }, clear=False):
            return mod.estimate_fused_chunking(
                n=5000,
                times=2,
                requested_chunks=1,
                degree=70,
                n_coeffs=71,
                fused_threads=8,
                solver_mode="aberth_mt",
                auto_hires_chunks=True,
            )

    def test_estimate_reports_memory_floor_when_memory_limits_first(self):
        estimate = self._estimate(memory_mb=512, tmp_mb=10240, timeout_sec=3600)
        self.assertEqual(estimate["safe_chunk_limit_reason"], "memory")
        self.assertEqual(estimate["min_safe_chunks"], estimate["memory_floor_chunks"])
        self.assertGreater(estimate["memory_floor_chunks"], estimate["tmp_floor_chunks"])
        self.assertGreater(estimate["memory_floor_chunks"], estimate["timeout_floor_chunks"])

    def test_estimate_reports_tmp_floor_when_tmp_limits_first(self):
        estimate = self._estimate(memory_mb=65536, tmp_mb=256, timeout_sec=3600)
        self.assertEqual(estimate["safe_chunk_limit_reason"], "/tmp")
        self.assertEqual(estimate["min_safe_chunks"], estimate["tmp_floor_chunks"])
        self.assertGreater(estimate["tmp_floor_chunks"], estimate["memory_floor_chunks"])
        self.assertGreater(estimate["tmp_floor_chunks"], estimate["timeout_floor_chunks"])

    def test_estimate_reports_timeout_floor_when_timeout_limits_first(self):
        estimate = self._estimate(memory_mb=65536, tmp_mb=65536, timeout_sec=30)
        self.assertEqual(estimate["safe_chunk_limit_reason"], "timeout")
        self.assertEqual(estimate["min_safe_chunks"], estimate["timeout_floor_chunks"])
        self.assertGreater(estimate["timeout_floor_chunks"], estimate["memory_floor_chunks"])
        self.assertGreater(estimate["timeout_floor_chunks"], estimate["tmp_floor_chunks"])


if __name__ == "__main__":
    unittest.main()
