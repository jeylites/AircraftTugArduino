/*
 *
 *  Aircraft Tug
 *  Author: Anthony DiPilato
 *  anthonydipilato.com
 *
 *  This project is for the Aircraft Tug that I built.
 *  You can see the build log and details at [url]
 *
 *  The arduino acts as slave to the iOS app.
 *  Inputs:
 *  iOS commands via bluetooth HC-08
 *
 *  Outputs:
 *  4 relays
 *  PWM pins for dual channel h-bridge to control motors
 *
 *  Relays:
 *   Horn
 *   Flood lights
 *   Warning strobe
 *   Actuator for wheel hitch
 *   
 */

#include "configuration.h"


char inData[8];
int index = 0;

void setup()
{
  // Serial output for debugging and bluetooth module
  Serial.begin(9600);
  // Relays Pins
  pinMode(RELAY_HORN,   OUTPUT); 
  pinMode(RELAY_LIGHTS, OUTPUT); 
  pinMode(RELAY_STROBE, OUTPUT); 
  pinMode(RELAY_HITCH,  OUTPUT);
  // H-Bridge Pins
  pinMode(MOTOR_L_C1,   OUTPUT); 
  pinMode(MOTOR_L_C2,   OUTPUT); 
  pinMode(MOTOR_L_PWM,  OUTPUT); 
  pinMode(MOTOR_R_C1,   OUTPUT);
  pinMode(MOTOR_R_C2,   OUTPUT);
  pinMode(MOTOR_R_PWM,  OUTPUT);
  // Initialize relays (HIGH is off)
  digitalWrite(RELAY_HORN,    HIGH);
  digitalWrite(RELAY_LIGHTS,  HIGH);
  digitalWrite(RELAY_STROBE,  HIGH);
  digitalWrite(RELAY_HITCH,   HIGH);
  // Initialize H-Bridge
  // C pins control direction
  // C1,C2 = HIGH,HIGH stop LOW,HIGH forward HIGH,LOW reverse
  digitalWrite(MOTOR_L_C1,    HIGH);
  digitalWrite(MOTOR_L_C2,    HIGH);
  digitalWrite(MOTOR_R_C1,    HIGH);
  digitalWrite(MOTOR_R_C2,    HIGH);
  // speed is controlled by PWM (analog)
  analogWrite(MOTOR_L_PWM, 0);
  analogWrite(MOTOR_R_PWM, 0);
}

// Get relay from address
int getRelay(int address){
  int relay;
  switch (address) {
    case 1:
      relay = RELAY_HORN;
      break;
    case 2:
      relay = RELAY_LIGHTS;
      break;
    case 3:
      relay = RELAY_STROBE;
      break;
    case 4:
      relay = RELAY_HITCH;
    default:
      relay = -1;
      break;
  }
  return relay;
}

// get address from relay
int getAddress(int relay){
  int address;
  switch(relay) {
    case RELAY_HORN:
      address = 1;
      break;
    case RELAY_LIGHTS:
      address = 2;
      break;
    case RELAY_STROBE:
      address = 3;
      break;
    case RELAY_HITCH:
      address = 4;
      break;
    default:
      address = -1;
      break;
  }
  return address;
}

/*    Commands
 *    0 - Status
 *    1 - Relay On
 *    2 - Relay Off
 *    3 - Set PWM
 */

void readCommand(){
  // input format is comma seperated: command,address,value[newline]
  char* items = strtok(inData,","); // split by comma
  int command = atoi(items);  // get command and convert to integer
  items = strtok(NULL,",");   // call strok again for next token value
  int address = atoi(items);  // get value and convert to integer
  items = strtok(NULL,",");   // call strtok agin for next token value
  int value = atoi(items);    // value for command
  int relay;
  // redirect to command action
  switch(command){
    case(0): // Status
      {
        if(address == 0){
          // loop through all addresses to output all
          for(int i=1; i<=6; i++){
            getStatus(i);
          }
        }else{
          getStatus(address);
        }
      }
    case(1): // Relay On
      relay = getRelay(value);
      if(relay == -1) break;
      toggleRelay(relay, true);
      break;
    case(2): // Relay Off
      relay = getRelay(value);
      if(relay == -1) break;
      toggleRelay(relay, false);
      break;
    case(3): // Set PWM
      setMotorSpeed(value);
      break;
  }
  // if not status command return command for confirmation
  if(command != 0){
    // output command for confirmation
    Serial.print(command);
    Serial.print(",");
    Serial.print(address);
    Serial.print(",");
    Serial.println(value);
  }
}

/* Output Status
 *  Addresses
 *  0 - All
 *  1 - Horn
 *  2 - Lights
 *  3 - Warning Strobes
 *  4 - Wheel Hitch
 *  5 - Left Motor
 *  6 - Right Motor
 */
void getStatus(int item){
  // statuses are updated in loop for this function only outputs current
  // output format 0,address,value
  char output[16];
  int value;
  switch(item){
    case(1):
      value = (int) hornState;
      break;
    case(2):
      value = (int) lightState;
      break;
    case(3):
      value = (int) strobeState;
      break;
    case(4):
      value = (int) hitchState;
      break;
    case(5):
      value = (int) motorLState;
      break;
    case(6):
      value = (int) motorRState;
      break;
    default:
      value = -1;
      break;
  }
  sprintf(output,"0,%d,%d",item,value);
  Serial.println(output);
}


void checkSerial(){
    // Reading incoming commands
    while(Serial.available() > 0){
      // commands are comma seperated integers: command,address,value followed by newline (\n)
      char inChar = (char)Serial.read();
      if(inChar != '\n'){
        inData[index] = inChar;
        index++;
      }else{
        readCommand();
        memset(&inData,0,8); // clear command
        index = 0;
      }
    }

  
    boolean response = false;
    while(Serial.available() > 0){
       response = true;
       char input = Serial.read();
       Serial.print(input);
    }
    if(response)
      Serial.println("-");
    
}

void loop()
{

  checkSerial();
  
}


