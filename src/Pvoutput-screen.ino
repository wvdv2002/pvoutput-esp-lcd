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
#include "theThings.h"

#include "pvstats.h"
#include "pvstatus.h"

const char pvVersion[] = "PVLCDVER100";

extern "C" {
  #include "user_interface.h"
}

PvStats pvStats;
PvSystemService pvSystemService;
PvStatus pvStatus;

void setupWindpark(void);
void windparkStatsTask(void);
void nextScreen(void);
void getAllWindparkStats(void);

Ticker nextScreenTicker;
Ticker windparkTaskTicker;


bool updateScreen = 0;
bool updateParkData = 0;
String parkName;
float hourValues[24];
int totalDay;
TheThings theThings;

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
  tftShowStartUpText("Output data ",1);
  utilStart();
  startSettingsServer();
  setupTheThings();
 // Connect to WiFi network
  windparkTaskTicker.attach(300,pvStatsTask);
  nextScreenTicker.attach(30,nextScreen);
  Serial.print("Heap free: ");
  Serial.println(system_get_free_heap_size());

  
  tftDrawGraphScreen();
  nextScreen();
  getAllParkStats();
}


void loop(void)
{
  settingsServerTask();
  delay(1);

  if(updateParkData)
  {
    updateParkData = 0;
    getAllParkStats();
  }
  if(updateScreen)
  {
    updateScreen = 0;
    screenTask();
  }
}

void getAllParkStats(void)
{
    float tempDay;
    if(!theThings.getThingsInfo()) {
      Serial.println("no stats received error");
      return;
    }
    theThings.getThingNames();
    parkName = theThings.getThingName(0);
    theThings.getThingKwhList(hourValues,24,0);
    tempDay = 0.0f;
    for(int i=0;i<24;i++){
      hourValues[i] *= 1000.0;
      tempDay += hourValues[i];
      Serial.print(hourValues[i]);
      Serial.print(',');
    }
    totalDay = (int)tempDay;
    Serial.println("");
}

void nextScreen(void)
{
    updateScreen = 1;
}

void pvStatsTask(void)
{
  updateParkData = 1;
}

void setupTheThings(void)
{
  char apiKey[APIKEY_LENGTH];
  char systemId[SYSTEMID_LENGTH];
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
  theThings.begin(apiKey, systemId);
}
