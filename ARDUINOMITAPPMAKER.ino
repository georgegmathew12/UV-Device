#include <ArduinoBLE.h>


// Define BLE service and characteristics UUIDs
BLEService service("fd85bd5c-08b7-48ac-bd2c-c6c98caf259f");
BLEByteCharacteristic inputChar1("00002a05-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite);
BLEByteCharacteristic inputChar2("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLEWrite);
BLEFloatCharacteristic outputChar("32c3c9a8-e995-4f72-ab00-e691e4d425c3", BLERead | BLEWrite | BLENotify);

int uvSensorPin = A0;

//  Calculated UV Index
float uvIntensity; 

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1);
  }
  
  BLE.setLocalName("Nano 33 IoT");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(inputChar1);
  service.addCharacteristic(inputChar2);
  service.addCharacteristic(outputChar);
  BLE.addService(service);
  
  //outputChar.setEventHandler(BLEWritten, onOutputWritten);
inputChar1.setValue(0);
inputChar2.setValue(0);
  
  BLE.advertise();
  Serial.println("BLE server started!");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      int age = inputChar1.value();
      if (age != inputChar1.written()) {
        Serial.print("Age: ");
        Serial.println(age);
        // do something with the incoming data
      }  
      int Fitz = inputChar2.value();
      if (Fitz != inputChar2.written()) {
        Serial.print("Fitz: ");
        Serial.println(Fitz);
        
      }
      int uvSensorValue = analogRead(uvSensorPin);
      float voltage = uvSensorValue * (5.0 / 1023.0);

  // Convert voltage to UV intensity in mW/cm²
      uvIntensity = voltage / 0.1;
      outputChar.writeValue(uvIntensity);

  // Send measured UV Index to app for it to display
      Serial.print("   UV Index: ");
      Serial.print(uvIntensity, 5);
      Serial.print(" mW/cm²");
      delay(500);


    }
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

}

//void onOutputWritten(BLEDevice central, BLECharacteristic characteristic) {
 // int data = 42;
  //utputChar.writeValue(data);
  //Serial.print("Received output: ");
  //Serial.println(data);


