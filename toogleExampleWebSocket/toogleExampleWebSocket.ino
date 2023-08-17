// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

// Replace with your network credentials
const char* ssid = "Orange_Swiatlowod_BBE0";
const char* password = "pC7WNEk6ib6cX3NfuQ";

const char* http_username = "sw3dADMIN";
const char* http_password = "1969#A";

const char* PARAM_INPUT_1 = "state";

bool ledState = 0;

const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    .communication {
      padding: 10px;
    }
    
    .start {
      text-align:center;
      border:solid;
      width: 200px;
      height: 50px;
    }
    
    .info_1 {
      border:solid;
      width: 200px;
    }
    
    .info_2 {
      border:solid;
      width: 200px;
    }
    
    .midContainer {
      display: flex;
      flex-direction: row;
    }
    
    .gameField {
      padding: 15px;
      border:solid;
    }
    
    .chat {
      width: 200px;
      height: 200px;
      border:solid;
      position:relative;
    }
    .send{
      width:40px;
      height:25px;
      position: absolute; 
      bottom: 0;
      right: 0px;
    }
    .lineOfControls {
      display: flex;
      flex-direction: row;
    }
    
    .button {
      width: 60px;
      height: 60px;
      font-size: 30px;
      text-align: center;
      color: #fff;
      background-color: #0f8b8d;
      border-radius: 5px;
      border: solid;
    }
    .type-text{
      position:absolute; bottom:0;
      width: 200px;
      height:25px;
    }
    span{
    opacity: 0.3;  
    }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>

<div class="communication">
      <button id="buttonStart" class="buttonStart">buttonStart</button>

      <div class="info_1" id="info_1">Info</div>
      <!-- <div class="info_2" id="info_2">Info</div> -->
    </div>
    <div class="midContainer">

      <div class='gameField'>
        <table>
          <div class="lineOfControls">
            <div id="button-00" class="button"></div>
            <div id="button-01" class="button"></div>
            <div id="button-02" class="button"></div>
          </div>
          <div class="lineOfControls">
            <div id="button-10" class="button"></div>
            <div id="button-11" class="button"></div>
            <div id="button-12" class="button"></div>
          </div>
          <div class="lineOfControls">
            <div id="button-20" class="button"></div>
            <div id="button-21" class="button"></div>
            <div id="button-22" class="button"></div>
          </div>
        </table>
        <div class="chat">
          <div class="type-text"><input type="text"></div>
          <div ><button class ="send">send</button></div>
        </div>

      </div>

    </div>

<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
    console.log(event);
  }
  function onClose(event) {
    console.log('Connection closed');
    console.log(event);
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    console.log('onMessage() !!!');
    console.log(event);
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
    initStart()
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }

function initStart() {
  console.log("INIT START");
    document.getElementById('buttonStart').addEventListener('click', testStart);
  }
  function testStart(){
    console.log("startGame()");
    startGame()
    websocket.send('testStart');
  }

var player = 1; // turn of player 1-X or 2-O 
var winCombinations = [
  "00,01,02",
  "10,11,12",
  "20,21,22",
  "00,10,20",
  "01,11,21",
  "02,12,22",
  "00,11,22",
  "02,11,20"
];

function startGame() {
  initButtons();
  drawGameBoard();
  checkWinConditions();
}
function initButtons() {
  for (let i = 0; i < 3; i += 1) {
    for (let j = 0; j < 3; j += 1) {
      let buttonIndex = 'button-'.concat(i.toString() + j.toString());
      document.getElementById(buttonIndex).addEventListener('click', gameClick);
      document.getElementById(buttonIndex).style.color = "#0f8b8d";
    }
  }
}

function drawGameBoard() {
  for (let i = 0; i < 3; i += 1) {
    for (let j = 0; j < 3; j += 1) {
      let buttonIndex = "button-".concat(i.toString() + j.toString());
      let button = document.getElementById(buttonIndex);
      button.innerHTML = i.toString() + j.toString();
      button.style.color = "#fff";
      button.style.opacity = "1";
    }
  }
}

function gameClick() {
console.log("GAMECLICK");
  let field = document.getElementById(this.id);
  if (player == 1) {
    field.innerHTML = 'X';
    field.style.color = "red";
    player = 2;
  } else {
    field.innerHTML = 'O';
    field.style.color = "blue";
    player = 1;
  }
  checkWinConditions();
  if (checkifFull()) {
  document.getElementById("info_1").innerHTML = "TIE"
  return "TIE"};

  //MAYBE HERE ADD NOTIFICATIONS
  //websocket.send('testStart');
}

function checkWinConditions() {
console.log("CHECK WIN CONDITIONS");
  let countOfElementsInLine = 0;
  let playerSymbol;
  player == 1 ? playerSymbol = "O" : playerSymbol = "X";
  winCombinations.forEach((lineOfConditions) => {
    lineOfConditions.split(",").forEach((buttonIndex) => {
      let button = document.getElementById("button-" + buttonIndex);
      if (button.textContent == playerSymbol) {
        countOfElementsInLine += 1;
      }
      if (countOfElementsInLine == 3) {
      document.getElementById("info_1").innerHTML = "WINNING player " + playerSymbol;
        paintWinnerPositions(lineOfConditions)
        startGame();
      }
    });
    countOfElementsInLine = 0;
  });
}


function checkifFull() {
  let counterXO = 0;
  for (let i = 0; i < 3; i += 1) {
    for (let j = 0; j < 3; j += 1) {
      let buttonIndex = "button-".concat(i.toString() + j.toString());
      let button = document.getElementById(buttonIndex);
      if (button.innerHTML == "X" || button.innerHTML == "O") {
        counterXO += 1;
      }
    }
  }
  if (counterXO == 9) {
   document.getElementById("info_1").innerHTML = "TIE";
    startGame();
  } else {
    counterXO = 0;
  }
}

function paintWinnerPositions(lineOfConditions){
	 lineOfConditions.split(",").forEach((buttonIndex) => {
   	let button = document.getElementById("button-" + buttonIndex);
    button.style.color = "black";
    button.style.opacity = "0.5";
   });
}

</script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

const char game_chat_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  
</body>
</html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(ledState));
}
void notifyClientsTestStart() {
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) { 
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.printf("DEBUG : (char*)data");
    Serial.printf((char*)data);
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
    if (strcmp((char*)data, "testStart") == 0) {
      notifyClientsTestStart();
    }
  }
}
// The type argument represents the event that occurs
    //WS_EVT_CONNECT when a client has logged in;
    //WS_EVT_DISCONNECT when a client has logged out;
    //WS_EVT_DATA when a data packet is received from the client;
    //WS_EVT_PONG in response to a ping request;
    //WS_EVT_ERROR when an error is received from the client.
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
  });

  server.on("/gamechat", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", game_chat_html, processor);
  });

  //game_chat_html
  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      digitalWrite(ledPin, inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
}