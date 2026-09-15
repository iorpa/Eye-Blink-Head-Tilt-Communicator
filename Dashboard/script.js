const messageElement =
    document.getElementById("message");

const messageTimeElement =
    document.getElementById("messageTime");


const lastMessageElement =
    document.getElementById("lastMessage");

const lastMessageTimeElement =
    document.getElementById("lastMessageTime");


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


/* SHOW LAST MESSAGE */

function showLastMessage(
    message,
    time
) {

    if (message) {

        lastMessageElement.textContent =
            message;

        if (time) {

            lastMessageTimeElement.textContent =
                "Received: " +
                formatTime(time);

        } else {

            lastMessageTimeElement.textContent =
                "--";
        }

    } else {

        lastMessageElement.textContent =
            "No previous message";

        lastMessageTimeElement.textContent =
            "--";
    }
}


/* CHECK LATEST MESSAGE */

async function checkLatestMessage() {

    try {

        const response =
            await fetch(
                SERVER_URL +
                "/latest",
                {
                    cache: "no-store"
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
            data.time
        ) {

            if (
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

        } else {

            lastMessageTime = null;

            clearCurrentMessage();
        }


        showLastMessage(
            data.last_message,
            data.last_time
        );


    } catch (error) {

        console.log(
            "Latest message error:",
            error
        );
    }
}


/* START DASHBOARD */

clearCurrentMessage();

showLastMessage(
    null,
    null
);

checkLatestMessage();


/* AUTO UPDATE */

setInterval(
    checkLatestMessage,
    3000
);

