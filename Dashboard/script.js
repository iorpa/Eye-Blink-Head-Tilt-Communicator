const messageElement = document.getElementById("message");
const connectionStatus = document.getElementById("connectionStatus");
const deviceStatus = document.getElementById("deviceStatus");
const statusDot = document.querySelector(".status-dot");

const SERVER_URL = window.location.origin;

let lastMessage = "";

function showMessage(message) {
    messageElement.textContent = message;
}

function setConnected() {
    connectionStatus.textContent = "Connected";
    deviceStatus.textContent = "Connected";
    statusDot.style.background = "#22c55e";
}

function setDisconnected() {
    connectionStatus.textContent = "Disconnected";
    deviceStatus.textContent = "Disconnected";
    statusDot.style.background = "#ef4444";
}

async function checkServer() {

    try {

        const response = await fetch(
            SERVER_URL + "/latest"
        );

        if (!response.ok) {
            throw new Error("Server response error");
        }

        const data = await response.json();

        setConnected();

        if (
            data.message &&
            data.message !== lastMessage
        ) {

            lastMessage = data.message;

            console.log(
                "Message received:",
                data.message
            );

            showMessage(data.message);

            deviceStatus.textContent =
                "Message Received";
        }

    } catch (error) {

        console.log(
            "Server connection error:",
            error
        );

        setDisconnected();

        deviceStatus.textContent =
            "Server disconnected";
    }
}

showMessage("Waiting for message...");
setDisconnected();

checkServer();

setInterval(
    checkServer,
    3000
);