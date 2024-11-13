#include <WiFiNINA.h>
#include <HTTPClient.h>
#include <Twilio.h>
#include <Arduino_MAX30102.h>
#include <PulseSensorPlugin.h>

// Wi-Fi Credentials
const char* ssid = "Palak";
const char* password = "11223344";

// ThingSpeak Credentials
const char* apiKey = "M92AZJD2DA38OTXB";
const char* server = "api.thingspeak.com";

// Twilio Credentials
const char* accountSid = "MG13beed3d3141a15e5566218287f5d710";
const char* authToken = "AC5696048cd1c8df42f2e639e36b8b8fcf";
const char* twilioNumber = "+1234567890"; // Your Twilio phone number
const char* myNumber = "+917617724456"; // Your phone number

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

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize MAX30102
  if (!pulseSensor.begin()) {
    Serial.println("MAX30102 Initialization failed!");
    while (1);
  }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize Twilio
  Twilio.begin(accountSid, authToken, twilioNumber);
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

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Heart Rate: ");
  Serial.print(bpm);
  Serial.println(" bpm");

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  // Check for alert conditions and send SMS
  if (distance > distanceThreshold || soilMoisture > wetBedThreshold || (bpm > highHeartRateThreshold || bpm < lowHeartRateThreshold)) {
    String message = "Alert! ";
    if (distance > distanceThreshold) {
      message += "Baby is not there. ";
    }
    if (soilMoisture > wetBedThreshold) {
      message += "Bed is wet. ";
    }
    if (bpm > highHeartRateThreshold || bpm < lowHeartRateThreshold) {
      message += "Abnormal heart rate detected. ";
    }
    Twilio.sendMessage(myNumber, message);
    Serial.println("Alert sent via SMS");
  }

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

  delay(1000); // Adjust the delay as needed
}