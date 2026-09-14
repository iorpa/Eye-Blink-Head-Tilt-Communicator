const messageElement = document.getElementById("message");
const messageTime = document.getElementById("messageTime");

const connectionStatus =
    document.getElementById("connectionStatus");

const deviceStatus =
    document.getElementById("deviceStatus");

const statusDot =
    document.querySelector(".status-dot");

const historyElement =
    document.getElementById("history");

const SERVER_URL = window.location.origin;

let lastMessageTime = null;


/* Show current message */

function showMessage(message, time) {

    messageElement.textContent = message;

    if (time) {

        messageTime.textContent =
            "Received: " + formatTime(time);

    } else {

        messageTime.textContent = "--";
    }
}


/* Convert server time to readable time */

function formatTime(timeString) {

    const date = new Date(
        timeString.replace(" ", "T")
    );

    return date.toLocaleTimeString(
        "en-BD",
        {
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit"
        }
    );
}


/* Clear selected message */

function clearCurrentMessage() {

    messageElement.textContent =
        "Waiting for message...";

    messageTime.textContent = "--";
}


/* Check latest message */

async function checkLatestMessage() {

    try {

        const response = await fetch(
            SERVER_URL + "/latest"
        );

        if (!response.ok) {
            throw new Error("Server response error");
        }

        const data = await response.json();

        if (
            data.message &&
            data.time &&
            data.time !== lastMessageTime
        ) {

            lastMessageTime = data.time;

            showMessage(
                data.message,
                data.time
            );

            loadHistory();

            /*
             * Remove the selected message
             * after 30 seconds.
             */

            setTimeout(
                clearCurrentMessage,
                30000
            );
        }

    } catch (error) {

        console.log(
            "Latest message error:",
            error
        );
    }
}


/* Load last 5 messages */

async function loadHistory() {

    try {

        const response = await fetch(
            SERVER_URL + "/history"
        );

        if (!response.ok) {
            throw new Error("History error");
        }

        const data = await response.json();

        historyElement.innerHTML = "";

        if (
            !data.history ||
            data.history.length === 0
        ) {

            historyElement.innerHTML =
                '<div class="history-empty">' +
                'No messages yet.' +
                '</div>';

            return;
        }


        data.history.forEach(
            function(item) {

                const historyItem =
                    document.createElement("div");

                historyItem.className =
                    "history-item";


                const historyMessage =
                    document.createElement("span");

                historyMessage.className =
                    "history-message";

                historyMessage.textContent =
                    item.message;


                const historyTime =
                    document.createElement("span");

                historyTime.className =
                    "history-time";

                historyTime.textContent =
                    formatTime(item.time);


                historyItem.appendChild(
                    historyMessage
                );

                historyItem.appendChild(
                    historyTime
                );

                historyElement.appendChild(
                    historyItem
                );
            }
        );

    } catch (error) {

        console.log(
            "History error:",
            error
        );
    }
}


/* Check whether the device is active */

async function checkDeviceStatus() {

    try {

        const response = await fetch(
            SERVER_URL + "/device-status"
        );

        if (!response.ok) {
            throw new Error("Device status error");
        }

        const data = await response.json();

        if (data.active) {

            connectionStatus.textContent =
                "Device Active";

            deviceStatus.textContent =
                "Device Active";

            statusDot.style.background =
                "#22c55e";

        } else {

            connectionStatus.textContent =
                "Device Inactive";

            deviceStatus.textContent =
                "Device Inactive";

            statusDot.style.background =
                "#ef4444";
        }

    } catch (error) {

        console.log(
            "Device status error:",
            error
        );

        connectionStatus.textContent =
            "Device Inactive";

        deviceStatus.textContent =
            "Device Inactive";

        statusDot.style.background =
            "#ef4444";
    }
}


/* Initial page */

clearCurrentMessage();

loadHistory();

checkLatestMessage();

checkDeviceStatus();


/* Keep checking */

setInterval(
    checkLatestMessage,
    3000
);

setInterval(
    loadHistory,
    3000
);

setInterval(
    checkDeviceStatus,
    5000
);