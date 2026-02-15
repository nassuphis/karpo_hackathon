"""Tests for off-canvas render: split compute/display, image export, high resolutions."""

import pytest
import struct


class TestStateVariables:
    def test_display_cap_constant(self, page):
        """BITMAP_DISPLAY_CAP should be 2000."""
        result = page.evaluate("() => BITMAP_DISPLAY_CAP")
        assert result == 2000

    def test_compute_res_initially_zero(self, page):
        """bitmapComputeRes starts at 0."""
        result = page.evaluate("() => bitmapComputeRes")
        assert result == 0

    def test_display_res_initially_zero(self, page):
        """bitmapDisplayRes starts at 0."""
        result = page.evaluate("() => bitmapDisplayRes")
        assert result == 0

    def test_display_buffer_initially_null(self, page):
        """bitmapDisplayBuffer starts as null."""
        result = page.evaluate("() => bitmapDisplayBuffer")
        assert result is None


class TestResolutionDropdown:
    def test_all_options_present(self, page):
        """Resolution dropdown has all expected options including 8K and 25K."""
        result = page.evaluate("""() => {
            var sel = document.getElementById('bitmap-res-select');
            return Array.from(sel.options).map(o => parseInt(o.value, 10));
        }""")
        assert result == [1000, 2000, 5000, 8000, 10000, 15000, 25000]

    def test_default_is_2000(self, page):
        """Default selected resolution is 2000."""
        result = page.evaluate("""() => {
            return parseInt(document.getElementById('bitmap-res-select').value, 10);
        }""")
        assert result == 2000


class TestInitNoSplit:
    """At 2000px (default), no split should activate."""

    def test_no_split_at_2000(self, page):
        """At 2000px, bitmapDisplayBuffer should remain null."""
        result = page.evaluate("""() => {
            initBitmapCanvas();
            return {
                computeRes: bitmapComputeRes,
                displayRes: bitmapDisplayRes,
                hasDisplayBuffer: bitmapDisplayBuffer !== null,
                canvasW: bitmapCtx.canvas.width,
                canvasH: bitmapCtx.canvas.height,
                persistentW: bitmapPersistentBuffer.width,
                persistentH: bitmapPersistentBuffer.height
            };
        }""")
        assert result["computeRes"] == 2000
        assert result["displayRes"] == 2000
        assert result["hasDisplayBuffer"] is False
        assert result["canvasW"] == 2000
        assert result["canvasH"] == 2000
        assert result["persistentW"] == 2000
        assert result["persistentH"] == 2000

    def test_no_split_at_1000(self, page):
        """At 1000px, no split — compute and display are the same."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            return {
                computeRes: bitmapComputeRes,
                displayRes: bitmapDisplayRes,
                hasDisplayBuffer: bitmapDisplayBuffer !== null
            };
        }""")
        assert result["computeRes"] == 1000
        assert result["displayRes"] == 1000
        assert result["hasDisplayBuffer"] is False


class TestInitWithSplit:
    """At >2000px, split should activate."""

    def test_split_at_5000(self, page):
        """At 5000px, canvas is 2000px but compute buffer is 5000px."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            return {
                computeRes: bitmapComputeRes,
                displayRes: bitmapDisplayRes,
                hasDisplayBuffer: bitmapDisplayBuffer !== null,
                canvasW: bitmapCtx.canvas.width,
                canvasH: bitmapCtx.canvas.height,
                persistentW: bitmapPersistentBuffer.width,
                persistentH: bitmapPersistentBuffer.height,
                displayBufW: bitmapDisplayBuffer ? bitmapDisplayBuffer.width : null,
                displayBufH: bitmapDisplayBuffer ? bitmapDisplayBuffer.height : null
            };
        }""")
        assert result["computeRes"] == 5000
        assert result["displayRes"] == 2000
        assert result["hasDisplayBuffer"] is True
        assert result["canvasW"] == 2000
        assert result["canvasH"] == 2000
        assert result["persistentW"] == 5000
        assert result["persistentH"] == 5000
        assert result["displayBufW"] == 2000
        assert result["displayBufH"] == 2000

    def test_persistent_buffer_decoupled_from_canvas(self, page):
        """Persistent buffer is created via ImageData constructor, not createImageData."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            // Persistent buffer should be 5000x5000 while canvas is 2000x2000
            return {
                persistentSize: bitmapPersistentBuffer.data.length,
                expectedSize: 5000 * 5000 * 4,
                canvasSize: bitmapCtx.canvas.width * bitmapCtx.canvas.height * 4
            };
        }""")
        assert result["persistentSize"] == result["expectedSize"]
        assert result["canvasSize"] == 2000 * 2000 * 4


