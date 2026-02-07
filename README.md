# PolyPaint

Interactive polynomial root visualizer. Drag coefficients in the complex plane and watch the roots move in real time.

## Run

```bash
uv run uvicorn server:app --reload --port 8000
```

Open [http://localhost:8000](http://localhost:8000).

## How it works

- **Left panel**: polynomial coefficients as draggable dots on the complex plane
- **Right panel**: roots computed via `numpy.roots`, updated live over WebSocket
- **Degree slider**: controls polynomial degree (3–30)
