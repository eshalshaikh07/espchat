#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>

// Replace with your network credentials
const char* ssid = "Eshal";
const char* password = "Eshal123";

// Firebase project credentials
const String firebaseHost = "https://chats-14529-default-rtdb.firebaseio.com/";
const String firebaseAuth = "AIzaSyCPy96R2oLf6ow_kdTLlIrkqvAUefPdSow";

String sender = "ESP32_1";  // Change this identifier for Board 2

std::map<String, bool> displayedMessages;  // Track displayed messages

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  
  Serial.println("Enter your message:");
}

void loop() {
  // Receive messages
  receiveMessages();
  
  // Check for input from the Serial Monitor
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    sendMessage(message, sender);
  }
  
  delay(10000); // Wait 10 seconds before checking for new messages again
}

void sendMessage(const String& message, const String& sender) {
  HTTPClient http;
  String url = firebaseHost + "/chats/messages.json?auth=" + firebaseAuth;

  // Prepare the JSON payload
  StaticJsonDocument<200> doc;
  doc["message"] = message;
  doc["sender"] = sender;
  doc["timestamp"] = millis();
  String payload;
  serializeJson(doc, payload);

  // Send HTTP POST request
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Failed to send message: " + String(httpResponseCode));
  }
  http.end();
}

void receiveMessages() {
  HTTPClient http;
  String url = firebaseHost + "/chats/messages.json?auth=" + firebaseAuth;

  // Send HTTP GET request
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    parseMessages(response);
  } else {
    Serial.println("Failed to get messages: " + String(httpResponseCode));
  }
  http.end();
}

void parseMessages(const String& response) {
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  for (JsonPair kv : doc.as<JsonObject>()) {
    String messageId = kv.key().c_str();
    if (displayedMessages.find(messageId) == displayedMessages.end()) {
      JsonObject messageObj = kv.value().as<JsonObject>();
      String message = messageObj["message"];
      String sender = messageObj["sender"];
      Serial.println("New message from " + sender + ": " + message);
      displayedMessages[messageId] = true;  // Mark this message as displayed
    }
  }
}