#include "SettingsServer.h"
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include "tftScreen.h"

const char* pvhostname = "pvmon";
const char* pvspiffspath = "/pvespupdate/pvupdatespiffs.php";
const char* pvfirmwarepath = "/pvespupdate/pvupdatespiffs.php";
const char* pvupdateserver = "";

ESP8266WebServer server(80);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdater;

extern void pvStatsTask(void);
extern void nextScreen(void);
extern const String pvVersion;
void showWifiConfigAPMessage(void);

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
  char persistentApiKey[APIKEY_LENGTH + 1];  
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
  char persistentsysid[SYSTEMID_LENGTH + 1];  
  getSystemID(persistentsysid);
  server.send(200, "text/plain", persistentsysid);
 
}

void handle_wifisetup(void){
  server.send(200, "text/plain", "Resetting WiFi settings and rebooting, connect to PV_setup_AP to setup new wifi connection.");
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  delay(1000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
}

void handle_brightness(void)
{
    String aStr = server.arg("brightness");
    int brightness = aStr.toInt();
    if ((brightness >= 0) && (brightness <= 1023))
    {
      analogWrite(0,brightness);
    }
    server.send(200, "text/plain", "brightness set");
}


void handle_reboot(void)
{
  server.send(200, "text/plain", "restarting.......");
  delay(600);
  ESP.reset();
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
  server.on("/reboot", handle_reboot);
  server.on("/brightness",handle_brightness);
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

void showWifiConfigAPMessage(WiFiManager *myWiFiManager)
{
    tftShowStartUpText("No WiFi, Connect",0);
    tftShowStartUpText("to PV_setup_AP",1);
    tftShowStartUpText("hotspot and go to",2); 
    char ipstr[15];
    WiFi.softAPIP().toString().toCharArray(ipstr,sizeof(ipstr));   
    tftShowStartUpText(ipstr,3);    
}
void setupWiFi(void){
    // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManager wifiManager;
  wifiManager.setMinimumSignalQuality();
  wifiManager.setAPCallback(showWifiConfigAPMessage);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectTimeout(30);
  if (!wifiManager.autoConnect("PV_setup_AP")) { 
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
}



