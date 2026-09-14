const messageElement =
    document.getElementById("message");

const messageTimeElement =
    document.getElementById("messageTime");


const SERVER_URL =
    window.location.origin;


let lastMessageTime = null;

let clearMessageTimer = null;


/* FORMAT TIME */

function formatTime(timeString) {

    if (!timeString) {

        return "--";
    }


    const date =
        new Date(timeString);


    if (
        isNaN(
            date.getTime()
        )
    ) {

        return timeString;
    }


    return date.toLocaleTimeString(
        "en-BD",
        {
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit"
        }
    );
}


/* SHOW CURRENT MESSAGE */

function showMessage(
    message,
    time
) {

    messageElement.textContent =
        message;


    if (time) {

        messageTimeElement.textContent =
            "Received: " +
            formatTime(time);

    } else {

        messageTimeElement.textContent =
            "--";
    }


    if (clearMessageTimer) {

        clearTimeout(
            clearMessageTimer
        );
    }


    clearMessageTimer =
        setTimeout(
            clearCurrentMessage,
            30000
        );
}


/* CLEAR CURRENT MESSAGE */

function clearCurrentMessage() {

    messageElement.textContent =
        "Waiting for message...";


    messageTimeElement.textContent =
        "--";
}


/* CHECK LATEST MESSAGE */

async function checkLatestMessage() {

    try {

        const response =
            await fetch(
                SERVER_URL +
                "/latest",
                {
                    cache:
                        "no-store"
                }
            );


        if (!response.ok) {

            throw new Error(
                "Server response error"
            );
        }


        const data =
            await response.json();


        if (
            data.message &&
            data.time &&
            data.time !==
                lastMessageTime
        ) {

            lastMessageTime =
                data.time;


            showMessage(
                data.message,
                data.time
            );
        }

    }

    catch (error) {

        console.log(
            "Latest message error:",
            error
        );
    }
}


/* START DASHBOARD */

clearCurrentMessage();

checkLatestMessage();


/* AUTO UPDATE */

setInterval(
    checkLatestMessage,
    3000
);