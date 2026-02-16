#!/usr/bin/env python3
"""
Capture screenshots for the PolyPaint Interface Guide.

Usage:
    python docs/capture_interface.py

Generates PNG screenshots in docs/images/ for each UI section.
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

        def shot(name, selector=None, **kwargs):
            path = str(IMG_DIR / f"{name}.png")
            if selector:
                page.locator(selector).screenshot(path=path, **kwargs)
            else:
                page.screenshot(path=path, **kwargs)
            print(f"  saved {name}.png")

        # ── Layout Overview: full app screenshot ──
        print("Layout overview")
        shot("iface_layout")

        # ── Header Bar ──
        print("Header bar")
        shot("iface_header", "#header")

        # ── Left Panel: Coefficients Tab (default) ──
        print("Coefficients tab")
        shot("iface_coeffs_tab", "#left-panel")

        # ── Animation Bar (trajectory editor + controls) ──
        print("Animation bar / trajectory editor")
        # Select all coefficients and set circle path so controls are visible
        page.evaluate("""() => {
            document.getElementById('select-all-coeffs-btn').click();
        }""")
        page.wait_for_timeout(300)
        page.evaluate("""() => {
            const sel = document.getElementById('anim-path');
            sel.value = 'circle';
            sel.dispatchEvent(new Event('change'));
        }""")
        page.wait_for_timeout(300)
        shot("iface_anim_bar", "#anim-bar")

        # ── Coefficients Tab with paths visible ──
        print("Coefficients tab with circle paths")
        shot("iface_coeffs_paths", "#left-panel")

        # ── List Tab ──
        print("List tab")
        page.evaluate("""() => {
            document.querySelector('.tab[data-ltab="list"]').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_list_tab", "#left-panel")

        # ── List Tab curve editor ──
        print("List curve editor")
        page.evaluate("""() => {
            document.getElementById('list-select-all-btn').click();
        }""")
        page.wait_for_timeout(200)
        shot("iface_list_editor", "#list-curve-editor")

        # ── D-List Tab ──
        print("D-List tab")
        page.evaluate("""() => {
            document.querySelector('.tab[data-ltab="dlist"]').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_dlist_tab", "#left-panel")

        # ── Morph Tab ──
        print("Morph tab")
        page.evaluate("""() => {
            document.querySelector('.tab[data-ltab="final"]').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_morph_tab", "#left-panel")

        # ── Mid-bar Operations ──
        print("Mid-bar operations")
        # Switch back to coefficients tab for context
        page.evaluate("""() => {
            document.querySelector('.tab[data-ltab="coeffs"]').click();
        }""")
        page.wait_for_timeout(200)
        shot("iface_midbar", ".mid-bar")

        # ── Roots Tab with toolbar ──
        print("Roots tab + toolbar")
        # Full right-side panel screenshot via full page then crop would be complex.
        # Instead take the full page and let the roots panel be visible.
        shot("iface_roots_tab")
        # Also capture just the roots toolbar
        shot("iface_roots_toolbar", "#roots-toolbar")

        # ── Root color popup ──
        print("Root color popup")
        page.evaluate("""() => {
            document.getElementById('root-color-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_root_color_pop")
        # Close it
        page.evaluate("""() => {
            const pop = document.getElementById('color-pop');
            if (pop) pop.classList.remove('open');
        }""")
        page.wait_for_timeout(200)

        # ── Domain coloring ──
        print("Domain coloring")
        page.evaluate("""() => {
            document.getElementById('domain-toggle-btn').click();
        }""")
        page.wait_for_timeout(500)
        shot("iface_domain_coloring")
        # Turn it off
        page.evaluate("""() => {
            document.getElementById('domain-toggle-btn').click();
        }""")
        page.wait_for_timeout(300)

        # ── Export / Snap popup ──
        print("Export popup")
        page.evaluate("""() => {
            document.getElementById('snap-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_export_popup")
        page.evaluate("""() => {
            const pop = document.getElementById('snap-pop');
            if (pop) pop.classList.remove('open');
        }""")
        page.wait_for_timeout(200)

        # ── Stats Tab ──
        print("Stats tab")
        # First start animation briefly so stats have some data
        page.evaluate("""() => {
            document.getElementById('play-btn').click();
        }""")
        page.wait_for_timeout(2000)
        page.evaluate("""() => {
            document.getElementById('play-btn').click();
        }""")
        page.wait_for_timeout(300)
        page.evaluate("""() => {
            document.querySelector('.tab[data-tab="stats"]').click();
        }""")
        page.wait_for_timeout(500)
        shot("iface_stats_tab")

        # ── Sound Tab ──
        print("Sound tab")
        page.evaluate("""() => {
            document.querySelector('.tab[data-tab="sound"]').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_sound_tab")

        # ── Bitmap Tab (uninitialized) ──
        print("Bitmap tab")
        page.evaluate("""() => {
            document.querySelector('.tab[data-tab="bitmap"]').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_bitmap_tab")

        # ── Bitmap Tab: init + toolbar ──
        print("Bitmap toolbar (initialized)")
        page.evaluate("""() => {
            document.getElementById('bitmap-start-btn').click();
        }""")
        page.wait_for_timeout(500)
        shot("iface_bitmap_toolbar", "#bitmap-toolbar")

        # ── Bitmap cfg popup ──
        print("Bitmap cfg popup")
        page.evaluate("""() => {
            document.getElementById('bitmap-cfg-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_bitmap_cfg")
        # Close cfg
        page.evaluate("""() => {
            const pop = document.getElementById('bitmap-cfg-pop');
            if (pop) pop.classList.remove('open');
        }""")
        page.wait_for_timeout(200)

        # ── Bitmap save popup ──
        print("Bitmap save popup")
        page.evaluate("""() => {
            document.getElementById('bitmap-save-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_bitmap_save")
        page.evaluate("""() => {
            const pop = document.getElementById('bitmap-save-pop');
            if (pop) pop.classList.remove('open');
        }""")
        page.wait_for_timeout(200)

        # ── Bitmap fast mode: render for a few seconds ──
        print("Bitmap rendering")
        page.evaluate("""() => {
            document.getElementById('bitmap-fast-btn').click();
        }""")
        page.wait_for_timeout(3000)
        page.evaluate("""() => {
            document.getElementById('bitmap-fast-btn').click();
        }""")
        page.wait_for_timeout(500)
        shot("iface_bitmap_rendered")

        # ── Selection demo: go back to roots, select a few ──
        print("Selection demo")
        page.evaluate("""() => {
            document.querySelector('.tab[data-tab="roots"]').click();
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            document.getElementById('select-all-roots-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_selection")

        # ── Trails running ──
        print("Trails running")
        page.evaluate("""() => {
            document.getElementById('deselect-all-roots-btn').click();
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            document.getElementById('trails-btn').click();
        }""")
        page.wait_for_timeout(200)
        page.evaluate("""() => {
            document.getElementById('play-btn').click();
        }""")
        page.wait_for_timeout(4000)
        page.evaluate("""() => {
            document.getElementById('play-btn').click();
        }""")
        page.wait_for_timeout(300)
        shot("iface_trails")

        browser.close()

    server.terminate()
    server.wait(timeout=5)
    count = len(list(IMG_DIR.glob("iface_*.png")))
    print(f"\nDone! {count} interface screenshots in {IMG_DIR}")


if __name__ == "__main__":
    capture_all()
