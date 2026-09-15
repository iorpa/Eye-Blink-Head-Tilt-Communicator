
from flask import Flask, request, jsonify, send_from_directory
from datetime import datetime, timezone
import os

app = Flask(__name__)

latest_message = None
latest_message_time = None

last_message = None
last_message_time = None

MESSAGE_DISPLAY_TIME = 30

SERVER_DIR = os.path.dirname(
    os.path.abspath(__file__)
)

PROJECT_DIR = os.path.abspath(
    os.path.join(
        SERVER_DIR,
        ".."
    )
)

DASHBOARD_DIR = os.path.join(
    PROJECT_DIR,
    "Dashboard"
)


def now_utc():
    return datetime.now(timezone.utc)


def now_iso_utc():
    return now_utc().strftime(
        "%Y-%m-%dT%H:%M:%SZ"
    )


def message_is_active():

    global latest_message
    global latest_message_time
    global last_message
    global last_message_time
<<<<<<< HEAD

=======
>>>>>>> d55e462 (code updated)

    if (
        latest_message is None
        or latest_message_time is None
    ):
        return False

    try:

        message_time = datetime.strptime(
            latest_message_time,
            "%Y-%m-%dT%H:%M:%SZ"
        ).replace(
            tzinfo=timezone.utc
        )

        elapsed_time = (
            now_utc() - message_time
        ).total_seconds()

        if elapsed_time >= MESSAGE_DISPLAY_TIME:

<<<<<<< HEAD
            # Move the expired current message
            # into the last-message slot
=======
            print(
                "Current message expired:",
                latest_message
            )
>>>>>>> d55e462 (code updated)

            last_message = latest_message
            last_message_time = latest_message_time

            latest_message = None
            latest_message_time = None

            return False

        return True

    except Exception as error:

        print(
            "Message time error:",
            error
        )

        latest_message = None
        latest_message_time = None

        return False


@app.route("/")
def home():

    return send_from_directory(
        DASHBOARD_DIR,
        "index.html"
    )


@app.route("/style.css")
def style():

    return send_from_directory(
        DASHBOARD_DIR,
        "style.css"
    )


@app.route("/script.js")
def script():

    return send_from_directory(
        DASHBOARD_DIR,
        "script.js"
    )


@app.route(
    "/message",
    methods=["POST"]
)
def receive_message():

    global latest_message
    global latest_message_time
    global last_message
    global last_message_time

    # Check whether the previous current
    # message has already expired.
    message_is_active()

    data = request.get_json(
        silent=True
    )

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

<<<<<<< HEAD

    # If there is an active current message,
    # move it to the last-message slot

=======
    # If there is still an active current
    # message, move it to LAST MESSAGE.
>>>>>>> d55e462 (code updated)
    if latest_message is not None:

        last_message = latest_message
        last_message_time = latest_message_time

<<<<<<< HEAD

    # Store the new message as current

=======
    # New message becomes CURRENT MESSAGE.
>>>>>>> d55e462 (code updated)
    latest_message = message
    latest_message_time = now

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


@app.route(
    "/latest",
    methods=["GET"]
)
def get_latest_message():

    message_is_active()
<<<<<<< HEAD

=======
>>>>>>> d55e462 (code updated)

    return jsonify({
        "status": "success",
<<<<<<< HEAD

        "message": latest_message,

        "time": latest_message_time,

=======
        "message": latest_message,
        "time": latest_message_time,
>>>>>>> d55e462 (code updated)
        "last_message": last_message,
        "last_time": last_message_time
    }), 200


if __name__ == "__main__":

    app.run(
        host="0.0.0.0",
        port=5000
<<<<<<< HEAD
    )
=======
    )
>>>>>>> d55e462 (code updated)
