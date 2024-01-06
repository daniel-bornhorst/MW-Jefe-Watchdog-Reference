
/********************************************************************************************************************************/
/****************************************** Jefe / Show Control Header Code *****************************************************/
/********************************************************************************************************************************/

// Autonet will automatically handle sending heartbeat to Jefe as well as safely sending all OSC messages.
#include "autonet.h" // It must be included

// MY network settings
IPAddress my_ip(10,32,16,33);                                 // Arduino/Teensy IP. Must be unique
byte my_mac[] = {0x56, 0x5A, 0x69, 0x66, 0x66, 0x61};         // Arduino/Teebsy. Must be uniqe. https://miniwebtool.com/mac-address-generator/
char my_name[50] = "9c710fdc-70cf-405f-8363-dd71efbfea3c";    // Unique ID for each device. generate here https://www.uuidgenerator.net/version4
const unsigned int  my_localPort = 6667;                      // Can be set in Jefe Device Qsys plugin. Default is 6667


// Network settings for Jefe Show controller and redudant core (QSys Core 101 and 104)
IPAddress jefe_ip(10, 32, 16, 10);          // Primary Core
// IPAddress jefe_ip2(10, 32, 16, 13);      // Redundant Core
IPAddress jefe_ip2(10, 32, 16, 14);         // Test Core
const unsigned int jefe_port = 6666;        // Jefe Port


// Stuff we need if we're not on 10.32.16.*
// This should not need to be changed.
byte ddns[] =     {10, 32, 16, 1};
byte *gateway =   ddns;
byte subnet[] =   {255, 255, 240, 0};


// Network settings for OSC messages. Typically Mac Mini Media Server (or OSC destination)
IPAddress remote_ip(10, 32, 16, 102);    // Audio recieving IP
const unsigned int remote_port = 6543;   // receiving port 


// Create your Autonet object, pass in the network settings from
Autonet autonet(my_ip, my_mac, my_name, ddns, subnet, jefe_ip, jefe_port, jefe_ip2, jefe_port, 1);
EthernetUDP Udp;
int is_connected = 1;               // 0 = broken connection, 1 = connected. Not required, but can be handy

/********************************************************************************************************************************/
/******************************************** End Jefe / Show Control Code ******************************************************/
/********************************************************************************************************************************/



void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}