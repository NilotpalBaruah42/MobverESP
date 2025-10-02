#include <WiFi.h>
#include <WebServer.h>

// ------------------- WiFi Settings -------------------
const char *ssid = "MobverESP";     // Name of hotspot
const char *password = "moverPass";  // Password

WebServer server(80);

// ------------------- Motor Pins -------------------
// Right Motor
int enableRightMotor = 22;
int rightMotorPin1   = 16;
int rightMotorPin2   = 17;

// Left Motor
int enableLeftMotor  = 23;
int leftMotorPin1    = 18;
int leftMotorPin2    = 19;

// ------------------- HTML Page -------------------
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>ESP32 Car Control</title>
  <style>
    body { text-align:center; font-family: Arial; background:#2c3e50; color:white; }
    button {
      width:80px; height:80px; font-size:24px; margin:10px;
      border-radius:10px; border:none; background:#ffc107; color:black;
      font-weight:bold; cursor:pointer;
    }
    button:active { background:#ff9800; }
  </style>
</head>
<body>
  <h1>ESP32 Car Control</h1>
  <div><button id="up">↑</button></div>
  <div>
    <button id="left">←</button>
    <button id="stop">■</button>
    <button id="right">→</button>
  </div>
  <div><button id="down">↓</button></div>

  <script>
    function sendRequest(url) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", url, true);
      xhttp.send();
      console.log("Sent request:", url);
    }

    document.addEventListener("DOMContentLoaded", function() {
      const controls = {
        up: "/forward",
        down: "/backward",
        left: "/left",
        right: "/right",
        stop: "/stop"
      };

      for (let id in controls) {
        let btn = document.getElementById(id);
        if (!btn) continue;

        if (id === "stop") {
          btn.addEventListener("click", () => sendRequest(controls[id]));
        } else {
          btn.addEventListener("mousedown", () => sendRequest(controls[id]));
          btn.addEventListener("mouseup", () => sendRequest("/stop"));
          btn.addEventListener("touchstart", () => sendRequest(controls[id]));
          btn.addEventListener("touchend", () => sendRequest("/stop"));
        }
      }
    });

    // --- Gamepad Controls ---
    window.addEventListener("gamepadconnected", function(e) {
      console.log("Gamepad connected:", e.gamepad);
      const gp = e.gamepad;

      function update() {
        const gamepad = navigator.getGamepads()[gp.index];
        if (gamepad) {
          if (gamepad.buttons[12].pressed) { sendRequest("/forward"); console.log("UP"); }
          if (gamepad.buttons[13].pressed) { sendRequest("/backward"); console.log("DOWN"); }
          if (gamepad.buttons[14].pressed) { sendRequest("/left"); console.log("LEFT"); }
          if (gamepad.buttons[15].pressed) { sendRequest("/right"); console.log("RIGHT"); }
          if (gamepad.buttons[0].pressed)  { sendRequest("/stop"); console.log("STOP"); }
        }
        requestAnimationFrame(update);
      }
      update();
    });
  </script>
</body>
</html>
)rawliteral";

// ------------------- Motor Functions -------------------
void stopMotors() {
  digitalWrite(rightMotorPin1, LOW);
  digitalWrite(rightMotorPin2, LOW);
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, LOW);
}

void moveForward() {
  digitalWrite(rightMotorPin1, HIGH);
  digitalWrite(rightMotorPin2, LOW);
  digitalWrite(leftMotorPin1, HIGH);
  digitalWrite(leftMotorPin2, LOW);
}

void moveBackward() {
  digitalWrite(rightMotorPin1, LOW);
  digitalWrite(rightMotorPin2, HIGH);
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, HIGH);
}

void turnLeft() {
  digitalWrite(rightMotorPin1, HIGH);
  digitalWrite(rightMotorPin2, LOW);
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, HIGH);
}

void turnRight() {
  digitalWrite(rightMotorPin1, LOW);
  digitalWrite(rightMotorPin2, HIGH);
  digitalWrite(leftMotorPin1, HIGH);
  digitalWrite(leftMotorPin2, LOW);
}

// ------------------- Server Handlers -------------------
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void handleForward() { moveForward(); server.send(200, "text/plain", "Moving Forward"); }
void handleBackward(){ moveBackward(); server.send(200, "text/plain", "Moving Backward"); }
void handleLeft()    { turnLeft();    server.send(200, "text/plain", "Turning Left"); }
void handleRight()   { turnRight();   server.send(200, "text/plain", "Turning Right"); }
void handleStop()    { stopMotors();  server.send(200, "text/plain", "Stopped"); }

// ------------------- Setup & Loop -------------------
void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  // Enable motors (PWM could be used here)
  analogWrite(enableRightMotor, 255);
  analogWrite(enableLeftMotor, 255);

  // Start WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("WiFi started. Connect to: " + String(ssid));

  // Routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}