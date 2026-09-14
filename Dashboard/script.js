const messageElement =
    document.getElementById("message");

const messageTimeElement =
    document.getElementById("messageTime");

const historyElement =
    document.getElementById("history");


const SERVER_URL =
    window.location.origin;


let lastMessageTime = null;

let clearMessageTimer = null;


// ==================================================
// FORMAT TIME
// ==================================================

function formatTime(timeString) {

    if (!timeString) {

        return "--";
    }


    const date =
        new Date(
            timeString.replace(
                " ",
                "T"
            )
        );


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


// ==================================================
// SHOW CURRENT MESSAGE
// ==================================================

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

    }

    else {

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


// ==================================================
// CLEAR CURRENT MESSAGE
// ==================================================

function clearCurrentMessage() {

    messageElement.textContent =
        "Waiting for message...";


    messageTimeElement.textContent =
        "--";
}


// ==================================================
// CHECK LATEST MESSAGE
// ==================================================

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


            loadHistory();
        }

    }

    catch (error) {

        console.log(
            "Latest message error:",
            error
        );
    }
}


// ==================================================
// LOAD HISTORY
// ==================================================

async function loadHistory() {

    try {

        const response =
            await fetch(
                SERVER_URL +
                "/history",
                {
                    cache:
                        "no-store"
                }
            );


        if (!response.ok) {

            throw new Error(
                "History response error"
            );
        }


        const data =
            await response.json();


        historyElement.innerHTML =
            "";


        if (
            !data.history ||
            data.history.length === 0
        ) {

            historyElement.innerHTML =
                `
                <div class="history-empty">
                    No messages yet.
                </div>
                `;


            return;
        }


        data.history.forEach(
            function(item) {

                const historyItem =
                    document.createElement(
                        "div"
                    );


                historyItem.className =
                    "history-item";


                const historyMessage =
                    document.createElement(
                        "span"
                    );


                historyMessage.className =
                    "history-message";


                historyMessage.textContent =
                    item.message;


                const historyTime =
                    document.createElement(
                        "span"
                    );


                historyTime.className =
                    "history-time";


                historyTime.textContent =
                    formatTime(
                        item.time
                    );


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

    }

    catch (error) {

        console.log(
            "History error:",
            error
        );
    }
}


// ==================================================
// START DASHBOARD
// ==================================================

clearCurrentMessage();

loadHistory();

checkLatestMessage();


// ==================================================
// AUTO UPDATE
// ==================================================

setInterval(
    checkLatestMessage,
    3000
);


setInterval(
    loadHistory,
    5000
);