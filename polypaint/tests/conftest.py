"""Pytest collection policy for the repository test suite."""

# The visual comparison files are standalone smoke scripts. They run expensive
# comparison loops at module import time, so collecting them as pytest modules
# makes `pytest` appear to hang before any test output is printed.
collect_ignore_glob = ["test_visual_*.py"]
