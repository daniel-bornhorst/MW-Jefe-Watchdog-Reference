// COPYRIGHT DANIEL FREAKIN BORNHORST - Edited by Ben Hagle for KTX
// El Jefe Arduino project reference
// This listens for the show mode integer sent from El Jefe. Probably use this version going forward?
/****************************************** Jefe / Show Control Header Code *****************************************************/
/********************************************************************************************************************************/

// Autonet will automatically handle sending heartbeat to Jefe as well as safely sending all OSC messages.
#include "autonet.h"    // It must be included, https://github.com/MeowWolf/MWArduino
#include <SPI.h>
#include <Ethernet.h>
#include <IPAddress.h>
#include <EthernetUdp.h>
#include <OSCMessage.h>

#define USE_HW_WATCHDOG false // You don't always need the hardware watchdog. This is seperate from the Jefe heartbeat which is also referred to as a watchdog!

#if USE_HW_WATCHDOG
#include <Watchdog.h>   // Watchdog by Peter Polidoro. Can be found in the Arduino Library Manager or https://github.com/janelia-arduino/Watchdog
Watchdog watchdog;
#endif

// This microcontroller's network settings
// ALWAYS EDIT
IPAddress my_ip(10, 52, 120, 42);                             // Arduino/Teensy IP. Must be unique
byte my_mac[] = {0x56, 0x5A, 0x69, 0x66, 0x66, 0x61};         // Arduino/Teensy. Must be unique. https://miniwebtool.com/mac-address-generator/. Sometimes you have to pull this mac address off the ethernet shield.
char my_uuid[50] = "3208422c-44b6-47d1-88ba-a2f2b9b38bbd";    // Unique ID for each device. generate here https://www.uuidgenerator.net/version4
const unsigned int  my_localPort = 7777;                      // Can be set in Jefe Device Qsys plugin. Default is 7777


// Network settings for El Jefe Show Controller, as well as the "test" secondary core
IPAddress jefe_ip(10, 52, 120, 10);           // Primary Core
IPAddress jefe_ip2(10, 52, 120, 11);          // Test Core
const unsigned int jefe_port = 6666;          // Jefe Port 


byte ddns[] = {10, 52, 120, 1};
byte *gateway = ddns;
byte subnet_mask[] = {255, 255, 252, 0};


// Network settings for OSC messages. Typically Mac Mini Media Server (or OSC destination)
// ALWAYS EDIT
IPAddress remote_ip(10, 52, 120, 102);        // Audio recieving IP
const unsigned int remote_port = 6543;        // receiving port 


// Create your Autonet object, pass in the network settings from
Autonet autonet(my_ip, my_mac, my_uuid, ddns, subnet_mask, jefe_ip, jefe_port, jefe_ip2, jefe_port, 1);
EthernetUDP Udp;
int is_connected = 1;                         // 0 = broken connection, 1 = connected. Not required, but can be handy

const unsigned int MAX_MESSAGE_LENGTH = 12;

enum ShowModes {
  STANDBY = 0,
  SHOW = 1,
  MAINTENANCE = 2,
  SENSORY = 3,
  HIGH_VISIBILITY = 9
  };

ShowModes showMode = SHOW;                      // Default mode is always show mode
#define USE_SENSORY false                       // Set to false if sensory mode should redirect to show mode

/* Most microcontroller projects will require zero changes when in sensory mode. If this is false, any mode change to sensory mode will instead put the
controller in show mode (1). This is so that you only have to code against switching to show mode, not against show and sensory mode.
If set to true, sensory mode will be set like normal.*/

// Level 1-3 show modes should generally be ignored unless specific requirements for a device in an overlay mode exist.
// Always follow your level 0 show mode at all times, while using the higher overlay modes as a trigger to enter an alternate show state.
int level1_overlay = 10;                       
int level2_environmental = 20;
int level3_moment = 30;

bool transmissionEvent = false;

/******************************************** End Jefe / Show Control Code ******************************************************/
/********************************************************************************************************************************/


void setup() {

  Serial.begin(9600);
  delay(500);

  /************** Network Setup Code *******************/
  // None of this should need to be changed
  Ethernet.begin(my_mac, my_ip, ddns, gateway, subnet_mask);
  Udp.begin(my_localPort);
  autonet.setup(Udp);
  /****************************************************/

  #if USE_HW_WATCHDOG
  watchdog.enable(Watchdog::TIMEOUT_1S);
  #endif


  // Do Stuff Blah Blah
  Serial.println("Startup");

}


