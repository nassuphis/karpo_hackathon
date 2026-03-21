**Preview Compatibility**

This note records the compatibility findings for the new bilevel TIFF output.

Tested file:

- [compute_mmzeim5h_10000px_bilevel.tif](/Users/nicknassuphis/karpo_hackathon/polypaint/images/compute_mmzeim5h_10000px_bilevel.tif)

**Findings**

The produced TIFF is valid.

Local tools that successfully read it:

- `file`
- `vipsheader`
- `tiffinfo`
- `vips copy`
- `tiffcp`

macOS native image stack tools that fail to read it:

- `Preview`
- `sips`

So the problem is not corruption. It is compatibility.

**What Combination Fails**

The stitched bilevel file is:

- TIFF
- CCITT Group 4 compressed
- 1-bit
- tiled

That combination is what Preview rejects.

The key comparison results were:

- tiled CCITT G4 TIFF:
  - readable by libvips / libtiff tools
  - not readable by `sips`
- strip-based CCITT G4 TIFF:
  - readable by `sips`
- this remained true whether the file was:
  - classic TIFF
  - or BigTIFF

So the main incompatibility is:

- tiled CCITT Group 4 TIFF

not:

- CCITT Group 4 by itself
- and not BigTIFF by itself

**Conclusion**

The new stitcher is producing valid TIFFs, but not TIFFs that macOS Preview reliably opens.

If Preview compatibility is required, the final downloadable TIFF should be:

- strip-based CCITT Group 4

not:

- tiled CCITT Group 4

**Can A Compatibility Conversion Be Added At The End**

Yes.

A compatibility step can be added after the large-image stitch completes:

1. write the exact-size tiled TIFF
2. rewrite it as strip-based CCITT G4
3. expose the strip-based file as the user-facing download

This is possible.

The local proof is:

```bash
tiffcp -s -c g4 input.tif output.tif
```

That produces a Preview-readable file from the tiled source.

**Recommended Policy**

Do **not** add another render mode for this.

The correct design is:

- render bilevel normally
- produce the scalable tiled TIFF as the primary artifact
- then offer Preview compatibility as a separate post-render operation

This should be the only compatibility approach.

Do **not**:

- add `BiLevel (Preview)` as another render button
- make compatibility conversion part of the normal bilevel render path
- force every bilevel render to pay for an extra full-file rewrite

**UI Placement**

This belongs in the Render tab, but **after** the bilevel render has already completed.

The flow should be:

1. user presses `BiLevel`
2. normal bilevel render runs
3. UI shows:
   - preview PNG
   - normal tiled TIFF download button
4. next to those render results, show a second action:
   - `Make Preview-Compatible TIFF`
   - or:
   - `Convert+Download TIFF`

This button should only appear:

- after a successful bilevel render
- for that specific completed render result

This is important because the feature is not another rendering mode. It is a post-render conversion step applied to an already finished bilevel TIFF.

So in UI terms, it belongs:

- in the Render tab
- in the result/download area
- next to the bilevel output controls
- not in the row of main render-mode buttons

**Backend Shape**

This should have its own dedicated Lambda.

Reason:

- separate timeout
- separate memory budget
- separate `/tmp` needs
- separate task status
- separate failure mode

The Lambda contract should be simple:

- input:
  - source tiled TIFF key
  - destination compatibility TIFF key
- output:
  - compatibility TIFF key

The job of this Lambda is:

- read the finished tiled TIFF
- rewrite it as strip-based CCITT Group 4 TIFF
- upload the compatibility file

That keeps the conversion entirely outside the core render path.

**Why This Must Be Post-Render**

The core bilevel render path is the scalable path.

Its job is:

- produce the exact-size tiled TIFF
- produce the PNG preview

The compatibility step is different:

- it is a convenience export
- it exists for desktop viewer compatibility
- it is an extra full-file pass

So it must remain:

- post-render
- opt-in
- separate from the main render workflow

not:

- another default render path
- and not another render-mode button

**Will Large Files Still Be Possible**

Yes.

That is exactly why this needs to be a post-render feature and not part of the main render button.

The scalable core remains:

- tiled exact-size TIFF
- PNG preview

The compatibility conversion is only invoked when the user explicitly asks for it.

That preserves the large-image render path.

For very large outputs, the realistic workflow is:

- render normally
- inspect via PNG preview
- download the tiled TIFF if needed
- only request compatibility conversion when it is worth the extra time

**Recommendation**

The recommended design is:

- keep the current tiled TIFF as `image_bilevel.tif`
- keep the PNG preview as the normal in-app preview
- add a post-render button in the Render results area:
  - `Make Preview-Compatible TIFF`
- have that button dispatch a separate conversion Lambda
- write a second file such as:
  - `image_bilevel_compat.tif`
- offer that file as a separate download once ready

This should be the only compatibility option.

Do not add:

- a `BiLevel (Preview)` render button
- a second render mode
- automatic compatibility rewriting during normal bilevel render

The goal is explicitly:

- a post-render feature
- attached to a completed bilevel result
- located in the Render tab’s result/download area
- implemented by a separate task-specific Lambda
