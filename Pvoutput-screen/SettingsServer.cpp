#include "SettingsServer.h"
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>

const char* pvhostname = "pvmon";
const char* pvspiffspath = "/pvespupdate/pvupdatespiffs.php";
const char* pvfirmwarepath = "/pvespupdate/pvupdatespiffs.php";
const char* pvupdateserver = "www.ledscircle.com";

ESP8266WebServer server(80);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdater;

extern void pvStatsTask(void);
extern void nextScreen(void);
extern const String pvVersion;


void CheckOTAServer(void)
{
//  t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(pvupdateserver, 80, pvspiffspath, pvVersion);
  t_httpUpdate_return ret = ESPhttpUpdate.update(pvupdateserver, 80, pvfirmwarepath, pvVersion);
switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update ok."); // may not called we reboot the ESP
        break;
}
}



void handle_root(){
  server.send(200, "text/plain", "ESP8266 Pvoutput monitor");
  delay(100);
}

void handle_apikey(void){
  String apiKey = server.arg("apikey");
  if (apiKey.length()>0){
    if(!setApiKey(apiKey)){Serial.println("Writing failed");};
  }
  char persistentApiKey[PVAPIKEY_LENGTH + 1];  
  getApiKey(persistentApiKey);
  server.send(200, "text/plain", persistentApiKey);
}

void handle_refresh(void){
  pvStatsTask();
  nextScreen();
  server.send(200, "text/plain", "Refreshing now");
}

void handle_sysid(void){
    String sysid = server.arg("systemid");
  if (sysid.length()>0){
    if(!setSystemID(sysid)){Serial.println("Writing failed");};
  }
  char persistentsysid[PVSYSTEMID_LENGTH + 1];  
  getSystemID(persistentsysid);
  server.send(200, "text/plain", persistentsysid);
 
}

void handle_wifisetup(void){
  setupWiFi();
}

void startSettingsServer(void){
  ArduinoOTA.setHostname(pvhostname);
  ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });
  ArduinoOTA.begin();
  httpUpdater.setup(&server);  
  server.on("/", handle_root);
  server.on("/apikey", handle_apikey);
  server.on("/refresh", handle_refresh);
  server.on("/systemid",handle_sysid);
  server.on("/wifisetup",handle_wifisetup);
  server.begin();
  Serial.println("HTTP server started");
  
  if(!mdns.begin(pvhostname, WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
} 
   
void settingsServerTask(void){
  ArduinoOTA.handle();
  server.handleClient();
  mdns.update();
}

void setupWiFi(void){
  char customSystemIDStr[PVSYSTEMID_LENGTH];
  char customPvApiKeyStr[PVAPIKEY_LENGTH];
    // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_systemid("systemid", "000000", customSystemIDStr, PVSYSTEMID_LENGTH);
  WiFiManagerParameter custom_pvapikey("PV-apikey", "1234", customPvApiKeyStr, PVAPIKEY_LENGTH);
  
  WiFiManager wifiManager;
  wifiManager.setMinimumSignalQuality();
  wifiManager.addParameter(&custom_systemid);
  wifiManager.addParameter(&custom_pvapikey);
  if (!wifiManager.autoConnect("PV_setup_AP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  strcpy(customSystemIDStr, custom_systemid.getValue());
  strcpy(customPvApiKeyStr, custom_pvapikey.getValue());
//  strcpy(blynk_token, custom_blynk_token.getValue());
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
}



