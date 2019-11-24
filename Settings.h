
#include <ESP8266WebServer.h> //-- https://github.com/esp8266/ESPWebServer
#include <EEPROM.h>


typedef struct TConfigData
{
  uint8_t signature[2];
  uint8_t version;

  char ssid[20];
  char password[20];

  char mqttServer[200];
  int mqttPort;
  char mqttUser[20];
  char mqttPass[20];
  char deviceName[50];

  char relayTopic[200];
  char relayTopicOn[20];
  char relayTopicOff[20];
   
};

class AppConfig
{
public: 
  void begin()
  { 
    if ( !load() ) {
      defSettings();
      save();
    }
  };

  TConfigData data;
  
  void defSettings(void)
  {
    data.signature[0] = EEPROM_SIG[0];
    data.signature[1] = EEPROM_SIG[1];
    data.version = CONFIG_VERSION;

    strncpy(data.ssid, "WiFi SSID", 20);
    strncpy(data.password, "WiFi password", 20);
    strncpy(data.mqttServer, "127.0.0.1", 200);    
    strncpy(data.mqttUser, "root", 20);
    strncpy(data.mqttPass, "root", 20);
    data.mqttPort = 1384;
    
    strncpy(data.deviceName, "testDevice", 50);
    strncpy(data.relayTopic, "/test1", 200);
    strncpy(data.relayTopicOn, "1", 20);
    strncpy(data.relayTopicOff, "0", 20);
  }
  
  bool load(void)
  {
    EEPROM.begin(4096);
    EEPROM.get(EEPROM_ADDR, data);
    
    if ( data.signature[0] != EEPROM_SIG[0] && data.signature[1] != EEPROM_SIG[1] ) return false;    
    if ( data.version != CONFIG_VERSION ) {
      
    }
    data.version = CONFIG_VERSION;
 
    return true;
  }
  
  bool save(void)
  {
    EEPROM.begin(sizeof(data));
    EEPROM.put(EEPROM_ADDR, data);    
    EEPROM.end();
    return true;
  }
 
private:    
  const uint16_t EEPROM_ADDR = 0;
  const uint8_t EEPROM_SIG[2] = { 0xee, 0x11 };
  const uint8_t CONFIG_VERSION = 0;

};

AppConfig appConfig;


ESP8266WebServer server(80);

//========================================================================================================

