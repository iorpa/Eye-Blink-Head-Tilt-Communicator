from flask import Flask, request, jsonify

app = Flask(__name__)

latest_message = "Waiting for message..."


@app.route("/")
def home():
    return "Patient Communication Server is running."


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