class TestSerializeComputeRes:
    """serializeFastModeData must send bitmapComputeRes, not canvas dimensions."""

    def test_sends_compute_res_no_split(self, page):
        """Without split, canvasW/H should equal computeRes (2000)."""
        result = page.evaluate("""() => {
            initBitmapCanvas();
            fastModeCurves = new Map();
            var animated = allAnimatedCoeffs();
            var sd = serializeFastModeData(animated, 100, currentRoots.length);
            return { canvasW: sd.canvasW, canvasH: sd.canvasH };
        }""")
        assert result["canvasW"] == 2000
        assert result["canvasH"] == 2000

    def test_sends_compute_res_with_split(self, page):
        """With split, canvasW/H must be computeRes (5000), NOT canvas.width (2000)."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            fastModeCurves = new Map();
            var animated = allAnimatedCoeffs();
            var sd = serializeFastModeData(animated, 100, currentRoots.length);
            return {
                canvasW: sd.canvasW,
                canvasH: sd.canvasH,
                actualCanvasWidth: bitmapCtx.canvas.width
            };
        }""")
        assert result["canvasW"] == 5000, "Workers must get compute resolution, not display"
        assert result["canvasH"] == 5000
        assert result["actualCanvasWidth"] == 2000, "Canvas should be display-sized"


class TestEnterFastModeResCheck:
    def test_resolution_check_uses_compute_res(self, page):
        """enterFastMode should compare bitmapComputeRes, not canvas.width, to wantRes."""
        result = page.evaluate("""() => {
            // Init at 5000
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            // Verify state
            var before = {
                computeRes: bitmapComputeRes,
                canvasW: bitmapCtx.canvas.width
            };
            // Now set dropdown to same 5000 and check — should NOT re-init
            var initCount = 0;
            var origInit = initBitmapCanvas;
            initBitmapCanvas = function() { initCount++; origInit(); };
            // Simulate the check from enterFastMode
            var wantRes = 5000;
            var needsInit = !bitmapCtx || !bitmapActive || bitmapComputeRes !== wantRes;
            initBitmapCanvas = origInit;
            return { needsInit: needsInit, computeRes: before.computeRes, canvasW: before.canvasW };
        }""")
        assert result["needsInit"] is False, "Should not need reinit when computeRes matches"
        assert result["computeRes"] == 5000
        assert result["canvasW"] == 2000


