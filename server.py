from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
import numpy as np
import json

app = FastAPI()

app.mount("/static", StaticFiles(directory="static"), name="static")


@app.get("/")
async def root():
    return FileResponse("static/index.html")


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            raw = await websocket.receive_text()
            data = json.loads(raw)
            coeffs = [complex(c[0], c[1]) for c in data["coefficients"]]

            # Strip leading near-zero coefficients
            while len(coeffs) > 1 and abs(coeffs[0]) < 1e-15:
                coeffs = coeffs[1:]

            try:
                roots = np.roots(coeffs)
                valid = [r for r in roots if np.isfinite(r)]
                result = {
                    "roots": [[float(r.real), float(r.imag)] for r in valid],
                    "error": None,
                }
            except Exception as e:
                result = {"roots": [], "error": str(e)}

            await websocket.send_text(json.dumps(result))
    except WebSocketDisconnect:
        pass
