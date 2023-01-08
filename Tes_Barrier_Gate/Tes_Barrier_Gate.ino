#include <ESP32_Servo.h>
#include <BluetoothSerial.h>

#define echoPin     12
#define trigPin     14

#define servoPin    13

#define greenLight  19
#define redLight    21

#define gateButton  2

Servo gate;

BluetoothSerial SerialBT;

enum gate_status{
  gate_open,
  gate_closed,
  gate_waiting,
  status_count
};

int openCount = 0, openThreshold = 8;

int gateStatus = gate_waiting;

bool statusLED = LOW;

char* list_licensePlate[] = {
  "A 1234 BC", "D 5678 EF", 0
};

float readDistance(){
  long duration; // variable for the duration of sound wave travel
  float distance; // variable for the distance measurement
  float max_size = 0, size_; //size of object
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

  return distance;
}

void openGate(){
  digitalWrite(greenLight, HIGH);
  digitalWrite(redLight, LOW);
  gate.writeMicroseconds(1360);
}

void closeGate(){
  digitalWrite(greenLight, LOW);
  digitalWrite(redLight, HIGH);
  gate.writeMicroseconds(560);
}

void blinkGreenLight(){
  digitalWrite(redLight, LOW);
  digitalWrite(greenLight, LOW);
  delay(100);
  digitalWrite(greenLight, HIGH);
  delay(100);
}

void blinkRedLight(){
  digitalWrite(greenLight, LOW);
  for(int i = 0; i < 10; i++){
    digitalWrite(redLight, LOW);
    delay(100);
    digitalWrite(redLight, HIGH);
    delay(100);
  }
}

void switchLED(bool status){
  digitalWrite(redLight, status);
  digitalWrite(greenLight, status);
  delay(100);
  statusLED = status;
}

bool compareLicensePlate(char** lists, String* msg){
  int i = 0;
  while(lists[i]) {
      if(strcmp(lists[i], msg->c_str()) == 0) {
          Serial.println("License plate match!");
          SerialBT.println("License plate match!");
          return true;
      }
      i++;
  }
  Serial.println("License plate doesn't match!");
  SerialBT.println("License plate doesn't match!");
  return false;
}

bool readLicensePlate(String* msg){
  if(SerialBT.available()){
    *msg = SerialBT.readString();
    return true;
  }
  else if(Serial.available()){
    *msg = Serial.readString();
    return true;
  }
  else return false;
}

void setup() {
  pinMode(gateButton, INPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(greenLight, OUTPUT);
  pinMode(redLight, OUTPUT);
  gate.attach(servoPin, 500, 2400);
  Serial.begin(9600);
  Serial.setTimeout(1);
  SerialBT.begin("IMK_SmartGate"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
//  while(!Serial.available()) {
//    Serial.println("waiting to start...");
//    delay(1000);
//  }
  closeGate();
}

void loop() {
  String msg = "X 0000 XX";
  switch (gateStatus)
  {
  case gate_waiting:
    if(readDistance() < 15){
      Serial.println("Car detected!");
      SerialBT.println("Car detected!");
      gateStatus = gate_closed;
    }
    break;

  case gate_closed:
    if(readDistance() > 15){
      Serial.println("Car detector error! Reinitializing...");
      SerialBT.println("Car detector error! Reinitializing...");
      digitalWrite(redLight, HIGH);
      digitalWrite(greenLight, LOW);
      gateStatus = gate_waiting;
      break;
    }
    switchLED(!statusLED);
    if(digitalRead(gateButton) == HIGH){
      Serial.println("Button pressed!");
      SerialBT.println("Button pressed!");
      gateStatus = gate_open;
    }
    else if(readLicensePlate(&msg)){
      if(compareLicensePlate(list_licensePlate, &msg)){
        gateStatus = gate_open;
      }
      else{
        blinkRedLight();
      }
    }
    if(gateStatus == gate_open){
      Serial.println("Opening Gate!");
      SerialBT.println("Opening Gate!");
      openGate();
    }
    break;

  case gate_open:
    if(readDistance() > 15){
      Serial.println("Car passed through! Closing the gate...");
      SerialBT.println("Car passed through! Closing the gate...");
      delay(2000);
      closeGate();
      gateStatus = gate_waiting;
    }
    else{
      Serial.println("Waiting for the car to pass through...");
      SerialBT.println("Waiting for the car to pass through...");
      blinkGreenLight();
    }
    break;
  
  default:
    gateStatus = gate_waiting;
    break;
  }
  delay(100);
}
