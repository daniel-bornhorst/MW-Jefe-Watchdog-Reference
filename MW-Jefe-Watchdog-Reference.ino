// COPYRIGHT DANIEL FREAKIN BORNHORST
/****************************************** Jefe / Show Control Header Code *****************************************************/
/********************************************************************************************************************************/

// Autonet will automatically handle sending heartbeat to Jefe as well as safely sending all OSC messages.
#include "autonet.h"    // It must be included, https://github.com/MeowWolf/MWArduino

#if defined(__MKL26Z64__)
extern "C" void startup_early_hook(void);

void startup_early_hook(void) {
  SIM_COPC = 0x0C;   // 1024 ms
  //SIM_COPC = 0x08; //  256 ms
  //SIM_COPC = 0x04; //   32 ms
}
#else
#include <Watchdog.h>   // Watchdog by Peter Polidoro. Can be found in the Arduino Library Manager or https://github.com/janelia-arduino/Watchdog
Watchdog watchdog;
#endif

// MY network settings
// ALWAYS EDIT
IPAddress my_ip(10,32,16,33);                                 // Arduino/Teensy IP. Must be unique
byte my_mac[] = {0x56, 0x5A, 0x69, 0x66, 0x66, 0x61};         // Arduino/Teebsy. Must be uniqe. https://miniwebtool.com/mac-address-generator/
char my_name[50] = "3208422c-44b6-47d1-88ba-a2f2b9b38bbd";    // Unique ID for each device. generate here https://www.uuidgenerator.net/version4
const unsigned int  my_localPort = 6667;                      // Can be set in Jefe Device Qsys plugin. Default is 6667


// Network settings for Jefe Show controller and redudant core (QSys Core 101 and 104)
// SOMETIMES EDIT
IPAddress jefe_ip(10, 32, 16, 10);          // Primary Core
// IPAddress jefe_ip2(10, 32, 16, 13);      // Redundant Core
IPAddress jefe_ip2(10, 32, 16, 14);         // Test Core
const unsigned int jefe_port = 6666;        // Jefe Port


// Stuff we need if we're not on 10.32.16.*
// NEVER EDIT
byte ddns[] =     {10, 32, 16, 1};
byte *gateway =   ddns;
byte subnet[] =   {255, 255, 240, 0};


// Network settings for OSC messages. Typically Mac Mini Media Server (or OSC destination)
// ALWAYS EDIT
IPAddress remote_ip(10, 32, 16, 102);    // Audio recieving IP
const unsigned int remote_port = 6543;   // receiving port 


// Create your Autonet object, pass in the network settings from
Autonet autonet(my_ip, my_mac, my_name, ddns, subnet, jefe_ip, jefe_port, jefe_ip2, jefe_port, 1);
EthernetUDP Udp;
int is_connected = 1;               // 0 = broken connection, 1 = connected. Not required, but can be handy


/******************************************** End Jefe / Show Control Code ******************************************************/
/********************************************************************************************************************************/


void setup() {

  Serial.begin(9600);
  delay(500);
  
  // Do Stuff Blah Blah

  /************** Network Setup Code *******************/
  // Non of this should need to be changed
  // Ethernet.begin(my_mac, my_ip, ddns, gateway, subnet);
  // Udp.begin(my_localPort);
  // autonet.setup(Udp);
  /****************************************************/


  // Do Stuff Blah Blah


  /*============= Hardware Watchdog code ==============*/
  // Setup watchdog
#if !defined(__MKL26Z64__)
  watchdog.enable(Watchdog::TIMEOUT_1S);    // Watchdog time is 1s, Teensy cannot be set to shorter than than this
#endif
  /*===================================================*/

  // Do Stuff Blah Blah
  Serial.println("Startup");

}


void loop() {
  /************** Network Loop Code *******************/
  // autonet.loop();         // Pat the show manager. If this is not called the hearbeat will not be sent.
  // checkForOSCMessage();   // Call this to check for received OSC messages
  /****************************************************/

#if defined(__MKL26Z64__)
  //watchdog_reset();
  SIM_SRVCOP = 0x55;
  SIM_SRVCOP = 0xAA;
#else
  watchdog.reset();
#endif

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
// outMsg("/doorbell") replace /doorbell with your OSC address string
// outMsg.add(....) replace .... with OSC variable. This line can be omitted if no variable needed
void sendOSCMessage(int doorbell){
  OSCMessage outMsg("/doorbell");
  outMsg.add(doorbell);
  is_connected = autonet.sendOSC(&outMsg, remote_ip, remote_port);
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
      msgIn.route("/reboot", rebootCallback);
      msgIn.route("/halt", haltCallback);
    }
    else {
      Serial.println("[OSC ERROR] --- MSG_HAS_ERROR");
    }
  }
}


void rebootCallback(OSCMessage &msg, int addrOffset ) {
  Serial.println("Reboot command received");
  autonet.echo(&msg);
  reboot();
  //while(1) {}
}


void haltCallback(OSCMessage &msg, int addrOffset ) {
  Serial.println("Halt! command received");
  autonet.echo(&msg);
  reboot();
  //while(1) {}
}



void reboot() {
  while(1) {}   // This will trigger the hardware watchdog and cause a reboot
}

// The above reboot funtion will reboot any hardware including teensy
// This fuction is just an alternative method to software reboot a teensy
void rebootTeensy() {
// Check for Teensy Hardware at compile time
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__)
  SCB_AIRCR = 0x05FA0004; // this reboot command only works on TEENSY
#endif
}
