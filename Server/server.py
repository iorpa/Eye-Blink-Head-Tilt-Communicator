from flask import Flask, request, jsonify, send_from_directory
from datetime import datetime, timezone
import os

app = Flask(__name__)

latest_message = None
latest_message_time = None
message_history = []

SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.abspath(os.path.join(SERVER_DIR, ".."))
DASHBOARD_DIR = os.path.join(PROJECT_DIR, "Dashboard")


def now_iso_utc():
    # e.g. 2026-09-15T10:23:45Z
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


@app.route("/")
def home():
    return send_from_directory(DASHBOARD_DIR, "index.html")


@app.route("/style.css")
def style():
    return send_from_directory(DASHBOARD_DIR, "style.css")


@app.route("/script.js")
def script():
    return send_from_directory(DASHBOARD_DIR, "script.js")


@app.route("/message", methods=["POST"])
def receive_message():
    global latest_message, latest_message_time, message_history

    data = request.get_json(silent=True)
    if not data:
        return jsonify({
            "status": "error",
            "message": "No JSON data received."
        }), 400

    message = data.get("message")
    if not message:
        return jsonify({
            "status": "error",
            "message": "No patient message received."
        }), 400

    now = now_iso_utc()
    latest_message = message
    latest_message_time = now

    message_history.insert(0, {"message": message, "time": now})
    message_history = message_history[:5]

    print()
    print("NEW PATIENT MESSAGE")
    print("-------------------")
    print("Message:", message)
    print("Received:", now)
    print()

    return jsonify({
        "status": "success",
        "message": message,
        "time": now
    }), 200


@app.route("/latest", methods=["GET"])
def get_latest_message():
    return jsonify({
        "status": "success",
        "message": latest_message,
        "time": latest_message_time
    }), 200


@app.route("/history", methods=["GET"])
def get_history():
    return jsonify({
        "status": "success",
        "history": message_history
    }), 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)