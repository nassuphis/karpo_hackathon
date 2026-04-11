# TODO

## Color RePalette

- Replace the current blocking `color_repalette -> synchronous encode -> synchronous preview` path with a real async tracked workflow.
- Reason: large Color RePalette runs can finish tile generation and then fail in the caller while waiting on `polypaint-encode`, even when encode itself is just slow rather than broken.
- Target shape:
  - phase 1: render recolored tile raws
  - phase 2: encode as its own tracked async step
  - phase 3: preview / cleanup
  - browser keeps polling normal task status instead of waiting on one blocking Lambda
- Keep the immediate mitigation in place for now:
  - `polypaint-encode` memory raised to `10240 MB`
  - longer Lambda invoke read timeout in [lambda/handler_color_repalette.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_color_repalette.py)
