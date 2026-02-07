/*
 * Bitcoin Timechain Calendar - ESP32 Web Server
 * 
 * Fetches Bitcoin block height AND receives messages from the companion app
 * Messages received will print to Serial Monitor
 * 
 * Libraries needed:
 * - ArduinoJson
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials - change these to your network
const char* ssid = "VM1475403";
const char* password = "vjdg5HZhv3jqqaai";

// Web server on port 80
WebServer server(80);

// API endpoint for block height
const char* blockHeightAPI = "https://mempool.space/api/blocks/tip/height";

// Global variables
int currentBlockHeight = 0;
String lastMessage = "";
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 600000; // 10 minutes

// -----------------------------------------------
// CORS headers - needed so the app can talk to the ESP32
// -----------------------------------------------
void setCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// -----------------------------------------------
// Web Server Routes
// -----------------------------------------------

// GET /api/status - returns current block height and last message
void handleStatus() {
  setCORSHeaders();
  
  StaticJsonDocument<256> doc;
  doc["blockHeight"] = currentBlockHeight;
  doc["lastMessage"] = lastMessage;
  doc["uptime"] = millis() / 1000;
  doc["wifiStrength"] = WiFi.RSSI();
  doc["ipAddress"] = WiFi.localIP().toString();
  
  String json;
  serializeJson(doc, json);
  
  server.send(200, "application/json", json);
}

// POST /api/message - receives a message from the app
void handleMessage() {
  setCORSHeaders();
  
  if (server.hasArg("plain")) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, server.arg("plain"));
    
    String incomingMessage = doc["message"].as<String>();
    lastMessage = incomingMessage;
    
    // Print to Serial Monitor - this is where you'll see it!
    Serial.println("=============================");
    Serial.println("  MESSAGE FROM APP:");
    Serial.println("=============================");
    Serial.println("  " + incomingMessage);
    Serial.println("=============================");
    Serial.println();
    
    // Send back a success response
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["received"] = incomingMessage;
    
    String json;
    serializeJson(response, json);
    server.send(200, "application/json", json);
  } else {
    server.send(400, "application/json", "{\"error\":\"No message received\"}");
  }
}

// Handle OPTIONS preflight requests (needed for CORS)
void handleOptions() {
  setCORSHeaders();
  server.send(200, "text/plain", "");
}

// -----------------------------------------------
// WiFi Connection
// -----------------------------------------------
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("=============================");
  Serial.println("  WiFi Connected!");
  Serial.print("  IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("=============================");
  Serial.println();
  Serial.println("  Open your app and enter this IP:");
  Serial.print("  http://");
  Serial.println(WiFi.localIP());
  Serial.println("=============================");
  Serial.println();
}

// -----------------------------------------------
// Fetch Bitcoin Block Height
// -----------------------------------------------
void fetchBlockHeight() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    Serial.println("Fetching block height...");
    http.begin(blockHeightAPI);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      int newBlockHeight = payload.toInt();
      
      if (newBlockHeight > currentBlockHeight) {
        currentBlockHeight = newBlockHeight;
        Serial.println("=============================");
        Serial.print("  New Block: ");
        Serial.println(currentBlockHeight);
        Serial.println("=============================");
        Serial.println();
      }
    } else {
      Serial.println("Block height fetch failed");
    }
    
    http.end();
  }
}

// -----------------------------------------------
// Setup & Loop
// -----------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("=============================");
  Serial.println("  Bitcoin Timechain Calendar");
  Serial.println("  Starting up...");
  Serial.println("=============================");
  Serial.println();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Set up web server routes
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/message", HTTP_POST, handleMessage);
  server.on("/api/status", HTTP_OPTIONS, handleOptions);
  server.on("/api/message", HTTP_OPTIONS, handleOptions);
  
  server.begin();
  Serial.println("Web server started!");
  Serial.println();
  
  // Fetch initial block height
  fetchBlockHeight();
}

void loop() {
  // Handle incoming web requests from the app
  server.handleClient();
  
  // Fetch block height every 10 minutes
  if (millis() - lastUpdate >= updateInterval) {
    fetchBlockHeight();
    lastUpdate = millis();
  }
  
  delay(10);
}
