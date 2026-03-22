// -------------------------------------------------
// Dome Expander v 1.2
// Chris James
// 6-15-2016
// -------------------------------------------------

//Variable Speed Servo Library with sequencing ability
//https://github.com/netlabtoolkit/VarSpeedServo
#include <VarSpeedServo.h> 

// i2c Library for communication to talk with main CJ Stealth Controller
#include <Wire.h> 

unsigned long loopTime; // We keep track of the "time" in this variable.

// -------------------------------------------------
// Define some constants to help reference objects, 
// pins, servos, leds etc by name instead of numbers
// -------------------------------------------------

#define STATUS_LED 13 // Our Status LED on Arduino Digital IO #13, this is the built in LED on the Pro Mini

// These are easy names for our Arduino I/O pins and what they're used for 
// Note on the Servo expander board, numbering starts at 1,2,3 etc. 
// but internally we add 1 to get the real Arduino number. 

#define PIE1_SERVO_PIN 2   // Servo 1 -  Internally this is Arduino Digitial #2 
#define PIE2_SERVO_PIN 3   // Servo 2 etc etc.
#define PIE3_SERVO_PIN 4   // 3
#define PIE4_SERVO_PIN 5   // 4
#define FLAP_SERVO_PIN 6   // 5
#define HP_SERVO_PIN 7     //6
#define HP_LED_PIN 12       // Our HP LED is attached to Digitial #12 (or 11 as marked on the board)

// Create an array of VarSpeedServo type, containing 5 elements/entries. 
// Note: Arrays are zero indexed. See http://arduino.cc/en/Reference/array

#define NBR_SERVOS 6  // Number of Servos
VarSpeedServo Servos[NBR_SERVOS]; // An Array of Servos, numbered 0 thru 6.

// More easy names  so we can reference the array element/row by name instead 
// trying to remember the number.
// Note: This is different to the pin number we attach each servo to.

#define PIE1 0
#define PIE2 1
#define PIE3 2
#define PIE4 3
#define FLAP 4
#define HP 5

#define FIRST_SERVO_PIN 2 //First Arduino Pin for Servos
#define NEUTRAL 90 //Start/Neutral/Center Position of our servos
#define PIE_ADD_DEGREES 22 //Pie Panel open degrees to add to NEUTRAL, this may need to be a negative number if you're servos are mounted differently
#define SPEED 150 // Used in some routines for the speed we move our servos, 1 is slow, 255 is full speed
#define OPENSPEED 100
#define CLOSESPEED 35
#define WAVESPEED 180
#define WAVEDELAY 100 // Delay between waving panels (milliseconds)

// Random Holo Projector Tunables
#define HPminDelay 1 // min delay in seconds between moving servo
#define HPmaxDelay 5 // max delay
#define MINHP 60 // Min value for HP servo movement
#define MAXHP 145 // Max value for HP servo movement
unsigned long HPrandDelay = 0;
unsigned long HPrandomTime = 0;
boolean HPrandomOn = true;

// Some variables to keep track off what's open or turned on
boolean hpLEDon=true;
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
  Serial.println("CJ Stealth Server Expander 1.1 - 4.16.14 - Dome");
  
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

  if (HPrandomOn) 
    Servos[HP-FIRST_SERVO_PIN].attach(HP); // attached to our HP Servo if it's active on startup

  // turn on HP LED 
  // I control the HP light via a relay - I have it set to Normally Open=LED ON, to save power. so LOW=ON, HIGH=OFF on the pin
  pinMode(HP_LED_PIN, OUTPUT);
  digitalWrite(HP_LED_PIN, LOW);
  
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

//-----------------------------------------------------------
// Random HP
void doRandomHP() {
  if (HPrandomOn && loopTime>HPrandomTime) { // Only move if ON and our HP Random Time elapsed 
      HPrandDelay = random(HPminDelay,HPmaxDelay);
      HPrandomTime=millis()+(HPrandDelay*1000); // next time to move in the future
      int rand=random(MINHP,MAXHP); // Range for HP Movement
      Servos[HP].write(rand,100);
  }
}

