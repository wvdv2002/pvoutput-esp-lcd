#include "thethings.h"
#include "WiFiClientSecure.h"

int TheThings::read(WiFiClient &client, char *buff, int len) {
  int readBytes = 0;
  int b;
  while((client.connected() || client.available() > 0) && readBytes < len) {
    if ((b = client.read()) != -1) {
      buff[readBytes++] = (char)b;
    }
    delay(0);
  }
  delay(0);
  return readBytes;
}

TheThings::TheThings()
{
}


void TheThings::begin(String apiKey, String systemId) {
  _apiKey = apiKey;
  _systemId = systemId;
}

bool TheThings::getThingsInfo(){
  String path = "https://api.thethings.io/v2/me/things";
  Serial.println(path);
   WiFiClientSecure client;
  if(!request(client,path,_apiKey))
  {
    return false;
  }
  getThingIds(client);
   for(int i = 0;i<_amountOfIds;i++){
    Serial.println(_idList[i]);
  }
  client.stop();
  return 1;
}

int TheThings::getThingKwhList(float* aList,int maxSize,int idNumber){
  if(idNumber>=_amountOfIds){return 0;};
  String path = "https://api.thethings.io/v2/me/resources/production-hour-v4/" +_idList[idNumber]+"?limit="+String(maxSize);
  Serial.println(path);
   WiFiClientSecure client;
  if(!request(client,path,_apiKey))
  {
    return false;
  }
  char emptyBuffer[10];
  int size=0;
  client.readBytes(emptyBuffer,1);
  while(1){
    DynamicJsonBuffer jb(256);
    JsonObject& root = jb.parseObject(client);
    root.prettyPrintTo(Serial); 
    client.readBytes(emptyBuffer,1);
    if(!root.success()){break;}   
    aList[size++]= root["value"].as<float>();
    if(emptyBuffer[0]==']'){break;};
    if(size>=maxSize){break;}
  }
  client.stop();
  return size;
}

String TheThings::getThingName(int id){
  if(id>=_amountOfIds){return "";}
  return _nameList[id];
}


bool TheThings::getThingNames(){
  String path = "https://api.thethings.io/v2/me/resources/publicDescription/";
   WiFiClientSecure client;
  char emptyBuffer[10];
  for(int i = 0;i<_amountOfIds;i++){
    Serial.println(i);
    Serial.println(path+_idList[i]);
    Serial.flush();
    if(!request(client,path+_idList[i],_apiKey))
    {
      return false;
    }
    Serial.println("gotconnection");
    Serial.flush();
    client.readBytes(emptyBuffer,1);
    DynamicJsonBuffer jb(2200);
    JsonObject& root = jb.parseObject(client);
    root.prettyPrintTo(Serial); 
    client.readBytes(emptyBuffer,1);
    if(!root.success()){break;}   
    _nameList[i]= root["value"]["name"].as<String>();
    _maximumKw[i] = root["value"]["public"]["maxPower"].as<float>();
    client.stop();
  }
  for(int i = 0;i<_amountOfIds;i++){
      Serial.print(_nameList[i]);
      Serial.print(' ');
      Serial.println(String(_maximumKw[i]));
  }
  return 1;
}

bool TheThings::getThingIds(WiFiClient &client){
  char emptyBuffer[10];

  client.readBytes(emptyBuffer,1);
  int i = 0;
  while(1){
    DynamicJsonBuffer jb(1024);
    JsonObject& root = jb.parseObject(client);
    root.prettyPrintTo(Serial); 
    if(!root.success()){break;}   
    _idList[i++] = root["_id"].as<String>();
    _amountOfIds = i;
    if(i>=MAXIMUMTHINGIDS){break;}
    client.readBytes(emptyBuffer,1);
    if(emptyBuffer[0]==']'){break;};
  }
  client.stop();
  return 1;
}



bool TheThings::request(WiFiClient &client, String path, String authorization)
{
   String host = "api.thethings.io";
  if (!client.connect(host.c_str(), 443)) {
    Serial.println("connection failed");
    return false;
  }


  String query = String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: " + authorization + "\r\n"
               "Connection: close\r\n\r\n";
  client.print(query);

  // Skipping HTTP headers
  const char *searchStr = "\r\n\r\n";
  int searchIndex = 0;
  int searchLen = 4;
  char c;
  while(read(client, &c, 1) > 0) {
    if (searchStr[searchIndex] == c) {
      searchIndex++;
      if (searchIndex >= searchLen) {
        break;
      }
    } else {
      searchIndex = 0;
    }
  }

   return true;
}