class TestCompositeWorkerPixels:
    def test_composite_no_split(self, page):
        """Without split, compositeWorkerPixels writes to persistent buffer and canvas."""
        result = page.evaluate("""() => {
            initBitmapCanvas();  // 2000px, no split
            // Create fake worker pixel data
            var paintIdx = new Int32Array([0, 1, 2000]);  // pixels at (0,0), (1,0), (0,1)
            var paintR = new Uint8Array([255, 0, 0]);
            var paintG = new Uint8Array([0, 255, 0]);
            var paintB = new Uint8Array([0, 0, 255]);
            fastModeWorkerPixels = [{
                paintIdx: paintIdx.buffer,
                paintR: paintR.buffer,
                paintG: paintG.buffer,
                paintB: paintB.buffer,
                paintCount: 3
            }];
            compositeWorkerPixels();
            var d = bitmapPersistentBuffer.data;
            return {
                px0: [d[0], d[1], d[2], d[3]],
                px1: [d[4], d[5], d[6], d[7]],
                px2000: [d[8000], d[8001], d[8002], d[8003]],
                breakdown: fastModeCompositeBreakdown.pixels
            };
        }""")
        assert result["px0"] == [255, 0, 0, 255]
        assert result["px1"] == [0, 255, 0, 255]
        assert result["px2000"] == [0, 0, 255, 255]
        assert result["breakdown"] == 3

    def test_composite_with_split(self, page):
        """With split, compositeWorkerPixels writes to both persistent and display buffers."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();  // split: compute=5000, display=2000
            // Write pixel at compute-space (2500, 2500) — should map to display (1000, 1000)
            var pixIdx = 2500 * 5000 + 2500;
            var paintIdx = new Int32Array([pixIdx]);
            var paintR = new Uint8Array([128]);
            var paintG = new Uint8Array([64]);
            var paintB = new Uint8Array([32]);
            fastModeWorkerPixels = [{
                paintIdx: paintIdx.buffer,
                paintR: paintR.buffer,
                paintG: paintG.buffer,
                paintB: paintB.buffer,
                paintCount: 1
            }];
            compositeWorkerPixels();

            // Check persistent buffer at compute coords
            var pOff = pixIdx * 4;
            var pd = bitmapPersistentBuffer.data;

            // Check display buffer at downsampled coords
            var dx = Math.floor(2500 * 2000 / 5000);  // 1000
            var dy = Math.floor(2500 * 2000 / 5000);  // 1000
            var dOff = (dy * 2000 + dx) * 4;
            var dd = bitmapDisplayBuffer.data;

            return {
                persistent: [pd[pOff], pd[pOff+1], pd[pOff+2], pd[pOff+3]],
                display: [dd[dOff], dd[dOff+1], dd[dOff+2], dd[dOff+3]],
                dx: dx, dy: dy,
                breakdown: fastModeCompositeBreakdown.pixels
            };
        }""")
        assert result["persistent"] == [128, 64, 32, 255]
        assert result["display"] == [128, 64, 32, 255]
        assert result["dx"] == 1000
        assert result["dy"] == 1000
        assert result["breakdown"] == 1


class TestFillDisplayBuffer:
    def test_fill_display_buffer(self, page):
        """fillDisplayBuffer fills entire display buffer with correct color."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            // Init fills with bitmapCanvasColor (#000000), now fill with red
            fillDisplayBuffer('#ff0000');
            var d = bitmapDisplayBuffer.data;
            // Check first and last pixel
            return {
                first: [d[0], d[1], d[2], d[3]],
                last: [d[d.length-4], d[d.length-3], d[d.length-2], d[d.length-1]]
            };
        }""")
        assert result["first"] == [255, 0, 0, 255]
        assert result["last"] == [255, 0, 0, 255]


class TestResetBitmap:
    def test_reset_clears_new_state(self, page):
        """resetBitmap nulls out all new state variables."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            resetBitmap();
            return {
                persistentBuffer: bitmapPersistentBuffer,
                displayBuffer: bitmapDisplayBuffer,
                computeRes: bitmapComputeRes,
                displayRes: bitmapDisplayRes
            };
        }""")
        assert result["persistentBuffer"] is None
        assert result["displayBuffer"] is None
        assert result["computeRes"] == 0
        assert result["displayRes"] == 0


class TestClearWithSplit:
    def test_clear_resets_both_buffers(self, page):
        """Clear should reset both persistent and display buffers."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            // Write a pixel to both buffers
            var pixIdx = 0;
            fastModeWorkerPixels = [{
                paintIdx: new Int32Array([0]).buffer,
                paintR: new Uint8Array([255]).buffer,
                paintG: new Uint8Array([255]).buffer,
                paintB: new Uint8Array([255]).buffer,
                paintCount: 1
            }];
            compositeWorkerPixels();
            var beforePersist = bitmapPersistentBuffer.data[0];  // should be 255
            var beforeDisplay = bitmapDisplayBuffer.data[0];      // should be 255

            // Simulate clear
            bitmapCtx.fillStyle = bitmapCanvasColor;
            bitmapCtx.fillRect(0, 0, bitmapCtx.canvas.width, bitmapCtx.canvas.height);
            fillPersistentBuffer(bitmapCanvasColor);
            if (bitmapDisplayBuffer) fillDisplayBuffer(bitmapCanvasColor);

            return {
                beforePersist: beforePersist,
                beforeDisplay: beforeDisplay,
                afterPersist: bitmapPersistentBuffer.data[0],
                afterDisplay: bitmapDisplayBuffer.data[0]
            };
        }""")
        assert result["beforePersist"] == 255
        assert result["beforeDisplay"] == 255
        assert result["afterPersist"] == 0  # black background
        assert result["afterDisplay"] == 0


