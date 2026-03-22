// -------------------------------------------------
// Dome Panels - Arduino Pro Mini
// Version 0.1
// Patrick Brown
// 03-22-2026
// -------------------------------------------------

//Variable Speed Servo Library with sequencing ability
//https://github.com/netlabtoolkit/VarSpeedServo
#include <VarSpeedServo.h> 

// i2c Library for communication with main Stealth Controller
#include <Wire.h> 

// These are easy names for our Arduino I/O pins and what they're used for 
// Note on the servo expander board, numbering starts at 1,2,3 etc. 
// but internally we add 1 to get the real Arduino number. 

#define PP1_SERVO 2   // Servo Pin 1 -  Internally this is Arduino Digitial #2 
#define PP2_SERVO 3   // Servo Pin 2
#define PP5_SERVO 4   // Servo Pin 3
#define PP6_SERVO 5   // Servo Pin 4
#define DP1_SERVO 6   // Servo Pin 5
#define DP2_SERVO 7   // Servo Pin 6
#define DP3_SERVO 8   // Servo Pin 7
#define DP4_SERVO 9   // Servo Pin 8
#define DP7_SERVO 10   // Servo Pin 9
#define DP10_SERVO 11   // Servo Pin 10

// Create an array of VarSpeedServo type, containing 5 elements/entries. 
// Note: Arrays are zero indexed. See http://arduino.cc/en/Reference/array

#define NBR_SERVOS 4  // Number of Servos
VarSpeedServo Servos[NBR_SERVOS]; // An Array of Servos, numbered 0 thru 4.

// More easy names  so we can reference the array element/row by name instead 
// trying to remember the number.
// Note: This is different to the pin number we attach each servo to.

#define PIE1 0
#define PIE2 1
#define PIE3 2
#define PIE4 3

#define FIRST_SERVO_PIN 2 //First Arduino Pin for Servos
#define NEUTRAL 90 //Start/Neutral/Center Position of our servos
#define PIE_ADD_DEGREES 22 //Pie Panel open degrees to add to NEUTRAL, this may need to be a negative number if you're servos are mounted differently
#define SPEED 150 // Used in some routines for the speed we move our servos, 1 is slow, 255 is full speed
#define OPENSPEED 100
#define CLOSESPEED 35
#define WAVESPEED 180
#define WAVEDELAY 100 // Delay between waving panels (milliseconds)

// Some variables to keep track off what's open
boolean piesOpen=false;

int i2cCommand = 0;
int myi2c = 10; // our i2c address

// New outgoing i2c Commands
String Packet;
int count = 0;
byte sum;
#define DESTI2C 0

//-----------------------------------------------------------
// Setup everything
void setup() {
  Serial.begin(57600); // Debug Console
  Serial.print("My i2c address: ");
  Serial.println(myi2c);
  Wire.begin(myi2c);   // Start I2C Communication Bus as a Slave
  Wire.onReceive(receivei2cEvent); // register i2c receive event which will be called when we execute a command

  // Turn off Status LED
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  
  i2cCommand = 0;

  // Activate/Center all the servos to NEUTRAL position
  Serial.print("Activating Servos"); 
  for( int i =0; i <  NBR_SERVOS; i++) {
    Serial.print(".");
    Servos[i].attach(FIRST_SERVO_PIN +i);
    Serial.print(".");
    Servos[i].write(NEUTRAL,120); // Return to NEUTRAL at a moderate speed
  }
   delay(1000); // wait for servos to get to neutral position

   //Detach from the Servos to save power and cut potential jitter/hum. 
  //Also avoids problems if you want to open them manually
  for( int i =0; i <  NBR_SERVOS; i++) {
    Serial.print(".");
    Servos[i].detach();
  }
  Serial.println("");  

  Serial.println("Waiting for i2c command");
}

//-----------------------------------------------------------
// Main Loop
void loop() {

  delay(50);
  loopTime = millis(); 
  
  // Check for new i2c command

  switch(i2cCommand) {
    case 1: // RESET
          Serial.println("Got reset message");
          digitalWrite(STATUS_LED, HIGH);
          i2cCommand=-1; 
          break;
          
    case 2:
          OpenClosePies();
          break;

    case 3:
          Wave1();
          break;

    case 4:
          Wave2();
          break;

    case 5:
          OpenOnePie();
          break;
          
    case 6:
          WaveFlap();
          break;

    default: // Catch All
    case 0: 
          digitalWrite(STATUS_LED, LOW);
          i2cCommand=-1;    
          break;
  }
 
}









