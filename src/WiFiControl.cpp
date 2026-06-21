#include <WiFi.h>
#include <WebServer.h>
#include "WiFiControl.h"

static const char* WIFI_SSID = "ESP32_CAR";
static const char* WIFI_PASS = "12345678";
static const uint16_t WEB_PORT = 80;

static WebServer server(WEB_PORT);
static WiFiControl* instancePtr = nullptr;

static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>ESP32 Car</title>
<style>
* { box-sizing: border-box; }
body { font-family: Arial, sans-serif; text-align: center; background: #1a1a2e; color: white; margin: 0; padding: 15px; min-height: 100vh; display: flex; flex-direction: column; align-items: center; }
h1 { color: #00d4ff; margin: 10px 0; font-size: 24px; }
.mode { font-size: 14px; color: #aaa; margin-bottom: 15px; }
.control-box { display: flex; flex-direction: column; align-items: center; gap: 8px; width: 100%; max-width: 300px; }
.row { display: flex; justify-content: center; gap: 8px; width: 100%; }
.btn {
    border: none; border-radius: 12px; cursor: pointer;
    transition: transform 0.1s, background 0.2s; min-width: 70px; height: 70px;
    display: flex; align-items: center; justify-content: center; font-weight: bold;
}
.btn:active { transform: scale(0.92); opacity: 0.8; }
.forward { background: linear-gradient(135deg, #00d4ff, #0099cc); color: #1a1a2e; width: 150px; height: 60px; font-size: 24px; }
.backward { background: linear-gradient(135deg, #e94560, #c73650); color: white; width: 150px; height: 60px; font-size: 24px; }
.left, .right, .stop-btn { background: linear-gradient(135deg, #533483, #3d2663); color: white; font-size: 20px; }
.auto-btn { background: linear-gradient(135deg, #00cc88, #00a86b); color: white; width: 228px; height: 55px; font-size: 18px; margin-top: 5px; }
.status { margin-top: 15px; padding: 12px; background: #16213e; border-radius: 8px; font-size: 14px; width: 100%; max-width: 300px; }
</style>
</head>
<body>
<h1>ESP32 Car</h1>
<div class="mode" id="mode">Mode: Auto</div>
<div class="control-box">
<div class="row"><button class="btn forward" onclick="sendCmd('FORWARD')">FWD</button></div>
<div class="row">
<button class="btn left" onclick="sendCmd('LEFT')">L</button>
<button class="btn stop-btn" onclick="sendCmd('STOP')">STOP</button>
<button class="btn right" onclick="sendCmd('RIGHT')">R</button>
</div>
<div class="row"><button class="btn backward" onclick="sendCmd('BACKWARD')">BACK</button></div>
<div class="row"><button class="btn auto-btn" onclick="sendCmd('AUTO')">AUTO</button></div>
</div>
<div class="status" id="status">Ready</div>
<script>
function sendCmd(cmd) {
    fetch('/control?cmd=' + cmd).then(r => r.text()).then(t => {
        document.getElementById('status').textContent = 'Sent: ' + cmd + ' -> ' + t;
        if (cmd == 'AUTO') document.getElementById('mode').textContent = 'Mode: Auto';
        else if (cmd == 'STOP') document.getElementById('mode').textContent = 'Mode: Stopped';
        else document.getElementById('mode').textContent = 'Mode: Manual';
    });
}
</script>
</body>
</html>
)rawliteral";

static void handleRoot() {
    server.send_P(200, "text/html", HTML_PAGE);
}

static void handleControl() {
    String cmd = server.arg("cmd");
    String response = "OK";

    if (instancePtr) {
        if (cmd == "FORWARD") instancePtr->setCommand(WiFiControl::Command::FORWARD);
        else if (cmd == "BACKWARD") instancePtr->setCommand(WiFiControl::Command::BACKWARD);
        else if (cmd == "LEFT") instancePtr->setCommand(WiFiControl::Command::LEFT);
        else if (cmd == "RIGHT") instancePtr->setCommand(WiFiControl::Command::RIGHT);
        else if (cmd == "STOP") instancePtr->setCommand(WiFiControl::Command::STOP);
        else if (cmd == "AUTO") instancePtr->setCommand(WiFiControl::Command::AUTO);
        else response = "UNKNOWN";
    }

    server.send(200, "text/plain", response);
}

WiFiControl::WiFiControl() : currentCommand(Command::NONE) {}

void WiFiControl::begin(const char* ssid, const char* password) {
    instancePtr = this;

    WiFi.softAP(ssid, password);
    delay(500);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("[WiFi] AP IP: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/control", handleControl);
    server.begin();
}

WiFiControl::Command WiFiControl::getCommand() {
    return currentCommand;
}

void WiFiControl::setCommand(Command cmd) {
    currentCommand = cmd;
}

void WiFiControl::handle() {
    server.handleClient();
}

void WiFiControl::clearCommand() {
    currentCommand = Command::NONE;
}