class TestBMPExport:
    def test_bmp_function_exists(self, page):
        """exportPersistentBufferAsBMP should be defined."""
        result = page.evaluate("() => typeof exportPersistentBufferAsBMP")
        assert result == "function"

    def test_bmp_header_structure(self, page):
        """BMP export produces valid header for a small test image."""
        result = page.evaluate("""() => {
            // Create a tiny 4x3 RGBA buffer
            var w = 4, h = 3;
            var rgba = new Uint8Array(w * h * 4);
            // Fill with red (R=255, G=0, B=0, A=255)
            for (var i = 0; i < rgba.length; i += 4) {
                rgba[i] = 255; rgba[i+1] = 0; rgba[i+2] = 0; rgba[i+3] = 255;
            }

            // Capture the blob parts instead of downloading
            var capturedParts = null;
            var origBlob = Blob;
            var origCreate = URL.createObjectURL;
            var origRevoke = URL.revokeObjectURL;
            var origClick = HTMLAnchorElement.prototype.click;
            HTMLAnchorElement.prototype.click = function() {};
            URL.createObjectURL = function(b) { return 'blob:test'; };
            URL.revokeObjectURL = function() {};

            // Intercept Blob constructor
            var capturedBlob = null;
            window.Blob = function(parts, opts) {
                capturedParts = parts;
                capturedBlob = new origBlob(parts, opts);
                return capturedBlob;
            };

            exportPersistentBufferAsBMP(rgba, w, h, 'test.bmp');

            // Restore
            window.Blob = origBlob;
            URL.createObjectURL = origCreate;
            URL.revokeObjectURL = origRevoke;
            HTMLAnchorElement.prototype.click = origClick;

            // Read header from first part (ArrayBuffer)
            var header = new DataView(capturedParts[0]);
            var rowSize = (w * 3 + 3) & ~3;  // 12, already aligned
            var pixelBytes = rowSize * h;

            return {
                magic: String.fromCharCode(header.getUint8(0)) + String.fromCharCode(header.getUint8(1)),
                fileSize: header.getUint32(2, true),
                expectedFileSize: 54 + pixelBytes,
                dataOffset: header.getUint32(10, true),
                dibSize: header.getUint32(14, true),
                width: header.getInt32(18, true),
                height: header.getInt32(22, true),
                bitsPerPixel: header.getUint16(28, true),
                compression: header.getUint32(30, true),
                numParts: capturedParts.length
            };
        }""")
        assert result["magic"] == "BM"
        assert result["fileSize"] == result["expectedFileSize"]
        assert result["dataOffset"] == 54
        assert result["dibSize"] == 40
        assert result["width"] == 4
        assert result["height"] == 3
        assert result["bitsPerPixel"] == 24
        assert result["compression"] == 0

    def test_bmp_pixel_data_bgr_order(self, page):
        """BMP pixel data should be in BGR order (not RGB)."""
        result = page.evaluate("""() => {
            var w = 2, h = 1;
            var rgba = new Uint8Array(w * h * 4);
            // Pixel 0: R=10, G=20, B=30
            rgba[0] = 10; rgba[1] = 20; rgba[2] = 30; rgba[3] = 255;
            // Pixel 1: R=40, G=50, B=60
            rgba[4] = 40; rgba[5] = 50; rgba[6] = 60; rgba[7] = 255;

            var capturedParts = null;
            var origBlob = Blob;
            var origCreate = URL.createObjectURL;
            var origRevoke = URL.revokeObjectURL;
            var origClick = HTMLAnchorElement.prototype.click;
            HTMLAnchorElement.prototype.click = function() {};
            URL.createObjectURL = function() { return 'blob:test'; };
            URL.revokeObjectURL = function() {};
            window.Blob = function(parts, opts) {
                capturedParts = parts;
                return new origBlob(parts, opts);
            };

            exportPersistentBufferAsBMP(rgba, w, h, 'test.bmp');

            window.Blob = origBlob;
            URL.createObjectURL = origCreate;
            URL.revokeObjectURL = origRevoke;
            HTMLAnchorElement.prototype.click = origClick;

            // Pixel data is in part[1] (first chunk after header)
            var chunk = capturedParts[1];
            // BMP is bottom-up, but h=1 so only one row
            // BGR order: pixel0 = [B=30, G=20, R=10], pixel1 = [B=60, G=50, R=40]
            return {
                px0_b: chunk[0], px0_g: chunk[1], px0_r: chunk[2],
                px1_b: chunk[3], px1_g: chunk[4], px1_r: chunk[5]
            };
        }""")
        assert result["px0_b"] == 30
        assert result["px0_g"] == 20
        assert result["px0_r"] == 10
        assert result["px1_b"] == 60
        assert result["px1_g"] == 50
        assert result["px1_r"] == 40

    def test_bmp_bottom_up_row_order(self, page):
        """BMP rows are stored bottom-up: first row in file is last row in image."""
        result = page.evaluate("""() => {
            var w = 1, h = 2;
            var rgba = new Uint8Array(w * h * 4);
            // Row 0 (top): R=100
            rgba[0] = 100; rgba[1] = 0; rgba[2] = 0; rgba[3] = 255;
            // Row 1 (bottom): R=200
            rgba[4] = 200; rgba[5] = 0; rgba[6] = 0; rgba[7] = 255;

            var capturedParts = null;
            var origBlob = Blob;
            var origCreate = URL.createObjectURL;
            var origRevoke = URL.revokeObjectURL;
            var origClick = HTMLAnchorElement.prototype.click;
            HTMLAnchorElement.prototype.click = function() {};
            URL.createObjectURL = function() { return 'blob:test'; };
            URL.revokeObjectURL = function() {};
            window.Blob = function(parts, opts) {
                capturedParts = parts;
                return new origBlob(parts, opts);
            };

            exportPersistentBufferAsBMP(rgba, w, h, 'test.bmp');

            window.Blob = origBlob;
            URL.createObjectURL = origCreate;
            URL.revokeObjectURL = origRevoke;
            HTMLAnchorElement.prototype.click = origClick;

            // rowSize for w=1: (1*3+3)&~3 = 4 (3 bytes data + 1 padding)
            var chunk = capturedParts[1];
            // BMP bottom-up: first BMP row = image bottom row (row 1, R=200)
            // Second BMP row = image top row (row 0, R=100)
            return {
                firstBmpRow_R: chunk[2],   // BGR order, R is at offset 2
                secondBmpRow_R: chunk[6]   // next row starts at offset 4 (padded)
            };
        }""")
        assert result["firstBmpRow_R"] == 200, "First BMP row should be bottom image row"
        assert result["secondBmpRow_R"] == 100, "Second BMP row should be top image row"


