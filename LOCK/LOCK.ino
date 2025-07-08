#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> 
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// ========== WiFi Credentials ==========
const char* ssid = "Airtel_Zerotouch";
const char* password = "Airtel@123";

// ========== Google Script Web App URL ==========
const String scriptURL = "https://script.google.com/macros/s/AKfycbxIuiHkbGxb8J1jTtXPExPF2u9lz3bhIPECN8t95_-0D9rr2tzy1o9f86ETrIBTCYcK/exec";

// ========== Relay Pin ==========
#define RELAY_PIN D1  // GPIO5

// ========== Fingerprint Setup ==========
SoftwareSerial fingerSerial(D5, D6); // RX (to TX of sensor), TX (to RX of sensor)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Lock engaged by default

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected.");
  } else {
    Serial.println("Fingerprint sensor NOT found.");
    while (1);
  }

  finger.getTemplateCount();
  Serial.print("Stored templates: "); Serial.println(finger.templateCount);
}

void loop() {
  uint8_t id = getFingerprintID();
  if (id != 0) {
    unlockDoor(id);
    delay(5000);  // Keep door unlocked for 5 seconds
    digitalWrite(RELAY_PIN, LOW);  // Lock again
  }
  delay(1000);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return 0;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return 0;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return 0;

  Serial.print("User ID "); Serial.print(finger.fingerID); Serial.println(" recognized.");
  return finger.fingerID;
}

void unlockDoor(uint8_t id) {
  digitalWrite(RELAY_PIN, HIGH);
  String user = "User_" + String(id);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Skip certificate validation (not recommended for production)

    HTTPClient https;
    String url = scriptURL + "?user=" + user;

    https.begin(client, url);  // Secure HTTPS connection
    https.addHeader("User-Agent", "ESP8266");

    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.println("Log Sent: " + user);
    } else {
      Serial.print("Log Failed, HTTP code: ");
      Serial.println(httpCode);
    }

    https.end();
  } else {
    Serial.println("WiFi Disconnected - Can't log");
  }
}
