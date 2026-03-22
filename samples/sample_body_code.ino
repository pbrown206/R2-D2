// -------------------------------------------------
// Servo Expander Body v 1.14
// Chris James
// 3-10-2017
// -------------------------------------------------

//Variable Speed Servo Library with sequencing ability
//https://github.com/netlabtoolkit/VarSpeedServo
#include <VarSpeedServo.h> 

// i2c Library for communication to talk with main Stealth Controller
#include <Wire.h> 
//#include <WSWire.h>

// -------------------------------------------------
// Define some constants to help reference objects, 
// pins, servos, leds etc by name instead of numbers
// -------------------------------------------------

#define STATUS_LED 13 // Our Status LED on Arduino Digital IO #13, this is the built in LED on the Pro Mini


// -------------------------------------------------
// New outgoing i2c Commands
// -------------------------------------------------
String Packet;
int count = 0;
byte sum;
#define DESTI2C 0

// These are easy names for our Arduino I/O pins and what they're used for 
// Note on the Servo expander board, numbering starts at 1,2,3 etc. 
// but internally we add 1 to get the real Arduino number. 
#define LEIA_DOOR_SERVO_PIN 2       // Leia Door Servo. Arduino Digitial IO #2, or 1st on the board
#define LEIA_ARM_SERVO_PIN 3        // Leia Antenna #3, 2nd on board
#define T_UTIL_ARM_SERVO_PIN 4      // Arduino Digitial IO #4, or 3rd on the board
#define B_UTIL_ARM_SERVO_PIN 5      // Arduino Digitial IO #5, or 4th on the board
#define LEFT_DOOR_SERVO_PIN 6
#define RIGHT_DOOR_SERVO_PIN 7 
#define SMALL_DOOR_SERVO_PIN 8

#define FIRE_EXT_PIN 11             // Relay for fire extinguisher. Arduino Digitial IO #11,
#define LEIA_SWITCH_PIN 12          // Switch for Leia Door (detect if it's open or closed), Arduino Digitial IO #12

// Create an array of VarSpeedServo type, containing 7 elements/entries. 
// Note: Arrays are zero indexed. See http://arduino.cc/en/Reference/array

#define NBR_SERVOS 7  // Number of Servos
VarSpeedServo Servos[NBR_SERVOS]; // An Array of Servos, numbered 0 thru 6.

// More easy names  so we can reference the array element/row by name instead 
// trying to remember the number.
// Note: This is different to the pin number we attach each servo to.
#define LEIA_DOOR 0
#define LEIA_ARM 1
#define T_UTIL_ARM 2 
#define B_UTIL_ARM 3
#define LEFT_DOOR 4
#define RIGHT_DOOR 5
#define SMALL_DOOR 6

#define FIRST_SERVO_PIN 2 //First Arduino Pin for Servos
#define NEUTRAL 90 //Start/Neutral/Center Position of our servos
#define UTILITYARMSSPEED 100 // servo speed for opening utility arms. 1=super slow, 255=fastest
#define UTILITYARMSSPEED2 45 // servo speed for opening utility arms. 1=super slow, 255=fastest
#define UTILITYARMSSPEED3 35 // servo speed for opening utility arms. 1=super slow, 255=fastest

#define DOOR_OPEN_SPEED 120 // servo speed for opening door
#define DOOR_LEIA_OPEN_SPEED 80
#define DOOR_CLOSE_SPEED 100 // servo speed for opening door

#define SCREAM_SPEED 250

//Tweaked Pulse width on arms to open them further. Usually 1000-2000

#define ARMMINPULSE 600
#define ARMMAXPULSE 2400

//Open position 

#define T_ARMOPEN 10 
#define B_ARMOPEN 0

//SOme door pulse widths
#define R_DOOR_MINPULSE 650
#define R_DOOR_MAXPULSE 2200 

#define L_DOOR_MINPULSE 650
#define L_DOOR_MAXPULSE 2100 

#define S_DOOR_MINPULSE 650
#define S_DOOR_MAXPULSE 2300 

