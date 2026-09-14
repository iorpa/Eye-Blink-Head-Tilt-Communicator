const messageElement =
    document.getElementById("message");

const connectionStatus =
    document.getElementById("connectionStatus");

const deviceStatus =
    document.getElementById("deviceStatus");

const statusDot =
    document.querySelector(".status-dot");

const testStatus =
    document.getElementById("testStatus");


const SERVER_URL =
    window.location.origin;


let lastMessage = "";


function showMessage(message) {

    messageElement.textContent = message;

}


function setConnected() {

    connectionStatus.textContent =
        "Connected";

    deviceStatus.textContent =
        "Connected";

    statusDot.style.background =
        "#22c55e";

}


function setDisconnected() {

    connectionStatus.textContent =
        "Disconnected";

    deviceStatus.textContent =
        "Disconnected";

    statusDot.style.background =
        "#ef4444";

}


async function checkServer() {

    try {

        const response =
            await fetch(
                SERVER_URL + "/latest"
            );


        if (!response.ok) {

            throw new Error(
                "Server response error"
            );

        }


        const data =
            await response.json();


        setConnected();


        if (
            data.message &&
            data.message !== lastMessage
        ) {

            lastMessage =
                data.message;


            console.log(
                "Message received:",
                data.message
            );


            showMessage(
                data.message
            );


            deviceStatus.textContent =
                "Message Received";

        }

    }
    catch (error) {

        console.log(
            "Server connection error:",
            error
        );


        setDisconnected();


        deviceStatus.textContent =
            "Server disconnected";

    }

}


async function sendTestMessage(message) {

    try {

        testStatus.textContent =
            "Sending " + message + "...";


        const response =
            await fetch(
                SERVER_URL + "/message",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({
                        message: message
                    })
                }
            );


        if (!response.ok) {

            throw new Error(
                "Server error"
            );

        }


        const data =
            await response.json();


        console.log(
            "Test message sent:",
            data
        );


        testStatus.textContent =
            "Sent: " + message;


        showMessage(
            message
        );


        deviceStatus.textContent =
            "Message Received";


        lastMessage =
            message;

    }
    catch (error) {

        console.error(
            "Test message error:",
            error
        );


        testStatus.textContent =
            "Failed to send message.";

    }

}


showMessage(
    "Waiting for message..."
);


setDisconnected();


checkServer();


setInterval(
    checkServer,
    3000
);