void loop() {
  /************** Network Loop Code *******************/
  autonet.loop();         // Pat the show manager. If this is not called the hearbeat will not be sent.
  checkForOSCMessage();   // Call this to check for received OSC messages
  /****************************************************/

  #if USE_HW_WATCHDOG
  watchdog.reset();
  #endif

  switch (showMode) { // do your thing in show
    case SHOW:
      break;
    case MAINTENANCE:
      break;
    case STANDBY:
      break;
    case HIGH_VISIBILITY:
      break;
  }

  // Example code: Sending anything over Serial will cause a reboot
  if (Serial.available()) {
    Serial.println("Reboot");
    sendOSCMessage(0);
    delay(500);
    reboot();
    Serial.println("reboot done");    // This line should never print if reboot was successful
  }
}

// Example: How to send custom OSC messages
// outMsg("/argument") replace /argument with your OSC address string
// outMsg.add(....) replace .... with OSC variable. This line can be omitted if no variable needed
void sendOSCMessage(int argument){
  
  // Using MW Autonet to easily send OSC
  OSCMessage outMsg("/example");
  outMsg.add(argument);
  is_connected = autonet.sendOSC(&outMsg, remote_ip, remote_port);



  //The raw dog method for sending OSC
  OSCMessage msg("/example");
  msg.add(argument);
  Udp.beginPacket(remote_ip, remote_port);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
}


// Example: How to check for incomming OSC Messages
void checkForOSCMessage() {
  OSCMessage msgIn;
  int s;
  if( (s = autonet.Udp->parsePacket() ) > 0) {
    Serial.println("got message!");
    
    while(s--) {
      msgIn.fill(autonet.Udp->read());
    }
    
    if(!msgIn.hasError()) {
      // OSC Command response functions
      // You can add as many callbacks here as you want

      // custom message handlers
      msgIn.route("/KTX/OBS/myProject/message", messageHandler);
      msgIn.route("/te/active", transmissionHandler); // need to set up an OSCSender in Q-Sys for this to work properly, but here it is!

      // show mode handlers
      msgIn.route("/show/mode", showModeHandler);
      msgIn.route("/reboot", rebootHandler);
    }
    else {
      Serial.println("[OSC ERROR] --- MSG_HAS_ERROR");
    }
  }
}

void messageHandler(OSCMessage &msg, int addrOffset )
{
  // do the thing!
  if (msg.size() == 1 && msg.isInt(0)) {
    Serial.println("Got a message! Received an int!");
  } else {
    Serial.println("Got a message!");
  }
}

void transmissionHandler(OSCMessage &msg, int addrOffset )
{
  transmissionEvent = (msg.getInt(0) == 1);
}

void showModeHandler(OSCMessage &msg, int addrOffset )
{
  int mode = msg.getInt(0);
  setShowMode(mode);
  
  autonet.echo(jefe_ip, jefe_port, &msg); // always be sure to echo the command back to the show controller!
  autonet.echo(jefe_ip2, jefe_port, &msg);
  //run mode has normal sound and lights 
}

void setShowMode(int mode)
{
  if(mode < 10)
  {
    #if USE_SENSORY == true
      showMode = (ShowModes)mode;
    #else
      if(mode == 3) showMode = SHOW; // sensory mode == show mode when not using sensory. this is so you only have to check mode == SHOW
      else showMode = (ShowModes)mode;
    #endif
    
  }
  else if(mode >= 10 && mode < 20)
  {
    level1_overlay = mode;
  }
  else if(mode >= 20 && mode < 30)
  {
    level2_environmental = mode;
  }
  else if(mode >= 30 && mode < 40)
  {
    level3_moment = mode;
  }
}

void rebootHandler(OSCMessage &msg, int addrOffset ) {
  Serial.println("Reboot command received");
  autonet.echo(jefe_ip, jefe_port, &msg);  // always be sure to echo the command back to the show controller!
  autonet.echo(jefe_ip2, jefe_port, &msg);
  
  // You have a couple of options to reboot the microcontroller based on the model
  
  reboot();
  //rebootTeensy();
  //_reboot_Teensyduino_();
}


void reboot() {
  while(1) {}   // This will trigger the hardware watchdog and cause a reboot
}

// The above reboot funtion will reboot any hardware including teensy
// This fuction is just an alternative method to software reboot a teensy
void rebootTeensy() {
// Check for Teensy Hardware at compile time
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__) || defined(__MKL26Z64__)
  SCB_AIRCR = 0x05FA0004; // this reboot command only works on TEENSY
#endif
}
