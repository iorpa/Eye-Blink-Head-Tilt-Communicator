from flask import Flask, request, jsonify, send_from_directory
import os

app = Flask(__name__)

latest_message = "Waiting for message..."

# Get the folder where server.py is located
SERVER_DIR = os.path.dirname(os.path.abspath(__file__))

# Go one level up to the project folder
PROJECT_DIR = os.path.abspath(os.path.join(SERVER_DIR, ".."))

# Dashboard folder
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

    latest_message = message

    print()
    print("NEW PATIENT MESSAGE")
    print("-------------------")
    print(message)
    print()

    return jsonify({
        "status": "success",
        "message": message
    }), 200


@app.route("/latest", methods=["GET"])
def get_latest_message():

    return jsonify({
        "status": "success",
        "message": latest_message
    }), 200


if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5000
    )