//Door positions
#define L_DOOR_OPEN 170
#define R_DOOR_OPEN 10
#define S_DOOR_OPEN 10
#define R_DOOR_CLOSE 90
#define L_DOOR_CLOSE 90

#define LEIA_DOOR_OPEN 130

// Some variables to keep track of doors and arms etc.
boolean topUtilityArmOpen=false;
boolean bottomUtilityArmOpen=false;
int leiaDoorOpen; // value of our digitial input pin attached to a switch to detect if door is open or closed

boolean leftDoorOpen = false;
boolean rightDoorOpen = false;
boolean doorsOpen=false;

int i2cCommand = 0;
int myi2c = 9; // our i2c address

// ------------------------------------------
void setup() {
  Serial.begin(57600); // Debug Console
  Serial.println("Stealth RC - Servo Expander v1.14 (3-10-2017) - Body");

  Serial.print("My i2C Address: ");
  Serial.println(myi2c);
  
  Wire.begin(myi2c);  // Start I2C communication Bus as a Slave (Device Number 9)
  Wire.onReceive(receivei2cEvent); // routine to call/run when we receive an i2c command
  pinMode(STATUS_LED, OUTPUT); // turn status led off
  digitalWrite(STATUS_LED, LOW);
  
  pinMode(FIRE_EXT_PIN, OUTPUT); // turn fire ext off
  digitalWrite(FIRE_EXT_PIN, LOW); 
  
  i2cCommand = 0;
  // Activate/Center all the servos to NEUTRAL position
  Serial.print("Activating Servos"); 
  resetServos();

  Serial.println("");  
  
  pinMode(LEIA_SWITCH_PIN, INPUT); // Set switch I/O pin to INPUT
  digitalWrite(LEIA_SWITCH_PIN, HIGH); // Enable Pullup resistor
  leiaDoorOpen = digitalRead(LEIA_SWITCH_PIN); // Read the switch
  
  if (leiaDoorOpen==1) {
    Serial.println("Leia Door Open. Oops!");
  } else {
    Serial.println("Leia Closed.");    
  }
  

  
}

//-----------------------------------------------------
// i2c Command Event Handler
//-----------------------------------------------------

void receivei2cEvent(int howMany) {
  i2cCommand = Wire.read();    // receive byte as an integer\
  Serial.print("Command Code:");
  Serial.println(i2cCommand); 
}

//----------------------------------------------------------------------------
// New i2c Commands
//----------------------------------------------------------------------------
void sendI2Ccmd(String cmd) {
  
  sum=0;
  Wire.beginTransmission(DESTI2C);
  for (int i=0;i<cmd.length();i++) {
    Wire.write(cmd[i]);
    sum+=byte(cmd[i]);
  }
  Wire.write(sum);
  Wire.endTransmission();  
  
}

//-----------------------------------------------------
// Fire Extinguisher
//-----------------------------------------------------

void FireExt() {
 
  digitalWrite(STATUS_LED, HIGH);
  Serial.println("Fire Extinguisher");
  
  // Toggle our digitial output 3 to fire ext in small bursts
  for (int i=0;i<3;i++) {
    digitalWrite(FIRE_EXT_PIN, HIGH);
    delay(100);
    digitalWrite(FIRE_EXT_PIN, LOW);
    delay(500);
  }
  digitalWrite(STATUS_LED, LOW);
  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
  
}

//-----------------------------------------------------
// Reset/Close Utility Arms
//-----------------------------------------------------

