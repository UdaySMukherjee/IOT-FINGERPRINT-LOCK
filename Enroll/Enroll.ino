#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial fingerSerial(D5, D6);  // D5 = RX (to TX of sensor), D6 = TX (to RX of sensor)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

uint8_t id = 1; // Change this to enroll under a different ID

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(1000);

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected.");
  } else {
    Serial.println("Fingerprint sensor not detected. Check wiring.");
    while (1);
  }

  Serial.print(F("Waiting for valid finger to enroll as ID #"));
  Serial.println(id);

  enrollFinger(id);
}


void loop() {
  // Nothing here
}

uint8_t enrollFinger(uint8_t id) {
  int p = -1;
  Serial.println("Place finger on sensor...");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        delay(100);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // Convert image to characteristics and store in buffer 1
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Image to template failed");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);

  // Wait for same finger again
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
    delay(100);
  }

  Serial.println("Place same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Image to template failed (second time)");
    return p;
  }

  // Create model
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint matched");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // Store in flash memory
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored successfully!");
  } else {
    Serial.println("Failed to store fingerprint");
    return p;
  }

  return true;
}
