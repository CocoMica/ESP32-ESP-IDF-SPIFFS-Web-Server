var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
let count = 0;
// Get references to HTML elements
const headerElement = document.getElementById("header");
const paragraphElement = document.getElementById("paragraph");
const updateButton = document.getElementById("updateButton");

// Add an event listener to the button
updateButton.addEventListener("click", () => {
    // Update the content of the header and paragraph
    console.log("button clicked!");
    headerElement.textContent = "New Header";
    paragraphElement.textContent = "New Paragraph Content";
});

window.addEventListener('load', onLoad);
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
    console.log('Connection opened');
    setInterval(toggle, 4000);
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log('Receiving message');
    var state;
    console.log(event.data);
    if (event.data == "1") {
        state = "ON";
    }
    else {
        state = "OFF";
    }
   // document.getElementById('state').innerHTML = state;
}
function onLoad(event) {
    initWebSocket();
    initButton();
}
function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
}

function toggle() {
    count++;
    let msg = 'info from browser: ' + count;
    console.log('sending toggle message');
    websocket.send(msg);
}
