#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "PHD1 2.4";
const char* password = "Andrew1Laura2";
const String apiUrl = "http://adamnet.ca/arduino/log2/log.php"; // API URL to log data to MySQL


WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket port


unsigned long lastUpload = 0;
const long uploadInterval = 30000; // 30 sec


// Variable to store incoming data
String receivedData = "";

// Variables for sensor simulation (temp, humidity, light)
float temp = 0.0;
float hum = 0.0;
int light = 0;
String source = "ESP32-Sim";
int push = 0;
String data = String("{\"s1\":") + 0 + ",\"s2\":" + 0 + ",\"s3\":" + 0 + ",\"s4\":" + 0 + "}";

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
  temp = random(180, 300) / 10.0; // Random temperature between 18.0 and 30.0°C
  hum = random(300, 800) / 10.0; // Random humidity between 30.0 and 80.0%
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
  String serverPath = apiUrl + "?temp=" + String(temp) + "&hum=" + String(hum) + "&light=" + String(light) + "&source=" + source;
  
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
<html>
<head>
  <title>Sensor Dashboard</title>
  <script>
    let ws = new WebSocket("ws://" + location.hostname + ":81/");
    ws.onmessage = function(event) {
      let data = JSON.parse(event.data);
      document.getElementById("s1").innerText = data.s1;
      document.getElementById("s2").innerText = data.s2;
      document.getElementById("s3").innerText = data.s3;
      document.getElementById("s4").innerText = data.s4;
    };
  </script>
</head>
<body>
  <h2>Live Sensor Data</h2>
  <p>Sensor 1: <span id="s1">--</span> C</p>
  <p>Sensor 2: <span id="s2">--</span> %</p>
  <p>Sensor 3: <span id="s3">--</span> units</p>
  <p>Sensor 4: <span id="s4">--</span></p>
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

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage);
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

  unsigned long now = millis();
  if (now - lastUpload >= uploadInterval) {
    lastUpload = now;




   //delay(15000);  
  // Generate random data for testing
  generateRandomData();

  // If there's new data, send it to the MySQL database via an HTTP API
  //if (receivedData != "1") {
    logToMySQL(temp, hum, light, source);
    Serial.println("log data");

    data = String("{\"s1\":") + temp + ",\"s2\":" + hum + ",\"s3\":" + light + ",\"s4\":" + source + "}";
    webSocket.broadcastTXT(data);
   Serial.print("Display data  ");
   Serial.println(data);

   // receivedData = ""; // Reset the data after sending it
  //}

  // Optional: Add a delay between logging (every 5 seconds for example)
  //delay(15000);  
  }
}