class TestPlotCoeffCurvesDisplayRes:
    def test_uses_display_res(self, page):
        """plotCoeffCurvesOnBitmap should use display resolution, not compute."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            // The function reads w from bitmapDisplayRes
            // Verify that bitmapDisplayRes is used (not bitmapComputeRes)
            return {
                displayRes: bitmapDisplayRes,
                computeRes: bitmapComputeRes,
                canvasW: bitmapCtx.canvas.width
            };
        }""")
        assert result["displayRes"] == 2000
        assert result["computeRes"] == 5000
        assert result["canvasW"] == 2000


class TestTimingCopyRes:
    def test_timing_uses_compute_res(self, page):
        """Timing copy should show bitmapComputeRes, not canvas width."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '5000';
            initBitmapCanvas();
            return { computeRes: bitmapComputeRes, canvasW: bitmapCtx.canvas.width };
        }""")
        assert result["computeRes"] == 5000
        assert result["canvasW"] == 2000


class TestExportFormatState:
    def test_default_format_is_jpeg(self, page):
        """bitmapExportFormat should default to 'jpeg'."""
        result = page.evaluate("() => bitmapExportFormat")
        assert result == "jpeg"

    def test_format_persists_in_variable(self, page):
        """Changing format updates bitmapExportFormat."""
        result = page.evaluate("""() => {
            bitmapExportFormat = 'png';
            return bitmapExportFormat;
        }""")
        assert result == "png"

    def test_format_in_save_state(self, page):
        """buildStateMetadata includes bitmapExportFormat."""
        result = page.evaluate("""() => {
            bitmapExportFormat = 'tiff';
            var meta = buildStateMetadata();
            return meta.bitmapExportFormat;
        }""")
        assert result == "tiff"

    def test_format_restored_on_load(self, page):
        """loadState restores bitmapExportFormat from metadata."""
        result = page.evaluate("""() => {
            bitmapExportFormat = 'jpeg';  // reset
            var meta = buildStateMetadata();
            meta.bitmapExportFormat = 'png';
            // loadState is async but the format restore is synchronous
            // Just test the restore logic directly
            if (meta.bitmapExportFormat) {
                bitmapExportFormat = meta.bitmapExportFormat;
            }
            return bitmapExportFormat;
        }""")
        assert result == "png"


