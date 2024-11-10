// Project - YouHaveMail! (By: Jems Chaudhary)

/**
 *  Created a battery-powered IoT device using an ESP8266 to detect mailbox activity, demonstrated interest in smart home applications.
 *  Configured ThingSpeak to send automated email or Twitter notifications upon mailbox updates, showcasing experience with cloud, SaaS, and
 *  web services to support real-time, remote monitoring for smart devices.
 */

#include <Arduino.h>
// for wifi
#include <ESP8266WiFi.h>
#include <Wire.h>
// for Arduino Wifi shield
#include <WiFiUdp.h>
// for thingSpeak 
#include <ThingSpeak.h> 
// for ntp client
#include <NTPClient.h>
// for flash memory
#include <ESP_EEPROM.h>
// secrets - wifi and thinkSpeak
#include "secrets.h"

// WiFI server.  Listen on port 80, which is the default TCP port for HTTP
WiFiServer server(80);
// WiFi client
WiFiClient wifiClient;
// Wifi ntp - UDP
WiFiUDP UDP;
// NTP Client (EST)
NTPClient ntpClient(UDP, "3.ca.pool.ntp.org", -18000);

// Wifi login details
char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 
// Channel details for Write action
int TS_Ch_ID = SECRET_CH_ID;
const char *TS_Ch_WriteApiKey = SECRET_WRITE_APIKEY;

//#define PIN_BUTTON RST                                                    // might be not needed

/**
 * Setup function connects to wifi and configuring libraries
*/
void setup()
{
  // configure the USB serial monitor
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // configures thingSpeak
  ThingSpeak.begin(wifiClient); 
  // configures eeprom - memory
  EEPROM.begin(16);
  // configures ntpClient
  ntpClient.begin();

  // Button is an INPUT 
  //pinMode(PIN_BUTTON, INPUT_PULLUP);                                        // might be not needed

  Serial.println("\n\n********   Project - You have a Mail!   **********");
  Serial.println("  I Jems Chaudhary (000814314) certify that this material is my original work. \n\tNo other person's work has been used without due acknowledgement. \n\tI have not made my work available to anyone else.");

  // connect to WiFi
  Serial.printf("\n-->Connecting to %s ", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}

void loop()
{
  // fetching time by calling update or forceUpdate method
  if(!ntpClient.update()){
    ntpClient.forceUpdate();
  }

  //getting the last value from memory 
  unsigned long lastVal;
  EEPROM.get(0,lastVal);

  //Only if 60 sec elapsed else go back to deep sleep
  // Checking if substraction of current time and lastly stored value is greatr than 60 (sec),
  //    Then: writing to thingSpeak, adding time val to memory.
  if(ntpClient.getEpochTime() - lastVal > 60){

    //Writing to the thingspeak at field 1, value 1.
    int statusCode = ThingSpeak.writeField(TS_Ch_ID, 1, 1, TS_Ch_WriteApiKey);
    
    if(statusCode == 200){
      Serial.println("--> ThingSpeak updated on time: "+ ntpClient.getFormattedTime()); 

      // writimg time to memory
      EEPROM.put(0, ntpClient.getEpochTime());
      
      // if not added to memory, mentioning to terminal.
      bool isCommited = EEPROM.commit();
      if(isCommited){
        Serial.println("-->Memory updated!");
      }
      else{
        Serial.println("-->Memory update action Failed!");
      }
    }
    else{
      // thingSpeak operation failed!
      Serial.println("-->ThingSpeak operation failed! Found code:" + String(statusCode) + " !");
    }
  }
  else{
      Serial.println("Watch out! “mischievous kid next door” is playing with our mail-box!");
      delay(10);      // to print function complete print
  }

  // sending D1Mini to deep sleep
  ESP.deepSleep(0);
}
