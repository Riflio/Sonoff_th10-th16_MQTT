#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h> //-- https://github.com/Imroy/pubsubclient
#include <Buttons.h> //-- https://github.com/Riflio/ArduinoButtons
#include "Settings.h"

#define PIN_RELAY 12 //-- RELAY pin (and RED led)
#define PIN_BLUE 13 //-- BLUE led pin, inverted
#define PIN_IN1 14 //-- TEM1 Temperature sensor data 1
#define PIN_IN2 4 //-- TEM1 Temperature sensor data 2
#define PIN_BTN 0 //-- Button S1

#define INTERVAL_CHECK_CONNECTION 5000

bool inSettingsMode = false;

WiFiClient mqttWiFiClient; 
PubSubClient mqttClient(mqttWiFiClient);
Buttons btnS1(PIN_BTN);

//================================================================================================================================

/**
* @brief Мигаем синим светодиодом и задаём заодно ему режим
* @param blinkMode - задаём режим
*/
void blinkBlue(int blinkMode=-1)
{
  static long int lastBlink = 0;
  static int ledModeBlue = -1;
  
  if ( blinkMode>=0 ) {
    if ( ledModeBlue==blinkMode ) return;
    ledModeBlue = blinkMode;
    lastBlink = millis();
  }

  long int passed = millis() - lastBlink;
  
  switch ( ledModeBlue ) {
    case 0: //-- Выключены
      digitalWrite(PIN_BLUE, HIGH);
      break;
    case 1: //-- Моргаем по пол секунды
      if ( passed>1000 ) {                
        lastBlink = millis();
      } else
      if ( passed>500 ) {
        digitalWrite(PIN_BLUE, HIGH);
      } else
      if ( passed>0 ) {
        digitalWrite(PIN_BLUE, LOW);        
      }
      break;
    case 2: //-- Быстро моргаем два раза каждую секунду
      if ( passed>1000 ) {
        lastBlink = millis();
      } else 
      if ( passed>600 ) {
        digitalWrite(PIN_BLUE, HIGH);
      } else 
      if ( passed>400 ) {
        digitalWrite(PIN_BLUE, LOW);        
      } else 
      if ( passed>200 ) {
        digitalWrite(PIN_BLUE, HIGH);        
      } else 
      if ( passed>0 ) {
        digitalWrite(PIN_BLUE, LOW);
      }      
      break;
    case 3: //-- Светимся постоянно
      digitalWrite(PIN_BLUE, LOW);
      break;
    case 4: //-- Быстро моргаем каждый 400 мс
      if ( passed>400 ) {
        lastBlink = millis();
      } else       
      if ( passed>200 ) {
        digitalWrite(PIN_BLUE, HIGH);        
      } else 
      if ( passed>0 ) {
        digitalWrite(PIN_BLUE, LOW);
      }      
      break;
  }
  
}

/*
* @brief Включаем/выключаем рэле
* @param en, 0 - disable, 1 - enable, 2 - switch
*/
void setRelayEn(int en)
{
  static int relayMode = -1;  
  
  if ( en==2 ) { 
    relayMode = (relayMode)? false : true;
  } else {
    if ( relayMode==en ) return;
    relayMode=en;  
  }
    
  digitalWrite(PIN_RELAY, (relayMode)? HIGH : LOW );
}

/**
* @brief От MQTT пришёл какой-то запрос 
*/
void mqttCallback(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();
  String topic = pub.topic();
  
  Serial.print("MQTT subscribe message: ");
  Serial.println(payload);
  Serial.print("Topic:");
  Serial.println(topic);
  
  if( strcmp(topic.c_str(), appConfig.data.relayTopic)==0 ) {
    if ( strcmp(payload.c_str(), appConfig.data.relayTopicOn)==0 ) { setRelayEn(true); }
    else if ( strcmp(payload.c_str(), appConfig.data.relayTopicOff)==0 ) { setRelayEn(false); }
  }
         
}

/*
* @brief Инициализируемся
*/
void setup(void) 
{
  //-- Инициализируем порты
  Serial.begin(9600);
  
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BTN, INPUT);
  pinMode(PIN_IN1, INPUT);
  pinMode(PIN_IN2, INPUT);
  
  blinkBlue(0);
  setRelayEn(false);  

  appConfig.begin();
  
  goWorkMode();
}