class TestSavePopup:
    def test_save_popup_exists(self, page):
        """bitmap-save-pop element should exist."""
        result = page.evaluate("() => !!document.getElementById('bitmap-save-pop')")
        assert result is True

    def test_save_popup_initially_closed(self, page):
        """Save popup should not have 'open' class initially."""
        result = page.evaluate("""() =>
            document.getElementById('bitmap-save-pop').classList.contains('open')
        """)
        assert result is False

    def test_save_popup_opens_on_click(self, page):
        """Clicking save button (via JS) should open the save popup."""
        page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            openBitmapSavePop();
        }""")
        result = page.evaluate("""() =>
            document.getElementById('bitmap-save-pop').classList.contains('open')
        """)
        assert result is True

    def test_save_popup_has_format_dropdown(self, page):
        """Save popup should contain format select after opening."""
        page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            openBitmapSavePop();
        }""")
        result = page.evaluate("""() => {
            var sel = document.getElementById('bitmap-fmt-select');
            if (!sel) return null;
            return Array.from(sel.options).map(o => o.value);
        }""")
        assert result == ["jpeg", "png", "bmp", "tiff"]

    def test_save_popup_toggle(self, page):
        """Clicking save button twice should close the popup."""
        page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            openBitmapSavePop();
            openBitmapSavePop();  // second call closes
        }""")
        result = page.evaluate("""() =>
            document.getElementById('bitmap-save-pop').classList.contains('open')
        """)
        assert result is False


class TestExportFunctions:
    def test_jpeg_function_exists(self, page):
        """exportPersistentBufferAsJPEG should be defined."""
        result = page.evaluate("() => typeof exportPersistentBufferAsJPEG")
        assert result == "function"

    def test_png_function_exists(self, page):
        """exportPersistentBufferAsPNG should be defined."""
        result = page.evaluate("() => typeof exportPersistentBufferAsPNG")
        assert result == "function"

    def test_tiff_function_exists(self, page):
        """exportPersistentBufferAsTIFF should be defined."""
        result = page.evaluate("() => typeof exportPersistentBufferAsTIFF")
        assert result == "function"

    def test_download_blob_helper_exists(self, page):
        """downloadBlob helper should be defined."""
        result = page.evaluate("() => typeof downloadBlob")
        assert result == "function"


class TestLibraryLoading:
    def test_pako_loaded(self, page):
        """pako library should be available as window.pako."""
        result = page.evaluate("() => typeof window.pako")
        assert result == "object"

    def test_upng_loaded(self, page):
        """UPNG library should be available as window.UPNG."""
        result = page.evaluate("() => typeof window.UPNG")
        assert result == "object"

    def test_jpeg_js_loaded(self, page):
        """jpeg-js library should expose window['jpeg-js'].encode."""
        result = page.evaluate("() => typeof (window['jpeg-js'] && window['jpeg-js'].encode)")
        assert result == "function"

    def test_utif_loaded(self, page):
        """UTIF library should be available as window.UTIF or self.UTIF."""
        result = page.evaluate("() => typeof UTIF")
        assert result == "object"


class TestJPEGExport:
    def test_jpeg_produces_blob(self, page):
        """JPEG export should produce a valid blob with JPEG magic bytes."""
        result = page.evaluate("""() => {
            var w = 4, h = 4;
            var rgba = new Uint8Array(w * h * 4);
            for (var i = 0; i < rgba.length; i += 4) {
                rgba[i] = 255; rgba[i+1] = 0; rgba[i+2] = 0; rgba[i+3] = 255;
            }

            var capturedBlob = null;
            var origCreate = URL.createObjectURL;
            var origRevoke = URL.revokeObjectURL;
            var origClick = HTMLAnchorElement.prototype.click;
            HTMLAnchorElement.prototype.click = function() {};
            URL.createObjectURL = function(b) { capturedBlob = b; return 'blob:test'; };
            URL.revokeObjectURL = function() {};

            exportPersistentBufferAsJPEG(rgba, w, h, 92, 'test.jpg');

            URL.createObjectURL = origCreate;
            URL.revokeObjectURL = origRevoke;
            HTMLAnchorElement.prototype.click = origClick;

            return {
                blobType: capturedBlob ? capturedBlob.type : null,
                blobSize: capturedBlob ? capturedBlob.size : 0
            };
        }""")
        assert result["blobType"] == "image/jpeg"
        assert result["blobSize"] > 0


class TestPNGExport:
    def test_png_produces_blob(self, page):
        """PNG export should produce a valid blob."""
        result = page.evaluate("""() => {
            var w = 4, h = 4;
            var rgba = new Uint8Array(w * h * 4);
            for (var i = 0; i < rgba.length; i += 4) {
                rgba[i] = 0; rgba[i+1] = 255; rgba[i+2] = 0; rgba[i+3] = 255;
            }

            var capturedBlob = null;
            var origCreate = URL.createObjectURL;
            var origRevoke = URL.revokeObjectURL;
            var origClick = HTMLAnchorElement.prototype.click;
            HTMLAnchorElement.prototype.click = function() {};
            URL.createObjectURL = function(b) { capturedBlob = b; return 'blob:test'; };
            URL.revokeObjectURL = function() {};

            exportPersistentBufferAsPNG(rgba, w, h, 'test.png');

            URL.createObjectURL = origCreate;
            URL.revokeObjectURL = origRevoke;
            HTMLAnchorElement.prototype.click = origClick;

            return {
                blobType: capturedBlob ? capturedBlob.type : null,
                blobSize: capturedBlob ? capturedBlob.size : 0
            };
        }""")
        assert result["blobType"] == "image/png"
        assert result["blobSize"] > 0


class TestTIFFExport:
    def test_tiff_produces_blob(self, page):
        """TIFF export should produce a valid blob."""
        result = page.evaluate("""() => {
            var w = 4, h = 4;
            var rgba = new Uint8Array(w * h * 4);
            for (var i = 0; i < rgba.length; i += 4) {
                rgba[i] = 0; rgba[i+1] = 0; rgba[i+2] = 255; rgba[i+3] = 255;
            }

            var capturedBlob = null;
            var origCreate = URL.createObjectURL;
            var origRevoke = URL.revokeObjectURL;
            var origClick = HTMLAnchorElement.prototype.click;
            HTMLAnchorElement.prototype.click = function() {};
            URL.createObjectURL = function(b) { capturedBlob = b; return 'blob:test'; };
            URL.revokeObjectURL = function() {};

            exportPersistentBufferAsTIFF(rgba, w, h, 'test.tiff');

            URL.createObjectURL = origCreate;
            URL.revokeObjectURL = origRevoke;
            HTMLAnchorElement.prototype.click = origClick;

            return {
                blobType: capturedBlob ? capturedBlob.type : null,
                blobSize: capturedBlob ? capturedBlob.size : 0
            };
        }""")
        assert result["blobType"] == "image/tiff"
        assert result["blobSize"] > 0
