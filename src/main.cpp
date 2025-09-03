#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Wait2.h>
#include <LittleFS.h>

const char* ssid = "PHD1 2.4";
const char* password = "Andrew1Laura2";
const String apiUrl = "http://adamnet.ca/arduino/log2/log.php"; // API URL to log data to MySQL


WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket port

 //1000 millis = 1 second
Wait2 delay1 (30000); 

unsigned long lastUpload = 0;
const long uploadInterval = 30000; // 30 sec


// Variable to store incoming data
String receivedData = "";

// Variables for sensor simulation (temp, humidity, light)
float temp = 0.00;
float hum = 0.00;
int light = 0;
String source = "ESP32-Sim";
int push = 0;
String data = String("{\"s1\":") + 0.0 + ",\"s2\":" + 0.0 + ",\"s3\":" + 0 + ",\"s4\":" + 0 + "}";

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.print("WebSocket client connected  ");
    Serial.println(num);
    push=1;
  } else if (type == WStype_DISCONNECTED) {
    Serial.print("WebSocket client disconnected  ");
    Serial.println(num);
  }
}


// Function to generate random data for temp, hum, and light
void generateRandomData() {
  temp = random(180, 300) / 10.000; // Random temperature between 18.0 and 30.0°C
  hum = random(300, 800) / 10.000; // Random humidity between 30.0 and 80.0%
  light = random(0, 1024); // Random light intensity between 0 and 1023
  source = 999; // Hardcoded source name
  Serial.print("Generated data: Temp = ");
  Serial.print(temp);
  Serial.print("°C, Hum = ");
  Serial.print(hum);
  Serial.print("%, Light = ");
  Serial.print(light);
  Serial.print(", Source = ");
  Serial.println(source);
}

// Function to send data to the MySQL database via HTTP POST
void logToMySQL(float temp, float hum, int light, String source) {
  HTTPClient http;
  String serverPath = apiUrl + "?temp=" + String(temp,2) + "&hum=" + String(hum,2) + "&light=" + String(light) + "&source=" + source;
  
  http.begin(serverPath); // Specify the URL

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send the request
  int httpResponseCode = http.GET();

  // Check the response code
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Sensor Dashboard</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: #f0f4f8;
      color: #333;
      margin: 0;
      padding: 2rem;
      display: flex;
      justify-content: center;
      align-items: flex-start;
      min-height: 100vh;
    }

    .dashboard {
      background: white;
      padding: 2rem 3rem;
      border-radius: 12px;
      box-shadow: 0 8px 24px rgba(0, 0, 0, 0.1);
      width: 320px;
      text-align: center;
    }

    h2 {
      margin-bottom: 1.5rem;
      color: #007acc;
      font-weight: 700;
      font-size: 1.8rem;
    }

    .sensor {
      background: #e9f1fc;
      border-radius: 8px;
      padding: 1rem 1.5rem;
      margin-bottom: 1rem;
      display: flex;
      align-items: center;
      justify-content: space-between;
      font-size: 1.1rem;
      box-shadow: inset 0 0 8px #d0e4fc;
    }

    .sensor span.label {
      font-weight: 600;
      color: #005a9e;
    }

    .sensor span.value {
      font-weight: 700;
      font-size: 1.4rem;
      color: #003d66;
      min-width: 60px;
      text-align: right;
    }

    /* Optional: subtle animation when values update */
    .sensor span.value.update {
      animation: pulse 0.5s ease-out;
    }

    @keyframes pulse {
      0% {
        background-color: #cce4ff;
        border-radius: 4px;
      }
      100% {
        background-color: transparent;
      }
    }
  </style>

  <script>
    let ws = new WebSocket("ws://" + location.hostname + ":81/");
    ws.onmessage = function (event) {
      let data = JSON.parse(event.data);

      const updateValue = (id, value, unit = "") => {
        const el = document.getElementById(id);
        if (!el) return;
        const newValue = id.match(/^f/) ? value.toFixed(2) : value;
        if (el.innerText !== newValue + unit) {
          el.innerText = newValue + unit;
          el.classList.add("update");
          setTimeout(() => el.classList.remove("update"), 500);
        }
      };

      updateValue("f1", data.s1, " °C");
      updateValue("f2", data.s2, " %");
      updateValue("s3", data.s3, "");
      updateValue("s4", data.s4, "");
    };
  </script>
</head>
<body>
  <div class="dashboard">
    <h2>Live Sensor Data</h2>
    <div class="sensor">
      <span class="label">Temperature</span>
      <span id="f1" class="value">-- °C</span>
    </div>
    <div class="sensor">
      <span class="label">Humidity</span>
      <span id="f2" class="value">-- %</span>
    </div>
    <div class="sensor">
      <span class="label">Light Level</span>
      <span id="s3" class="value">--</span>
    </div>
    <div class="sensor">
      <span class="label">Source</span>
      <span id="s4" class="value">--</span>
    </div>
  </div>
</body>
</html>
)rawliteral";




void setup() {
  Serial.begin(9600);
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

   Serial.println(WiFi.localIP());

//  server.on("/", HTTP_GET, []() {
//    server.send(200, "text/html", htmlPage);

if (!LITTLEFS.begin()) {
    Serial.println("LITTLEFS mount failed! Formatting...");
    if (LITTLEFS.format()) {
      Serial.println("Formatting successful! Trying to mount again...");
      if (LITTLEFS.begin()) {
        Serial.println("LittleFS mounted after formatting");
      } else {
        Serial.println("Mount failed even after formatting");
      }
    } else {
      Serial.println("Formatting failed");
    }
  } else {
    Serial.println("LittleFS mounted successfully");
  }





server.on("/", HTTP_GET, []() {
 File file = LITTLEFS.open("/index.html", "r");
server.streamFile(file, "text/html");
file.close();

  });



   server.begin();
  // Start the WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // Handle WebSocket communication
  server.handleClient();
  webSocket.loop();
if (push ==1) {
     webSocket.broadcastTXT(data);
     push =0;
}

//  unsigned long now = millis();
//  if (now - lastUpload >= uploadInterval) {
 //   lastUpload = now;
if (delay1.ok_togo()){



   //delay(15000);  
  // Generate random data for testing
  generateRandomData();

  // If there's new data, send it to the MySQL database via an HTTP API
  //if (receivedData != "1") {
    logToMySQL(temp, hum, light, source);
    Serial.println("log data");
char tempHum[10];
dtostrf(hum, -6,2,tempHum);

    data = String("{\"s1\":") + String(temp,2) + ",\"s2\":" + tempHum + ",\"s3\":" + String(light) + ",\"s4\":" + source + "}";
    webSocket.broadcastTXT(data);
   Serial.print("Display data  ");
   Serial.println(data);

   // receivedData = ""; // Reset the data after sending it
  //}

  // Optional: Add a delay between logging (every 5 seconds for example)
  //delay(15000);  
  }
}