/**
* @brief Отвечаем при запросе главной страницы 
*/
void handleRoot() 
{  
  if ( server.hasArg("action") ) {
    String action = server.arg("action");
    if ( action=="savesettings" ) {

      if ( server.hasArg("wifissid") ) { strncpy(appConfig.data.ssid, server.arg("wifissid").c_str(), 20); }
      if ( server.hasArg("wifipassword") ) { strncpy(appConfig.data.password, server.arg("wifipassword").c_str(), 20); }
      if ( server.hasArg("mqttserver") ) { strncpy(appConfig.data.mqttServer, server.arg("mqttserver").c_str(), 200); }
      if ( server.hasArg("mqttport") ) { appConfig.data.mqttPort = server.arg("mqttport").toInt(); }
      if ( server.hasArg("mqttuser") ) { strncpy(appConfig.data.mqttUser, server.arg("mqttuser").c_str(), 20); }
      if ( server.hasArg("mqttpassword") ) { strncpy(appConfig.data.mqttPass, server.arg("mqttpassword").c_str(), 20); }
      if ( server.hasArg("devicename") ) { strncpy(appConfig.data.deviceName, server.arg("devicename").c_str(), 50); }
      if ( server.hasArg("relaytopic") ) { strncpy(appConfig.data.relayTopic, server.arg("relaytopic").c_str(), 200); }
      if ( server.hasArg("relaytopicon") ) { strncpy(appConfig.data.relayTopicOn, server.arg("relaytopicon").c_str(), 20); }
      if ( server.hasArg("relaytopicoff") ) { strncpy(appConfig.data.relayTopicOff, server.arg("relaytopicoff").c_str(), 20); }

      Serial.println("Settings saved!");
      appConfig.save();
    }
  }

  String settingsPage = "\
  <!DOCTYPE html>\
  <html>\
    <head>\
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
      <link rel=\"icon\" href=\"data:,\">\
      <style>\
        html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\
      </style>\
    </head>\
    <body>\
      <h1>Sonoff web config</h1>\
      <form method=\"GET\">\
        <p>\
          <label for=\"wifissid\">WiFi SSID:</label><br/>\
          <input type=\"text\" id=\"wifissid\" name=\"wifissid\" value=\""+String(appConfig.data.ssid)+"\" /><br/>\
          <label for=\"wifipassword\">WiFi password:</label><br/>\
          <input type=\"password\" id=\"wifipassword\" name=\"wifipassword\" value=\""+String(appConfig.data.password)+"\" /><br/>\
        </p>\
        <p>\
          <label for=\"mqttserver\">MQTT server address:</label><br/>\
          <input type=\"text\" id=\"mqttserver\" name=\"mqttserver\" value=\""+String(appConfig.data.mqttServer)+"\" /><br/>\
          <label for=\"mqttport\">MQTT server port:</label><br/>\
          <input type=\"text\" id=\"mqttport\" name=\"mqttport\" value=\""+String(appConfig.data.mqttPort)+"\" /><br/>\
          <label for=\"mqttuser\">MQTT user name:</label><br/>\
          <input type=\"text\" id=\"mqttuser\" name=\"mqttuser\" value=\""+String(appConfig.data.mqttUser)+"\" /><br/>\
          <label for=\"mqttpassword\">MQTT user password:</label><br/>\
          <input type=\"password\" id=\"mqttpassword\" name=\"mqttpassword\" value=\""+String(appConfig.data.mqttPass)+"\" /><br/>\
        </p>\
        <p>\
          <label for=\"devicename\">Device name:</label><br/>\
          <input type=\"text\" id=\"devicename\" name=\"devicename\" value=\""+String(appConfig.data.deviceName)+"\" /><br/>\
          <label for=\"relaytopic\">Relay topic:</label><br/>\
          <input type=\"text\" id=\"relaytopic\" name=\"relaytopic\" value=\""+String(appConfig.data.relayTopic)+"\" /><br/>\
          <label for=\"relaytopicon\">Relay topic value ON:</label><br/>\
          <input type=\"text\" id=\"relaytopicon\" name=\"relaytopicon\" value=\""+String(appConfig.data.relayTopicOn)+"\" /><br/>\
          <label for=\"relaytopicoff\">Relay topic value OFF:</label><br/>\
          <input type=\"text\" id=\"relaytopicoff\" name=\"relaytopicoff\" value=\""+String(appConfig.data.relayTopicOff)+"\" /><br/>\
        </p>\
        <button type=\"submit\" name=\"action\" value=\"savesettings\">SAVE SETTINGS</button>\
      </form>\
      <br/><br/><br/>\
      <p>Created by <a href=\"https://pavelk.ru\">PavelK.ru</a></p>\
    </body>\
  </html>\
  ";
  
  server.send(200, "text/html", settingsPage);
}

/**
* @brief Отвечаем при запросе хз какой страницы
*/
void handleNotFound() 
{  
  server.send(404, "text/plain", "Page not found =(");  
}

/**
* @brief Запускаем сервер
*/
bool startServer(String ssid, String password)
{  
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP);
  
  IPAddress DeviceIP = IPAddress(192, 168, 1, 1);
  IPAddress Submask = IPAddress(255, 255, 255, 0);
  WiFi.softAPConfig(DeviceIP, DeviceIP, Submask);
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  appConfig.begin();

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  
  server.begin();
  
  Serial.println("HTTP server started!");
  return true;
}

/**
* @brief Останавливаем сервер
*/
void stopServer()
{
  server.stop();
  WiFi.softAPdisconnect(true);
}

/**
* @brief Обрабатываем запросы
*/
void processServerLoop()
{
  server.handleClient();
}
