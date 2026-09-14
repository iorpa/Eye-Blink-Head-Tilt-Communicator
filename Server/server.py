from flask import Flask, request, jsonify, send_from_directory
from datetime import datetime
import os

app = Flask(__name__)

latest_message = None
latest_message_time = None

message_history = []

last_device_heartbeat = None

SERVER_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.abspath(os.path.join(SERVER_DIR, ".."))
DASHBOARD_DIR = os.path.join(PROJECT_DIR, "Dashboard")


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

    global latest_message
    global latest_message_time
    global message_history

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

    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    latest_message = message
    latest_message_time = now

    message_history.insert(0, {
        "message": message,
        "time": now
    })

    message_history = message_history[:5]

    print()
    print("NEW PATIENT MESSAGE")
    print("-------------------")
    print(message)
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


@app.route("/heartbeat", methods=["POST"])
def device_heartbeat():

    global last_device_heartbeat

    last_device_heartbeat = datetime.now().timestamp()

    return jsonify({
        "status": "success",
        "message": "Device heartbeat received."
    }), 200


@app.route("/device-status", methods=["GET"])
def device_status():

    if last_device_heartbeat is None:

        return jsonify({
            "status": "success",
            "active": False
        }), 200

    current_time = datetime.now().timestamp()

    time_since_heartbeat = (
        current_time - last_device_heartbeat
    )

    # Device is considered active if
    # heartbeat was received within 20 seconds.

    active = time_since_heartbeat <= 20

    return jsonify({
        "status": "success",
        "active": active
    }), 200


if __name__ == "__main__":

    app.run(
        host="0.0.0.0",
        port=5000
    )