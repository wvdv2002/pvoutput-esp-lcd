#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "SettingsServer.h"
#include "pvoutput.h"
#include <Ticker.h>
#include "tftScreen.h"


const char pvVersion[] = "PVEAZVER100";

extern "C" {
  #include "user_interface.h"
}


void setupPvOutput(void);
void pvStatsTask(void);
void nextScreen(void);


Ticker nextScreenTicker;
Ticker pvTaskTicker;
PvStats pvStats;
PvStatus pvStatus;
PvSystemService pvSystemService;
bool updateScreen = 0;
bool updatePvOutputData = 0;

void setup(void)
{
  delay(100);
  Serial.begin(115200);
  Serial.println("start"); 
  tftSetup();
 // Connect to WiFi network
  setupWiFi();
  utilStart();
  startSettingsServer();
  setupPvOutput();
  pvTaskTicker.attach(300,pvStatsTask);
  nextScreenTicker.attach(10,nextScreen);
  Serial.print("Heap free: ");  
  Serial.println(system_get_free_heap_size());
  pvStatsTask(); 
  nextScreen();
}

 
void loop(void)
{
  settingsServerTask();
  delay(1);

  if(updateScreen)
  {
    updateScreen = 0;
    screenTask();
  }
  if(updatePvOutputData)
  {
    updatePvOutputData = 0;
    if(!PVOutput.getStats(&pvStats,GRAPHWIDTH)) {
      Serial.println("no stats received error");
    }
    PVOutput.getStatus(&pvStatus);
    PVOutput.getPvSystemService(&pvSystemService);
  }
} 

void nextScreen(void)
{
    updateScreen = 1;
}

void pvStatsTask(void)
{
  updatePvOutputData = 1;
}

void setupPvOutput(void)
{
  char apiKey[41];
  char systemId[8];
  if(!getApiKey(apiKey))
  {
    setApiKey("EAZWindData");
    getApiKey(apiKey);
  }
  if(!getSystemID(systemId))
  {
    setSystemID("21479");
    getSystemID(systemId);
  }
  
  Serial.print("Apikey: ");Serial.println(apiKey);
  Serial.print("Systemid: ");Serial.println(systemId);
  PVOutput.begin(apiKey, systemId);
}

