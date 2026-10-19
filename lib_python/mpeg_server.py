from http.server import BaseHTTPRequestHandler, HTTPServer
import cv2

PORT = 8080

def web_server(queue):
    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path != "/":
                return

            self.send_response(200)
            self.send_header(
                "Content-Type",
                "multipart/x-mixed-replace; boundary=frame"
            )
            self.end_headers()

            while True:
                frame = queue.get()
                _, jpg = cv2.imencode(".jpg", frame)

                self.wfile.write(
                    b"--frame\r\n"
                    b"Content-Type: image/jpeg\r\n\r\n" +
                    jpg.tobytes() +
                    b"\r\n"
                )

    HTTPServer(("0.0.0.0", PORT), Handler).serve_forever()

