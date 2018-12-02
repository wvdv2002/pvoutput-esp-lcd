#ifndef thethings_h
#define thethings_h

#include <stddef.h>
#include <stdint.h>
#include <ESP8266WiFi.h>
#include "Arduino.h"
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>
#define MAXIMUMTHINGIDS 5


class TheThings{
public:
  void begin(String apiKey,String systemId);
  TheThings();
  bool getData(void);
  bool getThingsInfo();
  int getThingKwhList(float*,int size,int id);
  bool getThingNames();
  String getThingName(int id);
  float getThingTotalMonth(int id);
  float getThingTotalDay(int id);

protected:

private:
  int read(WiFiClient &client, char *buff, int len);
  bool request(WiFiClient &client, String path, String authorization);
  bool getThingIds(WiFiClient&);
  String getThingName(WiFiClient&);
  String _apiKey;
  String _systemId;
  String _idList[MAXIMUMTHINGIDS];
  String _nameList[MAXIMUMTHINGIDS];
  float _maximumKw[MAXIMUMTHINGIDS];
  int _amountOfIds;


};

#endif
