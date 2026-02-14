import pytest
import subprocess
import time
import socket
from pathlib import Path
from playwright.sync_api import sync_playwright


def _find_free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("", 0))
        return s.getsockname()[1]


@pytest.fixture(scope="session")
def server_port():
    return _find_free_port()


@pytest.fixture(scope="session")
def server(server_port):
    """Start a local HTTP server serving the project directory."""
    proc = subprocess.Popen(
        ["python", "-m", "http.server", str(server_port), "--directory",
         str(Path(__file__).parent.parent)],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    time.sleep(0.5)
    yield f"http://localhost:{server_port}"
    proc.terminate()
    proc.wait(timeout=5)


@pytest.fixture(scope="session")
def pw():
    p = sync_playwright().start()
    yield p
    p.stop()


@pytest.fixture(scope="session")
def browser(pw):
    b = pw.chromium.launch(headless=True)
    yield b
    b.close()


@pytest.fixture
def page(browser, server):
    p = browser.new_page()
    p.goto(f"{server}/index.html", wait_until="domcontentloaded")
    p.wait_for_timeout(500)  # let scripts initialize
    yield p
    p.close()


@pytest.fixture(scope="session")
def wasm_b64():
    """Load WASM solver base64 string (or None if not built)."""
    path = Path(__file__).parent.parent / "solver.wasm.b64"
    if path.exists():
        return path.read_text().strip()
    return None
