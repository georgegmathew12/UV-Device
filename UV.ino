#include <ArduinoBLE.h>

// Constants for Bluetooth connection
const char* DEVICE_NAME = "Nano 33 IoT";
const char* SERVICE_UUID = "fd85bd5c-08b7-48ac-bd2c-c6c98caf259f";
const char* INPUT_CHAR1_UUID = "00002a05-0000-1000-8000-00805f9b34fb";
const char* INPUT_CHAR2_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const char* OUTPUT_CHAR1_UUID = "32c3c9a8-e995-4f72-ab00-e691e4d425c3";
const char* OUTPUT_CHAR2_UUID = "0a6aae4a-bcea-44eb-bc8a-313e86f769a9";

// Pin configuration
const int UV_SENSOR_PIN = A0;
const int LED_PIN = 12;

// Timing configuration
const int DELAY_TIME_MS = 500;
const float DELAY_TIME_SEC = static_cast<float>(DELAY_TIME_MS) / 1000;

// Function declarations
void setupBluetooth();
void readAndSendUVIndex();
void calculateUVIndexCounts(float uvIntensity);
void updateSkinDamageScales(int skintone, int age);
void checkAndSendUVExposureWarning();

// Global variables
float uvIntensity;
float UVindoorcnt = 0;
float UVlowcnt = 0;
float UVmidcnt = 0;
float UVhighcnt = 0;
float UVlowdamage = 0;
float UVmiddamage = 0;
float UVhighdamage = 0;
float UVtotaldamage = 0;
float lowdamage, middamage, highdamage;
int skintone;
int age;
int wait = 1;
int yes = 1;
int no = 1;

BLEService service(SERVICE_UUID);
BLEByteCharacteristic inputChar1(INPUT_CHAR1_UUID, BLERead | BLEWrite);
BLEByteCharacteristic inputChar2(INPUT_CHAR2_UUID, BLERead | BLEWrite);
BLEFloatCharacteristic outputChar1(OUTPUT_CHAR1_UUID, BLERead | BLEWrite);
BLEFloatCharacteristic outputChar2(OUTPUT_CHAR2_UUID, BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  setupBluetooth();

  // Set LED_PIN as output and turn on the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Optional: Tell user skin protection recommendations
  Serial.println("For best protection against UV radiation, wear sunscreen and cover skin with clothing.");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    while (central.connected()) {
      age = inputChar1.value();
      skintone = inputChar2.value();
    }

    updateSkinDamageScales(skintone, age);
    readAndSendUVIndex();
    calculateUVIndexCounts(uvIntensity);
    checkAndSendUVExposureWarning();
    delay(DELAY_TIME_MS);
  }
}

void setupBluetooth() {
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1);
  }

  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(service);
  service.addCharacteristic(inputChar1);
  service.addCharacteristic(inputChar2);
  service.addCharacteristic(outputChar1);
  service.addCharacteristic(outputChar2);
  BLE.addService(service);

  inputChar1.setValue(0);
  inputChar2.setValue(0);

  BLE.advertise();
}

void readAndSendUVIndex() {
  int uvSensorValue = analogRead(UV_SENSOR_PIN);
  float voltage = uvSensorValue * (5.0 / 1023.0);

  uvIntensity = voltage / 0.1;
  outputChar.writeValue(uvIntensity);

  Serial.print(" UV Index: ");
  Serial.print(uvIntensity, 5);
  Serial.print(" mW/cm²");
  Serial.println();
}

void calculateUVIndexCounts(float uvIntensity) {
  if (uvIntensity <= 1) {
    UVindoorcnt += 1;
  } else if (uvIntensity <= 3) {
    UVlowcnt += 1;
  } else if (uvIntensity > 3 && uvIntensity < 6) {
    UVmidcnt += 1;
  } else if (uvIntensity >= 6) {
    UVhighcnt += 1;
  }
}

void updateSkinDamageScales(int skintone, int age) {
// Customize damage scale according to skin tone
  switch (skintone) {
    case 1:
      lowdamage = 3;
      middamage = 4.5;
      highdamage = 6;
    break;
    case 2:
      lowdamage = 2.6;
      middamage = 3.9;
      highdamage = 5.2;
    break;
    case 3:
      lowdamage = 2.2;
      middamage = 3.3;
      highdamage = 4.4;
    break;
    case 4:
      lowdamage = 1.8;
      middamage = 2.6;
      highdamage = 3.6;
    break;
    case 5:
      lowdamage = 1.4;
      middamage = 2.0;
      highdamage = 2.8;
    break;
    case 6:
      lowdamage = 1.0;
      middamage = 1.4;
      highdamage = 2.0;
    break;
  }
  // Customize damage scale according to age
  if (no == 1) {
      if (age > 55) {
      lowdamage *= 1.2;
      middamage *= 1.2;
      highdamage *= 1.2;
      no = 0;
      }
  }
}

void checkAndSendUVExposureWarning() {
  // Calculate cumulative damage from the amount of time spent in each index range
  UVlowdamage = UVlowcnt * DELAY_TIME_SEC * lowdamage / 60.0;
  UVmiddamage = UVmidcnt * DELAY_TIME_SEC * middamage / 60.0;
  UVhighdamage = UVhighcnt * DELAY_TIME_SEC * highdamage / 60.0;
  UVtotaldamage = UVlowdamage + UVmiddamage + UVhighdamage;

// Reset cumulative damage if the user has spent more than 30 minutes indoors
  if (UVindoorcnt > 3600) {
    UVlowcnt = 0;
    UVmidcnt = 0;
    UVhighcnt = 0;
    UVlowdamage = 0;
    UVmiddamage = 0;
    UVhighdamage = 0;
    UVtotaldamage = 0;
    UVindoorcnt = 0;
  }

  // Send warning if cumulative damage exceeds the dangerous amount
  if (yes == 1) {
    if (UVtotaldamage >= 70) {
      float UVtotaltime = (UVlowcnt + UVmidcnt + UVhighcnt) * DELAY_TIME_SEC / 60.0;
      Serial.println();
      Serial.println("You have been exposed to a dangerous amount of UV radiation.");
      Serial.print("You have experienced a dangerous amount of cumulated UV exposure for ");
      Serial.print(UVtotaltime);
      Serial.print(" minutes.");
      // Wait 600 loops or 5 minutes before checking for dangerous levels again
      wait = 600;
      yes = 0;
    }
  } 
  else if (wait > 1) {
      wait -= 1;
      if (wait == 1) {
        yes = 1;
      }
  }
}
