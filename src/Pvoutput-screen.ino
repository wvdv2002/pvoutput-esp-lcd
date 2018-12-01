#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "SettingsServer.h"
#include <Ticker.h>
#include "tftScreen.h"


const char pvVersion[] = "PVLCDVER100";

extern "C" {
  #include "user_interface.h"
}


void setupWindpark(void);
void windparkStatsTask(void);
void nextScreen(void);
void getAllWindparkStats(void);

Ticker nextScreenTicker;
Ticker windparkTaskTicker;
parkStats pvStats;
parkStatus pvStatus;
ParkDescription windParkDescription;
bool updateScreen = 0;
bool updateParkdata = 0;

void setup(void)
{
  analogWrite(0,0);
  delay(10);
  Serial.begin(115200);
  Serial.println("start"); 
  tftSetup();
  tftShowLedscircle();
  delay(1000);
  tftShowPVOutput();
  tftShowStartUpText("Trying WiFi",0); 
  setupWiFi();
  
  tftShowStartUpText("Wifi Connected",0); 
  delay(100);
  char ipstr[15];
   WiFi.localIP().toString().toCharArray(ipstr,sizeof(ipstr));
  tftShowStartUpText(ipstr,1); 
  delay(2000);
  tftShowStartUpText("Trying to get ",0); 
  tftShowStartUpText("pvoutput data ",1); 
  utilStart();
  startSettingsServer();
  setupPvOutput();  
 // Connect to WiFi network
  pvTaskTicker.attach(300,pvStatsTask);
  nextScreenTicker.attach(30,nextScreen);
  Serial.print("Heap free: ");  
  Serial.println(system_get_free_heap_size());
  
  getAllPvStats();
  tftDrawGraphScreen();
  nextScreen();
}

 
void loop(void)
{
  settingsServerTask();
  delay(1);

  if(updatePvOutputData)
  {
    updatePvOutputData = 0;
    getAllPvStats();
  }
  if(updateScreen)
  {
    updateScreen = 0;
    screenTask();
  }
} 

void getAllPvStats(void)
{
    if(!PVOutput.getStats(&pvStats,GRAPHWIDTH)) {
      Serial.println("no stats received error");
    }
    PVOutput.getStatus(&pvStatus);
    PVOutput.getPvSystemService(&pvSystemService);
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
    setApiKey("");
    getApiKey(apiKey);
  }
  if(!getSystemID(systemId))
  {
    setSystemID("");
    getSystemID(systemId);
  }
  
  Serial.print("Apikey: ");Serial.println(apiKey);
  Serial.print("Systemid: ");Serial.println(systemId);
  PVOutput.begin(apiKey, systemId);
}