//------------------------------------
// Turn HP LED on or off
void HPLED() {
 
    digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board 
    Serial.println("HP LED");
    if (hpLEDon) {
       hpLEDon=false;
       digitalWrite(HP_LED_PIN, HIGH); // see note above about using a relay
    } else {
       hpLEDon=true;
       digitalWrite(HP_LED_PIN, LOW);  // see note above about using a relay
    }
    i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command
    digitalWrite(STATUS_LED, LOW);
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
      Servos[PIE1].attach(PIE1_SERVO_PIN);
      Servos[PIE2].attach(PIE2_SERVO_PIN);
      Servos[PIE3].attach(PIE3_SERVO_PIN);
      Servos[PIE4].attach(PIE4_SERVO_PIN);

      // Close them in a non-sequencial order and not at the same time to make it more interesting
      // Basically 2, then 2
      Servos[PIE1].write(NEUTRAL,CLOSESPEED);
      Servos[PIE3].write(NEUTRAL,CLOSESPEED,true); // wait
      Servos[PIE4].write(NEUTRAL,CLOSESPEED);
      Servos[PIE2].write(NEUTRAL,CLOSESPEED,true); // wait

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
        Servos[i].attach(PIE1_SERVO_PIN+i);
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
      Servos[i].attach(PIE1_SERVO_PIN +i);
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
 
    digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board 
    //Wave 2 / Scream
    
    Serial.println("Scream Sequence");

    sendI2Ccmd("tmpvol=100,08"); // Temp max volume for 8 seconds
    delay(150);
    sendI2Ccmd("tmprnd=60"); // delay random sounds by 60 seconds so we dont stomp on sequence
    delay(1000);
    sendI2Ccmd("$0403"); // Play Scream, SOUNDBANK 4, SOUND 3
    

    // Routine below should last as long as the sound we just started playing
    
    //Attach to all the pie panels
    Servos[PIE1].attach(PIE1_SERVO_PIN);
    Servos[PIE2].attach(PIE2_SERVO_PIN);
    Servos[PIE3].attach(PIE3_SERVO_PIN);
    Servos[PIE4].attach(PIE4_SERVO_PIN);
    Servos[FLAP].attach(FLAP_SERVO_PIN);
    delay(250);
    
    // Loop 7 times in panic mode
    for (int x=0; x<7;x++) {

      digitalWrite(HP_LED_PIN, LOW); // We're going to also blink the HP LED

      // Open some the Servos, they could in theory do this at different speed to be more organic
      Servos[PIE2].write(NEUTRAL+2,SPEED); 
      Servos[PIE4].write(NEUTRAL+2,SPEED);
      Servos[PIE1].write(NEUTRAL+PIE_ADD_DEGREES,SPEED);
      Servos[FLAP].write(140,SPEED);
      Servos[PIE3].write(NEUTRAL+50,SPEED,true); // wait on servos

      // Now close/open the opposite Pie panels so they're waving at different intervals
      digitalWrite(HP_LED_PIN, HIGH); 
      Servos[PIE2].write(NEUTRAL+PIE_ADD_DEGREES,SPEED);
      Servos[PIE4].write(NEUTRAL+PIE_ADD_DEGREES,SPEED);
      Servos[PIE1].write(NEUTRAL+2,SPEED);
      Servos[FLAP].write(NEUTRAL,SPEED);
      Servos[PIE3].write(NEUTRAL+2,SPEED,true);
    }

    // end of panic sequence reset thing back to normal

    //reset HP LED to original value
    if (hpLEDon) {
       digitalWrite(HP_LED_PIN, LOW);
    } else {
       digitalWrite(HP_LED_PIN, HIGH);
    }

    // Close all the pies and flaps
    Servos[PIE1].write(NEUTRAL,SPEED);
    Servos[PIE2].write(NEUTRAL,SPEED,true);
    Servos[PIE3].write(NEUTRAL,SPEED);
    Servos[PIE4].write(NEUTRAL,SPEED,true);
    Servos[FLAP].write(NEUTRAL-3,255,true);
    
    // wait for everything to settle
    delay(500);
    
    // Detach all servos so we can go back to opening them manually
    Servos[PIE1].detach();
    Servos[PIE2].detach();
    Servos[PIE3].detach();
    Servos[PIE4].detach();
    Servos[FLAP].detach();

    i2cCommand=-1; // always reset i2cCommand to -1, so we don't repeatedly do the same command
    digitalWrite(STATUS_LED, LOW);

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
// Turn Random HP Movement on/off
void RandomOnOff() {
  digitalWrite(STATUS_LED, HIGH); // turn on STATUS LED so we can visually see we got the command on the board 
  if (HPrandomOn) {
    HPrandomOn=false;
    Servos[HP-FIRST_SERVO_PIN].detach();

    Serial.println("Random HP Off"); 
  } else {
    HPrandomOn=true;
    HPrandomTime = 0;
    HPrandDelay = 0;
    Servos[HP-FIRST_SERVO_PIN].attach(HP);
    Serial.println("Random HP On"); 
  }
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
  if (HPrandomOn) doRandomHP(); 
  
  // Check for new i2c command

  switch(i2cCommand) {
    case 1: // RESET
          Serial.println("Got reset message");
          digitalWrite(STATUS_LED, HIGH);
          i2cCommand=-1; 
          break;
          
    case 4:
          HPLED();
          break;
          
    case 5:
          OpenClosePies();
          break;

    case 6:
          Wave1();
          break;

    case 7:
          Wave2();
          break;

    case 8:
          OpenOnePie();
          break;
          
    case 9:
          RandomOnOff();
          break;
          
    case 10:
          WaveFlap();
          break;

    default: // Catch All
    case 0: 
          digitalWrite(STATUS_LED, LOW);
          i2cCommand=-1;    
          break;
  }
 
}
