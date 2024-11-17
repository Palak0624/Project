#include <WiFiNINA.h>
#include <HTTPClient.h>
#include <Twilio.h>
#include <Arduino_MAX30102.h>
#include <PulseSensorPlugin.h>

// Wi-Fi Credentials
const char* ssid = "Palak";
const char* password = "11223344";
const char* serverIP = "192.168.65.252"; 
const int serverPort = 5000;

// ThingSpeak Credentials
const char* apiKey = "M92AZJD2DA38OTXB";
const char* server = "api.thingspeak.com";

// Twilio Credentials
const char* accountSid = "MG13beed3d3141a15e5566218287f5d710";
const char* authToken = "AC5696048cd1c8df42f2e639e36b8b8fcf";
const char* twilioNumber = "+1234567890"; 
const char* myNumber = "+917617724456";

// Sensor Pins and Thresholds
const int trigPin = 2;
const int echoPin = 3;
const int soilMoistureSensorPin = A0;

const int distanceThreshold = 25;
const int highHeartRateThreshold = 180;
const int lowHeartRateThreshold = 60;
const int wetBedThreshold = 800; 
PulseSensorPlugin pulseSensor;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize MAX30102 (PulseSensor)
  if (!pulseSensor.begin()) {
    Serial.println("MAX30102 Initialization failed!");
    while (1);
  }

  // Initialize sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize Twilio
  Twilio.begin(accountSid, authToken, twilioNumber);
}

void sendCaptureRequest() {
  WiFiClient client;
  if (client.connect(serverIP, serverPort)) {
    client.println("GET /trigger HTTP/1.1");
    client.println("Host: " + String(serverIP));
    client.println("Connection: close");
    client.println();
    client.stop();
  } else {
    Serial.println("Failed to connect to server.");
  }
}

void loop() {
  // Read distance from HC-SR04
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Read heart rate from MAX30102
  int bpm = pulseSensor.getBeatsPerMinute();

  // Read soil moisture level
  int soilMoisture = analogRead(soilMoistureSensorPin);

  // Print values for debugging
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Pulse: ");
  Serial.print(bpm);

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  // Check for alert conditions
  bool alertTriggered = false;
  String message = "Alert! ";

  if (distance > distanceThreshold) {
    message += "Baby is not there. ";
    alertTriggered = true;
  }
  
  if (soilMoisture > wetBedThreshold) {
    message += "Bed is wet. ";
    alertTriggered = true;
  }
  
  if (bpm > highHeartRateThreshold || bpm < lowHeartRateThreshold) {
    message += "Abnormal heart rate detected. ";
    alertTriggered = true;
  }

  // If an alert was triggered, send the SMS and ThingSpeak data
  if (alertTriggered) {
    // Send SMS using Twilio
    Twilio.sendMessage(myNumber, message);
    Serial.println("Alert sent via SMS");

    // Send data to ThingSpeak
    String url = "https://" + String(server) + "/update?api_key=" + apiKey + "&field1=" + String(distance) + "&field2=" + String(bpm) + "&field3=" + String(soilMoisture);
    HTTPClient http;
    http.begin(url);
    int httpCode = http.POST("");
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("Data sent to ThingSpeak successfully");
    } else {
      Serial.println("HTTP Request failed");
    }
    http.end();

    // Optionally send a capture request to another server
    sendCaptureRequest();
  }

  // Wait before next loop iteration
  delay(1000); 
}