/**
* @brief Отдаём статус о подключении к WIFI и подключаемся, если не подключены
* @param setStatus - если нужно принудительно задать статус, например для перподключения, применения новых натсроек и т.д.
* @return
*/
bool wifiConnect(int setStatus=-1)
{
  static int wifiStatus = 0;
  static long int lastCheckConnection = -INTERVAL_CHECK_CONNECTION-1;
  long int passedLastCheck = millis() - lastCheckConnection;

  if ( setStatus>-1 ) { wifiStatus = setStatus; return (setStatus>=2); }
  
  if ( passedLastCheck>INTERVAL_CHECK_CONNECTION ) {
    lastCheckConnection = millis();
    
    if ( wifiStatus==0 ) { //-- Если мы не подключены к WIFI, то пора бы уже
      blinkBlue(1);
      Serial.print("Connecting to ");
      Serial.println(appConfig.data.ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(appConfig.data.ssid, appConfig.data.password);
      wifiStatus=1;
    } else  
    if ( wifiStatus==1 ) { //-- Ожидаем подключения
      if ( WiFi.status() != WL_CONNECTED ) {
        Serial.print(".");      
      } else {          
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        if ( strcmp(appConfig.data.deviceName, "")==0 ) {
          strncpy(appConfig.data.deviceName, WiFi.macAddress().c_str(), 50);
        }
        wifiStatus=2;
        blinkBlue(3);
      }
    } else 
    if ( wifiStatus!=1 && WiFi.status()!=WL_CONNECTED ) { //-- Если были подключены, а теперь нет, то попробуем снова
      wifiStatus = 0;
    }
  }

  return ( wifiStatus==2 && WiFi.status()==WL_CONNECTED )? true : false;
}

/**
* @brief Отдаём статус о подключении к MQTT серверу и подключаемся, если не подключены
* @param setStatus - если нужно принудительно задать статус, например для перподключения, применения новых натсроек и т.д.
* @return
*/
bool mqttConnect(int setStatus=-1)
{
  static int mqttConnectStatus = 0;
  static long int lastCheckMQTTConnection = -INTERVAL_CHECK_CONNECTION-1;
  long int passedLastCheckMQTT = millis() - lastCheckMQTTConnection;

  if ( setStatus>-1 ) { mqttConnectStatus = setStatus; return (setStatus>=2); }
  
  if ( passedLastCheckMQTT>INTERVAL_CHECK_CONNECTION ) {
    lastCheckMQTTConnection = millis();
    
    if ( mqttConnectStatus==0 ) {
      blinkBlue(2);
      Serial.println("Connect to MQTT Server");
      mqttClient.set_server(appConfig.data.mqttServer, appConfig.data.mqttPort);
      mqttConnectStatus = 1;
    } else
    if ( mqttConnectStatus==1 ) {
      if ( mqttClient.connect(MQTT::Connect(appConfig.data.deviceName)
        .set_auth(appConfig.data.mqttUser, appConfig.data.mqttPass)
      )) {
        mqttClient.set_callback(mqttCallback);
        mqttClient.subscribe(appConfig.data.relayTopic);
        blinkBlue(3);
        Serial.println("MQTT Server connected!");
        mqttConnectStatus = 2;
      }
    } else
    if ( mqttConnectStatus!=1 && !mqttClient.connected() ) {
      mqttConnectStatus = 0;
    }
  }
  
  return (mqttConnectStatus==2 && mqttClient.connected())? true : false;
}

/**
* @brief Переходим в рабочий режим
*/
void goWorkMode()
{
  Serial.println("Go WORK mode!");  
  blinkBlue(0);
  stopServer();
  WiFi.disconnect(true);
  inSettingsMode = false;

  wifiConnect(0);
  mqttConnect(0);
}

/**
* @brief Переходим в режим настроек
*/
void goSettingsMode()
{
  if ( inSettingsMode ) return;
  WiFi.disconnect(true);
  Serial.println("Go SETTINGS mode!");
  inSettingsMode = true;
  blinkBlue(4);  
  startServer("Sonoff_settings", "PRIDE1488");
}

/**
* @brief Основной рабочий цикл
*/
void loop(void)
{
  blinkBlue();

  //-- Обрабатываем кнопку
  btnS1.service();
  byte btnState = btnS1.getButton();
  if ( btnState==Buttons::Clicked ) {
    if ( inSettingsMode ) { goWorkMode(); return;} //-- Выходим из режима настроек и переходим в обычный режим
    setRelayEn(2);
  } else
  if ( btnState==Buttons::Held ) { //-- Долгое нажатие, запускаем режим конфигурации    
    goSettingsMode();
  }

  if ( inSettingsMode ) { //-- Если мы в режим настроек
    processServerLoop();
    return; //-- Дальше нам делать нечего
  }

  //-- Проверяем подключения и подключаемся, если нужно
  if ( !wifiConnect() ) return;  
  if ( !mqttConnect() ) return;

  //-- Дальше у нас все подключения есть, можно работать с сервером
  
  mqttClient.loop();

}

//-- Happy End
