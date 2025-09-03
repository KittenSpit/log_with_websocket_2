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


// Variable to store incoming data
String receivedData = "";

// Variables for sensor simulation (temp, humidity, light)
float temp = 0.00;
float hum = 0.00;
int light = 0;
String source = "ESP32-Sim";
int new_client = 0;
String data = String("{\"s1\":") + 0.0 + ",\"s2\":" + 0.0 + ",\"s3\":" + 0 + ",\"s4\":" + 0 + "}";

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.print("WebSocket client connected  ");
    Serial.println(num);
    new_client=1;
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
  //read webpage from LittleFS
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
if (new_client ==1) {
     webSocket.broadcastTXT(data);
     new_client =0;
}


if (delay1.ok_togo()){

  // Generate random data for testing
  generateRandomData();

  // If there's new data, send it to the MySQL database via an HTTP API
  //if (receivedData != "1") {
    logToMySQL(temp, hum, light, source);
    Serial.println("log data");
    // changed from string to char[] when trying to fix format?
    //internet suggest that strings can cause memory faults if used alot?
char hum_c[10];
dtostrf(hum, -6,2,hum_c);

    data = String("{\"s1\":") + String(temp,2) + ",\"s2\":" + hum_c + ",\"s3\":" + String(light) + ",\"s4\":" + source + "}";
    webSocket.broadcastTXT(data);
   Serial.print("Display data  ");
   Serial.println(data);

   // receivedData = ""; // Reset the data after sending it
  //}

  
  }
}


