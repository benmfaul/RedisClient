#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>

#include <RedisClient.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "SuperiorCourtData"           // cannot be longer than 32 characters!
#define WLAN_PASS       "jiujitsu"

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
                                   
uint32_t ip;                       // the IP address

/////////////////////////////////////////////////////////////////////////////////////////////////////////


RedisClient* redis;
#define LED  8                       // we will blink an LED on pin 8.

void setup() {
  Serial.begin(19200);
  pinMode(LED,OUTPUT);
  
  ////////////// Adafruit CC3000 code ////////////////////////////////////
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  // Optional SSID scan
  // listSSIDResults();
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////// REDIS SETUP /////////////////////////////////////////////////////////////

  /**
   * Set the host address for the REDIS server. On my system its 192,168.1.2
   */
    ip = cc3000.IP2U32(52, 21, 85, 247);
 // ip = cc3000.IP2U32(192, 168, 1, 2);                        

  Serial.println("HERE WE GO");
  redis = new RedisClient(ip,6379, &cc3000);

}

void loop() {
  char buffer[32];
  
  redis->connect();
  Serial.println("Connected, now we will test!");

  redis->SUBSCRIBE("xxx",buffer,32);
  
  redis->DEL("test");

  long time=millis();
  long i = redis->INCR("test");
  time = millis() - time;
  if (i != 1) {
    Serial.println("INCR FAILED");
    while(1);
  }
  Serial.print("Time (ms) to increment = "); Serial.println(time);

  i = redis->PUBLISH("xxx","1234567890");
  Serial.print("PUB: "); Serial.println(i);
  
  redis->SET("junk","hello bye");
  redis->GET("junk",buffer,32);
  if (strcmp(buffer,"hello bye") != 0) {
    Serial.println("GET/SET FAILED");
    while(1);;
  }

  redis->DEL("list");
  redis->startRPUSH("list",6);
  redis->addArg("1");
  redis->addArg("2");
  redis->addArg("3");
  redis->addArg("4");
  redis->addArg("5");
  redis->addFloatArg(1.23);
  time=millis();
  i = redis->endPUSH();
  time = millis() - time;
  Serial.print("RPUSHED = "); Serial.print(i); Serial.print(" items");;
  Serial.print("RPUSH, time (MS) was "); Serial.println(time);

  redis->SET("bignum","4554848883888123");
  redis->INCR("bignum",buffer,32);
  if (strcmp(buffer,"4554848883888124")!=0) {
    Serial.println("Increment Big Number Failed!");
    while(1);
  }
  
  redis->DECR("bignum",buffer,32);
  if (strcmp(buffer,"4554848883888123")!=0) {
    Serial.println("Decrement Big Number Failed!");
    while(1);
  }

  i = redis->LTRIM("list",1,3);
  Serial.print("TRIM: "); Serial.println(i);
  int mode = 0;

  while(1) {
    i = redis->INCR("test");

    Serial.println(i); 
    if (mode == 0) {
      digitalWrite(LED,HIGH);
    } else {
      digitalWrite(LED,LOW);
    }
    mode = !mode;
    if (i % 300 == 0) {
      Serial.println("STOPPING");
      redis->disconnect();
      redis->connect();
      Serial.println("RESTARTED");
    }
    delay(1000);
  }

}

void listSSIDResults(void)
{
  uint32_t index;
  uint8_t valid, rssi, sec;
  char ssidname[33]; 

  if (!cc3000.startSSIDscan(&index)) {
    Serial.println(F("SSID scan failed!"));
    return;
  }

  Serial.print(F("Networks found: ")); Serial.println(index);
  Serial.println(F("================================================"));

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);
    
    Serial.print(F("SSID Name    : ")); Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