void resetServos() {
    
  Servos[T_UTIL_ARM].attach(T_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);
  Servos[B_UTIL_ARM].attach(B_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);

  Servos[T_UTIL_ARM].write(NEUTRAL+5,UTILITYARMSSPEED3); 
  Servos[B_UTIL_ARM].write(NEUTRAL+5,UTILITYARMSSPEED3); 

  Servos[LEFT_DOOR].attach(LEFT_DOOR_SERVO_PIN,L_DOOR_MINPULSE,L_DOOR_MAXPULSE);  
  Servos[LEFT_DOOR].write(L_DOOR_CLOSE-3,DOOR_CLOSE_SPEED); 

  Servos[RIGHT_DOOR].attach(RIGHT_DOOR_SERVO_PIN,R_DOOR_MINPULSE,R_DOOR_MAXPULSE);  
  Servos[RIGHT_DOOR].write(R_DOOR_CLOSE,DOOR_CLOSE_SPEED); 

  Servos[SMALL_DOOR].attach(SMALL_DOOR_SERVO_PIN,S_DOOR_MINPULSE,S_DOOR_MAXPULSE);  
  Servos[SMALL_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 

  Servos[LEIA_DOOR].attach(LEIA_DOOR_SERVO_PIN);  
  Servos[LEIA_DOOR].write(NEUTRAL-3,DOOR_CLOSE_SPEED);

  delay(1000); // wait on servos
 
  // Detach from the servo to save power and reduce jitter
  Servos[T_UTIL_ARM].detach();
  Servos[B_UTIL_ARM].detach();
  Servos[LEFT_DOOR].detach();
  Servos[RIGHT_DOOR].detach();
  Servos[SMALL_DOOR].detach();
  Servos[LEIA_DOOR].detach();

  doorsOpen=false; 
  topUtilityArmOpen=false;
  bottomUtilityArmOpen=false;

}

//------------------------------------------------------------------
// Open/Close both Utility Arms
void UtilityArms() {

  digitalWrite(STATUS_LED, HIGH);

  // If the Arms are open the close them
  if (topUtilityArmOpen) { 
    Serial.println("Close utility arms");
    topUtilityArmOpen=false; 
    Servos[T_UTIL_ARM].attach(T_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);
    Servos[B_UTIL_ARM].attach(B_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);
    
    // pull arms slightly beyond neutral to make sure they're really closed. Will vary on your droid
    Servos[T_UTIL_ARM].write(NEUTRAL+5,UTILITYARMSSPEED2); 
    Servos[B_UTIL_ARM].write(NEUTRAL+5,UTILITYARMSSPEED2); // wait on arm to reach position
    delay(1000);

    Servos[T_UTIL_ARM].detach(); // detach so we can move the arms freely
    Servos[B_UTIL_ARM].detach();
  // Open the arms if closed
  } else {
    Serial.println("Open utility arms");
    topUtilityArmOpen=true; 
    Servos[T_UTIL_ARM].attach(T_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);
    Servos[B_UTIL_ARM].attach(B_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);

    // Set Servo position to Open. In our case NEUTRAL is 90, and open is close to zero, 
    // but on one of my arms it needs to be just shy of that so it's set to 25
    // We're also moving them at a moderate speed   
    Servos[T_UTIL_ARM].write(T_ARMOPEN,UTILITYARMSSPEED2); // open at moderate speed
    Servos[B_UTIL_ARM].write(B_ARMOPEN,UTILITYARMSSPEED2);
  } 

  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
  digitalWrite(STATUS_LED, LOW);
}

//-----------------------------------------------------
// Holographic Leia Sequence
//-----------------------------------------------------

void LeiaSequence() { 

  digitalWrite(STATUS_LED, HIGH);
  Serial.println("Leia Sequence");
  
  //Open
  Serial.println("Open Door");
  Servos[LEIA_DOOR].attach(LEIA_DOOR_SERVO_PIN);    
  // open door to position LEIA_DOOR_OPEN, moderate speed and wait till it's there
  // remember 90 is our NEUTRAL position
  Servos[LEIA_DOOR].write(LEIA_DOOR_OPEN,DOOR_LEIA_OPEN_SPEED);   
  sendI2Ccmd("autod=2");
  delay(500);
  
  //Send out leia via antenna/motor attached to speed controller
  // but only if our door is open.
  leiaDoorOpen = digitalRead(LEIA_SWITCH_PIN);
  if (leiaDoorOpen==1) {
    Serial.println("Send out Leia");
    Servos[LEIA_ARM].attach(LEIA_ARM_SERVO_PIN);    
    Servos[LEIA_ARM].write(120); // slowish at first
    delay(500); 
    Servos[LEIA_ARM].write(140); // then speed up a bit

    delay(2000); // wait for the antenna to extend
    
    // because it's a speed controller/motor and not a servo 
    // we need to stop the antenna by returning to neutral position
    Servos[LEIA_ARM].write(91); 
    
  } else  {
    // door was closed so we didnt extend the antenna
    Serial.println("door still closed."); 
  }
  
  // Send an i2c command back to the Stealth Controller to play the Leia message
  // Also set max volume temporarily and delay random sounds for 60 seconds.
  Serial.println("Play Leia sound by sending i2c command");
  sendI2Ccmd("tmpvol=100,08");
  delay(150);
  sendI2Ccmd("tmprnd=60");
  delay(850);
  sendI2Ccmd("$07");
  
  Serial.println("Delay");
  delay(500);
  
  // Leia should now be extened all the way and sound playing, so we could sit idle
  // but instead we loop around flickering the HP LED in the dome via another i2c command.
  // The while loop is time to last as long as our Leia sound
  unsigned long tmp=millis(); // timer
  
  Serial.println("Flicker HP");
  while (millis()<tmp+5000) {
     Wire.beginTransmission(10); // transmit to device #10
     Wire.write(4);
     Wire.endTransmission();    // stop transmitting
     int rand=30+(10*random(1,6));
     delay(rand);
     Wire.beginTransmission(10); // transmit to device #10
     Wire.write(4);
     Wire.endTransmission();    // stop transmitting
     rand=rand=50+(10*random(1,6));
     delay(rand);
  }

   // DO HP LED command one more time to return to original state.
    Wire.beginTransmission(10); // transmit to device #10
    Wire.write(4);
    Wire.endTransmission();    // stop transmitting
  
  // Okay sound has finished 
  
  //Retract leia
  if (leiaDoorOpen==1) {
    Serial.println("Bring in Leia");
    Servos[LEIA_ARM].write(60); // Fast at first
   delay(500); 
    Servos[LEIA_ARM].write(30); // Slow down toward end
    delay(2500);
    Servos[LEIA_ARM].write(NEUTRAL); // stop antenna motor
  }    
  
  // Close Door
  Serial.println("Close Door");
  // Close door just beyond neutral to make sure it's closed. 
  // Note the "true", this tells us to wait till the servo has done moving
  Servos[LEIA_DOOR].write(NEUTRAL-3,DOOR_CLOSE_SPEED,true); 
  
  // Detach from servos/speed controller to save power and allow the door to open freely if needed  
  Servos[LEIA_DOOR].detach();
  Servos[LEIA_ARM].detach();
  sendI2Ccmd("autod=1");


  digitalWrite(STATUS_LED, LOW); // signal end of routine
  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
}

//---------------------------------------------
// S C R E A M
//---------------------------------------------
void Scream() {
  
  digitalWrite(STATUS_LED, HIGH);
  
  Servos[T_UTIL_ARM].attach(T_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);
  Servos[B_UTIL_ARM].attach(B_UTIL_ARM_SERVO_PIN,ARMMINPULSE,ARMMAXPULSE);
  Servos[LEFT_DOOR].attach(LEFT_DOOR_SERVO_PIN,L_DOOR_MINPULSE,L_DOOR_MAXPULSE);  
  Servos[RIGHT_DOOR].attach(RIGHT_DOOR_SERVO_PIN,R_DOOR_MINPULSE,R_DOOR_MAXPULSE);  
  Servos[SMALL_DOOR].attach(SMALL_DOOR_SERVO_PIN,S_DOOR_MINPULSE,S_DOOR_MAXPULSE);  
  Servos[LEIA_DOOR].attach(LEIA_DOOR_SERVO_PIN);  

  for (int i=0;i<3;i++) {
    
    Serial.print("Loop:");
    Serial.println(i+1);
    Servos[SMALL_DOOR].write(S_DOOR_OPEN,SCREAM_SPEED); 
    Servos[LEFT_DOOR].write(L_DOOR_OPEN,SCREAM_SPEED); 

    Servos[RIGHT_DOOR].write(NEUTRAL,SCREAM_SPEED); 
    Servos[LEIA_DOOR].write(NEUTRAL,SCREAM_SPEED); 

    Servos[T_UTIL_ARM].write(60,255); // open at moderate speed
    Servos[B_UTIL_ARM].write(NEUTRAL-2,255,true); // 0=open all the way
 
    delay(150);

    Servos[SMALL_DOOR].write(NEUTRAL,SCREAM_SPEED); 
    Servos[LEFT_DOOR].write(NEUTRAL,SCREAM_SPEED); 
    Servos[RIGHT_DOOR].write(R_DOOR_OPEN,SCREAM_SPEED); 
    Servos[LEIA_DOOR].write(LEIA_DOOR_OPEN,SCREAM_SPEED); 
  
    Servos[T_UTIL_ARM].write(NEUTRAL-2,255); // open at moderate speed
    Servos[B_UTIL_ARM].write(15,255,true);  // 0=open all the way
   
    delay(150);
    
  }

  Serial.println("Close everything");
  
  Servos[T_UTIL_ARM].write(NEUTRAL+5,UTILITYARMSSPEED2); 
  Servos[B_UTIL_ARM].write(NEUTRAL+5,UTILITYARMSSPEED2); // wait on arm to reach position
    
  Servos[LEFT_DOOR].write(L_DOOR_CLOSE,DOOR_CLOSE_SPEED); 
  Servos[RIGHT_DOOR].write(R_DOOR_CLOSE,DOOR_CLOSE_SPEED); 
  Servos[SMALL_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 
  Servos[LEIA_DOOR].write(NEUTRAL-3,DOOR_CLOSE_SPEED);

  delay(1000); // wait on arm to reach position

  Serial.println("Detach");
 
  Servos[LEFT_DOOR].detach();
  Servos[RIGHT_DOOR].detach();
  Servos[SMALL_DOOR].detach();
  Servos[LEIA_DOOR].detach();  
  Servos[T_UTIL_ARM].detach(); 
  Servos[B_UTIL_ARM].detach();  

  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
  digitalWrite(STATUS_LED, LOW);    
    
  
}

//-----------------------------------------------------
// D O O R S
//-----------------------------------------------------

void Doors() {

  digitalWrite(STATUS_LED, HIGH);

  // If the Arms are open the close them
  if (doorsOpen) { 
    Serial.println("Close doors");
    doorsOpen=false; 

    Servos[LEFT_DOOR].attach(LEFT_DOOR_SERVO_PIN,L_DOOR_MINPULSE,L_DOOR_MAXPULSE);  
    Servos[RIGHT_DOOR].attach(RIGHT_DOOR_SERVO_PIN,R_DOOR_MINPULSE,R_DOOR_MAXPULSE);  
    Servos[SMALL_DOOR].attach(SMALL_DOOR_SERVO_PIN,S_DOOR_MINPULSE,S_DOOR_MAXPULSE);  
    Servos[LEIA_DOOR].attach(LEIA_DOOR_SERVO_PIN);  

    Servos[LEFT_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 
    Servos[RIGHT_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 
    Servos[SMALL_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 
    Servos[LEIA_DOOR].write(NEUTRAL-3,DOOR_CLOSE_SPEED);
  
    delay(1000); // wait on arm to reach position
   
    Servos[LEFT_DOOR].detach();
    Servos[RIGHT_DOOR].detach();
    Servos[SMALL_DOOR].detach();
    Servos[LEIA_DOOR].detach();

  } else {
    Serial.println("Open doors");
    doorsOpen=true; 

    Servos[LEFT_DOOR].attach(LEFT_DOOR_SERVO_PIN,L_DOOR_MINPULSE,L_DOOR_MAXPULSE);  
    Servos[RIGHT_DOOR].attach(RIGHT_DOOR_SERVO_PIN,R_DOOR_MINPULSE,R_DOOR_MAXPULSE);  
    Servos[SMALL_DOOR].attach(SMALL_DOOR_SERVO_PIN,S_DOOR_MINPULSE,S_DOOR_MAXPULSE);  
    Servos[LEIA_DOOR].attach(LEIA_DOOR_SERVO_PIN);  

    Servos[SMALL_DOOR].write(S_DOOR_OPEN,DOOR_OPEN_SPEED); 
    Servos[LEFT_DOOR].write(L_DOOR_OPEN,DOOR_OPEN_SPEED); 
    Servos[RIGHT_DOOR].write(R_DOOR_OPEN,DOOR_OPEN_SPEED); 
    Servos[LEIA_DOOR].write(LEIA_DOOR_OPEN,DOOR_LEIA_OPEN_SPEED); 
  
    delay(1000);

    Servos[T_UTIL_ARM].detach(); 
    Servos[B_UTIL_ARM].detach();    
    Servos[LEFT_DOOR].detach();
    Servos[RIGHT_DOOR].detach();
    Servos[SMALL_DOOR].detach();
    Servos[LEIA_DOOR].detach();

  } 

  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
  digitalWrite(STATUS_LED, LOW);

  
}

void openLeftDoor() {
  
  digitalWrite(STATUS_LED, HIGH);
  
  if (leftDoorOpen) {
    Serial.println("Close Left Door");
    leftDoorOpen=false;
    Servos[LEFT_DOOR].attach(LEFT_DOOR_SERVO_PIN,L_DOOR_MINPULSE,L_DOOR_MAXPULSE);  
    Servos[LEFT_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 
    delay(1000); // wait on arm to reach position
    Servos[LEFT_DOOR].detach();
     
  } else {
    leftDoorOpen=true;
    Serial.println("Open Left Door");
    Servos[LEFT_DOOR].attach(LEFT_DOOR_SERVO_PIN,L_DOOR_MINPULSE,L_DOOR_MAXPULSE);  
    Servos[LEFT_DOOR].write(L_DOOR_OPEN,DOOR_OPEN_SPEED); 
    delay(1000);
    Servos[LEFT_DOOR].detach();
  } 
  
  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
  digitalWrite(STATUS_LED, LOW);
 
}

void openRightDoor() {
  
  digitalWrite(STATUS_LED, HIGH);
  
  if (rightDoorOpen) {
    Serial.println("Close Right Door");
    rightDoorOpen=false;
    Servos[RIGHT_DOOR].attach(RIGHT_DOOR_SERVO_PIN,R_DOOR_MINPULSE,R_DOOR_MAXPULSE);  
    Servos[RIGHT_DOOR].write(NEUTRAL,DOOR_CLOSE_SPEED); 
    delay(1000); // wait on arm to reach position
    Servos[RIGHT_DOOR].detach();
     
  } else {
    rightDoorOpen=true;
    Serial.println("Open Right Door");
    Servos[RIGHT_DOOR].attach(RIGHT_DOOR_SERVO_PIN,R_DOOR_MINPULSE,R_DOOR_MAXPULSE);  
    Servos[RIGHT_DOOR].write(R_DOOR_OPEN,DOOR_OPEN_SPEED); 
    delay(1000);
    Servos[RIGHT_DOOR].detach();
  } 
  
  i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command 
  digitalWrite(STATUS_LED, LOW);
 
}


//-----------------------------------------------------
// Main Loop
//-----------------------------------------------------

void loop() {

  delay(50);

  // Check for new i2c command
  switch(i2cCommand) {
    case 1: // RESET
          Serial.println("Got reset message");
          resetServos();   // Close utility arms
          digitalWrite(STATUS_LED, HIGH);
          i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command  
          break;
          
    case 2:
          LeiaSequence();
          break;

    case 3:
          UtilityArms();
          break;
  
    case 4:
          Doors();
          break;  
          
    case 5:
          Scream();
          break;    

    case 6:
          openLeftDoor();
          break; 

    case 7:
          openRightDoor();
          break; 
                    
    case 9:
          FireExt();
          break;
          
    default: // Catch All
    case 0: 
          digitalWrite(STATUS_LED, LOW);
          i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command     
          break;
  }
 
}
