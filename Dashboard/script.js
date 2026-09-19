const messageElement =
    document.getElementById("message");


const messageTimeElement =
    document.getElementById("messageTime");


const SERVER_URL =
    window.location.origin;


/* FORMAT TIME */

function formatTime(timeString) {

    if (!timeString) {

        return "--";
    }


    const date =
        new Date(timeString);


    if (isNaN(date.getTime())) {

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


/* CLEAR CURRENT MESSAGE */

function clearMessage() {

    messageElement.textContent =
        "Waiting for message...";


    messageTimeElement.textContent =
        "--";
}


/* SHOW CURRENT MESSAGE */

function showMessage(
    message,
    time
) {

    messageElement.textContent =
        message;


    messageTimeElement.textContent =
        time
            ? "Received: " +
              formatTime(time)
            : "--";
}


/* CHECK SERVER */

async function checkLatestMessage() {

    const latestURL =
        SERVER_URL + "/latest";


    console.log(
        "Checking:",
        latestURL
    );


    try {

        const response =
            await fetch(
                latestURL,
                {
                    cache: "no-store"
                }
            );


        console.log(
            "HTTP STATUS:",
            response.status
        );


        if (!response.ok) {

            throw new Error(
                "Server response error: " +
                response.status
            );
        }


        const data =
            await response.json();


        console.log(
            "SERVER DATA:",
            data
        );


        if (
            data.message &&
            data.time
        ) {

            showMessage(
                data.message,
                data.time
            );

        } else {

            clearMessage();
        }


    } catch (error) {

        console.error(
            "LATEST MESSAGE ERROR:",
            error
        );
    }
}


/* START DASHBOARD */

clearMessage();


console.log(
    "Dashboard started."
);


console.log(
    "Server URL:",
    SERVER_URL
);


checkLatestMessage();


/*
  Check every 1 second.

  Only the latest message is displayed.
*/

setInterval(
    checkLatestMessage,
    1000
);

