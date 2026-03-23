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

unsigned long loopTime; // We keep track of the "time" in this variable.

// -------------------------------------------------
// Define some constants to help reference objects, 
// pins, servos, leds etc by name instead of numbers
// -------------------------------------------------

#define STATUS_LED 13 // Our Status LED on Arduino Digital IO #13, this is the built in LED on the Pro Mini
// These are easy names for our Arduino I/O pins and what they're used for 
// Note on the servo expander board, numbering starts at 1,2,3 etc. 
// but internally we add 1 to get the real Arduino number. 

#define PP1_SERVO 2   // Servo Pin 1 -  Internally this is Arduino Digitial #2 
#define PP2_SERVO 3   // Servo Pin 2
#define PP3_SERVO 4   // Servo Pin 3
#define PP4_SERVO 5   // Servo Pin 4
#define DP1_SERVO 6   // Servo Pin 5
#define DP2_SERVO 7   // Servo Pin 6
#define DP3_SERVO 8   // Servo Pin 7
#define DP4_SERVO 9   // Servo Pin 8
#define DP5_SERVO 10   // Servo Pin 9
#define DP6_SERVO 11   // Servo Pin 10

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


//----------------------------------------------------------------------------
// New i2c Commands
//----------------------------------------------------------------------------
void sendI2Ccmd(String cmd) {
  
  sum=0;
  Wire.beginTransmission(DESTI2C);
  for (int i=0;i<cmd.length();i++) {
//    Serial.print("TX=");
//    Serial.println(Packet[i]);
    Wire.write(cmd[i]);
    sum+=byte(cmd[i]);
  }
//  Serial.print("Checksum=");
//  Serial.println(sum);
  Wire.write(sum);
  Wire.endTransmission();  
  
}


//------------------------------------------------------
// i2c Command Event Handler
void receivei2cEvent(int howMany) {
  i2cCommand = Wire.read();    // receive i2c command
}


//---------------------------------------------------------
// Open/Close Pies
void OpenClosePies() {

    digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board     
    //Open or close  All Pie Panels
    
    Serial.print("Pie Panels: ");
    
    if (piesOpen) { // Close the Pie Panels
      Serial.println("Closing");
      piesOpen=false;
      // Attach to PIE1-4 so we can move them
      // We could do this in a loop with some math as well.
      Servos[PIE1].attach(PP1_SERVO);
      Servos[PIE2].attach(PP2_SERVO);
      Servos[PIE3].attach(PP3_SERVO);
      Servos[PIE4].attach(PP4_SERVO);

      // Close them in a non-sequencial order and not at the same time to make it more interesting
      // Basically 2, then 2
      Servos[PIE1].write(NEUTRAL,CLOSESPEED);
      Servos[PIE2].write(NEUTRAL,CLOSESPEED,true); // wait
      Servos[PIE3].write(NEUTRAL,CLOSESPEED);
      Servos[PIE4].write(NEUTRAL,CLOSESPEED,true); // wait

      // Detach from the Pies      
      Servos[PIE1].detach();
      Servos[PIE2].detach();
      Servos[PIE3].detach();
      Servos[PIE4].detach();

      Serial.println("Closed");
       
    } else { // Open Pie Panels
      Serial.println("Opening");
      piesOpen=true;

      // Attach to PIE1-4 so we can move them
      // This time we'll do it as a loop instead of calling out individual Pie panels
      // This only works because the numbers and pins are sequencial
      for (int i=PIE1; i<=PIE4;i++) {
        Serial.print("Open ");
        Serial.println(i+1); // PIE=0, so we add one for the print statement
        Servos[i].attach(PP1_SERVO+i);
        Servos[i].write(NEUTRAL+PIE_ADD_DEGREES,OPENSPEED);
      }
      Serial.println("Opened");
    }
    i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command
    digitalWrite(STATUS_LED, LOW);
}


//---------------------------------------------------------
// Wave Pies
void Wave1() {

    Serial.println("Wave Panels 1");
    digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board 
    // Wave 1
    for (int i=PIE1; i<=PIE4;i++) {
      Servos[i].attach(PP1_SERVO +i);
      Servos[i].write(NEUTRAL,150);
    }

    for (int i=0; i<4; i++) { 
      Serial.println("Wave");
      Servos[PIE1].write(NEUTRAL+PIE_ADD_DEGREES,WAVESPEED);
      delay(WAVEDELAY);
      Servos[PIE2].write(NEUTRAL+PIE_ADD_DEGREES,WAVESPEED);
      delay(WAVEDELAY);
      Servos[PIE3].write(NEUTRAL+PIE_ADD_DEGREES,WAVESPEED);
      delay(WAVEDELAY);
      Servos[PIE4].write(NEUTRAL+PIE_ADD_DEGREES,WAVESPEED);
      delay(WAVEDELAY *2.5);
      Servos[PIE4].write(NEUTRAL,WAVESPEED);
      delay(WAVEDELAY);
      Servos[PIE3].write(NEUTRAL,WAVESPEED);
      delay(WAVEDELAY);
      Servos[PIE2].write(NEUTRAL,WAVESPEED);
      delay(WAVEDELAY);
      Servos[PIE1].write(NEUTRAL,WAVESPEED);
      delay(WAVEDELAY * 2);
    }
    
    // Disconnect from our servos
    for (int i=PIE1; i<=PIE4;i++) 
        Servos[i].detach();

    Serial.println("Wave Done");

    i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command
    digitalWrite(STATUS_LED, LOW);
}


//-----------------------------------------------------------------------------------------------
// Scream / Wave Pies 2 - this is my short ciruit/panic routine
// This is a pretty complex routine. In summary we will do the following:
// Play Scream Sound, and in a loop flap servos, blink HP LEDs
void Wave2() {

}

//-----------------------------------------------------------------------------------------------
void OpenOnePie() {
    // to be implemented
    digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board 
    Serial.println("Open One Panel");
    i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command
    digitalWrite(STATUS_LED, LOW);
}


//-----------------------------------------------------------------------------------------------
// Wave Side Panel Flap
void WaveFlap() {
  digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board 
  Serial.println("Wave Flap");
  Servos[FLAP].attach(FLAP_SERVO_PIN);
  for (int i=0;i<4;i++) {
      Serial.print("Wave Servo ");
      Serial.println(FLAP-1);
      Servos[FLAP].write(140,60,true); // Move to max position and wait till it moved fully
      Servos[FLAP].write(NEUTRAL,60,true); // Move back to center/neutral and wait till it's moved fully
  }
  Servos[FLAP].write(NEUTRAL-3,255,true); // On my dome I have to make sure the panel is all the way close before detaching
  delay(100);
  Servos[FLAP].detach();
  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command
  digitalWrite(STATUS_LED, LOW);
  Serial.println("Wave Flap Done");
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









