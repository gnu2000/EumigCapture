// Include the AccelStepper Library
#include <AccelStepper.h>

// Set up functions to call
void frameAdvance();
void trigger();
void jogging();
void capture();
void recvWithStartEndMarkers();
void parseData();
void trigPort();

// Define pin connections
const int trigPin = 2;
const int dirPin = 5;
const int stepPin = 3;
const int enablePin = 4;
const int relayPin = 6;
const int sensorPin = 7;

const int lightPin = 11;
const int jogPin = 9;
const int framePin = 10;

//set variables for commands
int light = 1;
int jog = 1;
int frame = 1;

// set variables for the optical sensor and count
int state = 0;
int oldstate = LOW;
int count = 0;
int prevcount = 0;

// variables for serial stuff
String command;
String buffer;
int framecount = 0;
int capturecount = 0;

// some variables for non blocking timing
int period = 200;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// potentiometer pin connection
int potPin = A0;
int potVal; // variable to store potentiometer value

// # steps for full 360-degree rotation
// change to match your motor specs
int stepsPerRevolution = 200;
float gearRatio = 2.7095;

// set the multiplier to use for microstepping
int microStepping = 2;

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

// Parsing code from example

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
int integerFromPC = 0;
float floatFromPC = 0.0;

boolean newData = false;

void setup() {
	// open the serial port at 115200 bps:
      Serial.begin(115200);
  
  // set the maximum speed, acceleration factor,
	// initial speed and the target position

  Serial.println("Setting up motor instance...");

	myStepper.setMaxSpeed(2000);
	myStepper.setAcceleration(300);
	myStepper.setSpeed(200);
	myStepper.moveTo((stepsPerRevolution * microStepping) * gearRatio);
  
  Serial.println("Setting pin modes...");

  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
 
  pinMode(lightPin, INPUT_PULLUP);
  pinMode(jogPin, INPUT_PULLUP);
  pinMode(framePin, INPUT_PULLUP);
  pinMode(sensorPin, INPUT);
}

void loop() {
  //Set motor direction (if it works)
  digitalWrite(dirPin, LOW);

  // Read the pins (the pins!)
    light = digitalRead(lightPin);
    jog = digitalRead(jogPin);
    frame = digitalRead(framePin);

  // Work out if the LED should be on or not
    if(light == LOW){
      digitalWrite(relayPin, HIGH);  
    }else if(light == HIGH){
      digitalWrite(relayPin, LOW);
    }

  // Add delay for debugging
  // delay(50);

  // Store current time
  currentMillis = millis();

  recvWithStartEndMarkers();
  if (newData == true) {
      strcpy(tempChars, receivedChars);
          // this temporary copy is necessary to protect the original data
          //   because strtok() used in parseData() replaces the commas with \0
      Serial.println(receivedChars);
      parseData();
      newData = false;
  }

  // Check for the capture command and store it
  //if (buffer == "capture"){
  //  command = "capture";
  //}

  trigPort();

  // Time for MOVEMENT!
    if(jog == LOW){
      jogging();
    }else if(strcmp(messageFromPC, "capture") == 0){
      capture();
      //command = "";
    }else if(frame == LOW){
      frameAdvance();
    }else{
      //Serial.println("No buttons pushed.");
    }

  // Reset position
    myStepper.setCurrentPosition(0);
}

void frameAdvance() {
  myStepper.setSpeed(1000);
  Serial.println("Running to position");
  myStepper.runToNewPosition((stepsPerRevolution * microStepping) * gearRatio);
  Serial.println("Frame advance done"); 
}

void trigger() {
  state = digitalRead(sensorPin);
  if (state == HIGH && state != oldstate){
//    count += 1;
    Serial.println("trig");
    capturecount += 1;
  } else if (state == HIGH && state != oldstate) {
    Serial.println("Running...");
  }
  

//  if (count == 1){
//    Serial.println("trig");
//    count = 0;
//    capturecount += 1;
//  } else {
//    if (count != prevcount) {
//      Serial.println(count);
//    }
//  }
  oldstate = state;
//  prevcount = count;
}

void jogging() {
  while (jog == LOW){
    myStepper.setSpeed(1650);
    myStepper.runSpeed();
    jog = digitalRead(jogPin);
    trigPort();
  }
}

void trigPort() {
  state = digitalRead(sensorPin);
  if (state == HIGH){
    digitalWrite(trigPin, HIGH);
  } else if (state == LOW){
    digitalWrite(trigPin, LOW);
  }
}

void capture() {
  while (capturecount <= framecount){
    myStepper.setSpeed(1000);
    myStepper.runSpeed();
    trigger();
    jog = digitalRead(jogPin);
    if(jog == LOW) {
      break;
    }
  }
  Serial.println("Done!");
  strcpy(messageFromPC, "stoppit");
}

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
      rc = Serial.read();

      if (recvInProgress == true) {
          if (rc != endMarker) {
              receivedChars[ndx] = rc;
              ndx++;
              if (ndx >= numChars) {
                  ndx = numChars - 1;
              }
          }
          else {
              receivedChars[ndx] = '\0'; // terminate the string
              recvInProgress = false;
              ndx = 0;
              newData = true;
          }
      }

      else if (rc == startMarker) {
          recvInProgress = true;
      }
  }
}

//============

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars,","); // get the first part - the string
  strcpy(messageFromPC, strtokIndx);  // copy it to messageFromPC

  strtokIndx = strtok(NULL, ",");     // this continues where the previous call left off
  framecount = atoi(strtokIndx);      // convert this part to an integer

  Serial.println(messageFromPC);
  Serial.println(framecount);
}