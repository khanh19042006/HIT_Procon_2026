import json
import subprocess
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SIMULATOR = Path(__file__).resolve().parent
SOLVER_API = ROOT / "solver_api.exe"


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(SIMULATOR), **kwargs)

    def do_POST(self):
        if self.path != "/api/solve":
            self.send_error(404)
            return
        length = int(self.headers.get("Content-Length", "0"))
        try:
            request = json.loads(self.rfile.read(length))
            result = subprocess.run([str(SOLVER_API)], input=json.dumps(request), text=True, capture_output=True, check=True)
            payload = json.loads(result.stdout)
            self.send_response(200)
        except (OSError, subprocess.CalledProcessError, json.JSONDecodeError, ValueError) as error:
            payload = {"error": str(error)}
            self.send_response(500)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps(payload).encode("utf-8"))


if __name__ == "__main__":
    print("HexaUdon simulator: http://localhost:4173")
    ThreadingHTTPServer(("localhost", 4173), Handler).serve_forever()