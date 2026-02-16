#!/usr/bin/env python3
"""
Capture screenshots for the PolyPaint tutorial.

Usage:
    python docs/capture_tutorial.py

Generates PNG screenshots in docs/images/ for each tutorial step.
Requires: playwright (pip install playwright && playwright install chromium)
"""

import subprocess
import socket
import time
from pathlib import Path
from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parent.parent
IMG_DIR = ROOT / "docs" / "images"
IMG_DIR.mkdir(exist_ok=True)


def find_free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("", 0))
        return s.getsockname()[1]


def capture_all():
    port = find_free_port()
    server = subprocess.Popen(
        ["python", "-m", "http.server", str(port), "--directory", str(ROOT)],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    time.sleep(0.5)
    url = f"http://localhost:{port}/index.html"

    with sync_playwright() as pw:
        browser = pw.chromium.launch(headless=True)
        page = browser.new_page(viewport={"width": 1280, "height": 720})
        page.goto(url, wait_until="domcontentloaded")
        page.wait_for_timeout(800)

        def shot(name, **kwargs):
            path = str(IMG_DIR / f"{name}.png")
            page.screenshot(path=path, **kwargs)
            print(f"  saved {name}.png")

        # ── Step 1: Initial state ──
        print("Step 1: Initial app state")
        shot("01_initial")

        # ── Step 2: Change pattern to Spiral (more interesting starting shape) ──
        print("Step 2: Select Spiral pattern")
        page.evaluate("""() => {
            const sel = document.getElementById('pattern');
            sel.value = 'spiral';
            sel.dispatchEvent(new Event('change'));
        }""")
        page.wait_for_timeout(400)
        shot("02_spiral_pattern")

        # ── Step 3: Select all coefficients ──
        print("Step 3: Select all coefficients")
        page.evaluate("""() => {
            document.getElementById('select-all-coeffs-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("03_select_all")

        # ── Step 4: Set path to Circle ──
        print("Step 4: Assign Circle path")
        page.evaluate("""() => {
            const sel = document.getElementById('anim-path');
            sel.value = 'circle';
            sel.dispatchEvent(new Event('change'));
        }""")
        page.wait_for_timeout(300)
        shot("04_circle_path")

        # ── Step 5: Apply PrimeSpeeds via the List tab ──
        print("Step 5: Apply PrimeSpeeds transform")
        page.evaluate("""() => {
            document.querySelector('.tab[data-ltab="list"]').click();
        }""")
        page.wait_for_timeout(300)
        page.evaluate("""() => {
            document.getElementById('list-select-all-btn').click();
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            const t = document.getElementById('list-transform');
            t.value = 'PrimeSpeeds';
            t.dispatchEvent(new Event('change'));
        }""")
        page.wait_for_timeout(300)
        # Switch back to Coefficients tab to show paths
        page.evaluate("""() => {
            document.querySelector('.tab[data-ltab="coeffs"]').click();
        }""")
        page.wait_for_timeout(300)
        shot("05_prime_speeds")

        # ── Step 6: Enable trails ──
        print("Step 6: Enable Trails")
        page.evaluate("""() => {
            document.getElementById('trails-btn').click();
        }""")
        page.wait_for_timeout(200)
        shot("06_trails_on")

        # ── Step 7: Play animation and let it run ──
        print("Step 7: Play animation (capturing after 3s)")
        page.evaluate("""() => {
            document.getElementById('play-btn').click();
        }""")
        page.wait_for_timeout(3000)
        shot("07_trails_running")

        # ── Step 8: Let trails build up more ──
        print("Step 8: Trails after 6s total")
        page.wait_for_timeout(3000)
        shot("08_trails_complete")

        # ── Step 9: Stop and show the export button ──
        print("Step 9: Stop animation, show Export popup")
        page.evaluate("""() => {
            document.getElementById('play-btn').click();  // pause
        }""")
        page.wait_for_timeout(300)
        page.evaluate("""() => {
            document.getElementById('snap-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("09_export_popup")

        # ── Step 10: Close export popup, switch to Bitmap tab ──
        print("Step 10: Bitmap tab")
        page.evaluate("""() => {
            if (typeof closeSnapPop === 'function') closeSnapPop();
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            document.querySelector('.tab[data-tab="bitmap"]').click();
        }""")
        page.wait_for_timeout(400)
        shot("10_bitmap_tab")

        # ── Step 11: Init bitmap ──
        print("Step 11: Initialize bitmap")
        page.evaluate("""() => {
            document.getElementById('bitmap-start-btn').click();
        }""")
        page.wait_for_timeout(500)
        shot("11_bitmap_init")

        # ── Step 12: Start fast mode and let it render ──
        print("Step 12: Fast mode rendering (5s)")
        page.evaluate("""() => {
            document.getElementById('bitmap-fast-btn').click();
        }""")
        page.wait_for_timeout(5000)
        shot("12_bitmap_rendering")

        # ── Step 13: Stop fast mode ──
        print("Step 13: Stop fast mode")
        page.evaluate("""() => {
            document.getElementById('bitmap-fast-btn').click();
        }""")
        page.wait_for_timeout(500)
        shot("13_bitmap_done")

        # ── Step 14: Open bitmap save popup ──
        print("Step 14: Bitmap save popup")
        page.evaluate("""() => {
            document.getElementById('bitmap-save-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("14_bitmap_save")

        # ── Step 15: Open bitmap cfg for color modes ──
        print("Step 15: Bitmap cfg popup")
        page.evaluate("""() => {
            // Close save popup first
            const savePop = document.getElementById('bitmap-save-pop');
            if (savePop) savePop.classList.remove('open');
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            document.getElementById('bitmap-cfg-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("15_bitmap_cfg")

        # ── Bonus: Roots panel close-up with domain coloring ──
        print("Step 16: Domain coloring")
        page.evaluate("""() => {
            const cfgPop = document.getElementById('bitmap-cfg-pop');
            if (cfgPop) cfgPop.classList.remove('open');
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            document.querySelector('.tab[data-tab="roots"]').click();
        }""")
        page.wait_for_timeout(300)
        page.evaluate("""() => {
            document.getElementById('domain-toggle-btn').click();
        }""")
        page.wait_for_timeout(500)
        shot("16_domain_coloring")

        browser.close()

    server.terminate()
    server.wait(timeout=5)
    print(f"\nDone! {len(list(IMG_DIR.glob('*.png')))} screenshots in {IMG_DIR}")


if __name__ == "__main__":
    capture_all()
