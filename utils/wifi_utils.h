#pragma once

#define NTP_SERVERS "ntp1.stratum2.ru" , "ru.pool.ntp.org", "pool.ntp.org"

//#include <WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>
#include "wifi_icon.h"
#include "eeprom_utils.h"

WiFiManager wm;
WiFiManagerParameter openWeatherApiKey; //("apiKey", "OpenWeather API key", apiKey, 40, ""placeholder=\"visit OpenWeather.com for get your Api key\"")" );
WiFiManagerParameter geolocationApiKey; 

#define POINT_STOP_WIFI
#ifdef POINT_STOP_WIFI
#define pointStop(ms, fmt, ...) { Serial.printf( "[%d] %s ", __LINE__,  __PRETTY_FUNCTION__); Serial.printf(fmt, ## __VA_ARGS__); delay(ms); }
#else
#define pointStop(ms, fmt, ...)
#endif

// extern const char* ssid;
// extern const char* password;
extern const char timeZone[];
extern WiFiManager wm;
const unsigned long connectionTime = 10000UL; 
//extern char * apiKey;
char openWeatherApiKeyStr[API_KEY_SIZE+1] = {0};
char geolocationApiKeyStr[API_KEY_SIZE+1] = {0};
volatile bool hasValidApiKey = false;


//extern WiFiManagerParameter openWeatherApiKey;

namespace CaptivePortal {
    static const char name[] /* PROGMEM */= "WEatherSTation";

    void saveParamsCallback () {
    EepromData apiKeys( openWeatherApiKey.getValue(), geolocationApiKey.getValue() );

    if ( apiKeys.valid() ){
      // Serial.printf("Get params: %s:%s, %s", openWeatherApiKey.getID(), apiKeys.getWeatherKey(), 
      //   strlen(geolocationApiKey.getValue()) == 0 ? "" : String(geolocationApiKey.getID()) +":" + apiKeys.getGeoKey());
      apiKeys.printTo(Serial);
      // Serial.println("Get Params:");
      // Serial.print(openWeatherApiKey.getID());
      // Serial.print(" : ");
      // Serial.println(apiKeys.getWeatherKey());
      hasValidApiKey = saveApiKeys(apiKeys );
      if ( hasValidApiKey ) {
        apiKeys.copyWeatherKeyTo( ::openWeatherApiKeyStr );
        apiKeys.copyGeoKeyTo( ::geolocationApiKeyStr );
        pointStop(0,"ApiKey saved.\n");
      }
    // need save to EEPROM
    } else {
      hasValidApiKey = false;
      Serial.println("error: save eeprom");
      Serial.printf( "Len %d, value '%s'\n", 
        strlen(openWeatherApiKey.getValue()), 
        openWeatherApiKey.getValue() );
      Serial.printf( "Len %d, value '%s', %s", 
        strlen( apiKeys.getWeatherKey() ), 
        apiKeys.getWeatherKey(), apiKeys.valid() ? "valid" : "invalid" );
    }
  };

  inline void init(){
    wm.setHostname(name);
    wm.setConfigPortalBlocking(false);
    pointStop(100,"LoadKeys\n");

    hasValidApiKey = loadApiKeys( openWeatherApiKeyStr, geolocationApiKeyStr );
    if ( !hasValidApiKey ) {
      openWeatherApiKeyStr[0] = '\0';
      geolocationApiKeyStr[0] = '\0';
    } 
    new (&openWeatherApiKey ) WiFiManagerParameter(
      "weatherKey", "OpenWeather API key", 
      openWeatherApiKeyStr, API_KEY_SIZE+1, 
      "placeholder=\"visit OpenWeather.com for get your Api key\"" );
    wm.addParameter(&openWeatherApiKey);

    new (&geolocationApiKey ) WiFiManagerParameter(
      "geoKey", "geolocation.io API key", 
      geolocationApiKeyStr, API_KEY_SIZE+1, 
      "placeholder=\"optional, for greater accuracy visit geolocation.io for get your Api key\"" );
    wm.addParameter(&geolocationApiKey);

    wm.setSaveParamsCallback(saveParamsCallback); 
    wm.setConfigPortalTimeout(180);
  };
};

struct FontSize {
  int width;
  int height;
};
FontSize fromSize(const int textSize){
  return FontSize{ 6*textSize, 8*textSize};
};

void printDots(Adafruit_PCD8544& display, const byte *icon = icon_wifi, const int textSize = 2) {

  pointStop(1, "\n");

  static int dots = 0;  // Счетчик точек
  static bool toggle = false;  // Переключатель для анимации
  const char dot = '.';  // Символ точки
  const char space = ' ';  // Символ пробела

  display.clearDisplay();
  display.drawBitmap(10, 1, icon, 64, 52, 1);  // Отрисовка иконки

  display.setTextSize(textSize);  // Установка размера текста
  FontSize fontSize = fromSize(textSize);  // Получение размера шрифта

  // Установка курсора внизу экрана
  display.setCursor(0, display.height() - fontSize.height);

  // Вычисление максимального количества точек, которые поместятся на экране
  int maxDots = display.width() / fontSize.width;

  for (int i = 0; i < maxDots; i++) {
      if (i < dots) {
          display.print(toggle ? space : dot);  // Чередование точек и пробелов
      } else {
          display.print(toggle ? dot : space);  // Чередование точек и пробелов
      }
  }

  // Обновление счетчика и переключателя
  dots++;
  if (dots >= maxDots) {
      dots = 0;
      toggle = !toggle;  // Переключение состояния
  }

  display.display();  // Обновление дисплея
  
};
void inline printDots(Adafruit_PCD8544* display, const byte* icon = icon_wifi, const int textSize = 2) {
  if (display != nullptr) printDots(*display, icon, textSize);
};


  
  #define each( ms, func ) { static unsigned long startMs=millis(); if( millis()- startMs >=(ms)){ startMs=millis(); { func; } } }

 
  // hasValidApiKey
  namespace Reconnect {
    struct Data {        
      char name[64] = {0};
      char psk[64] = {0};  
      uint8_t _channel;
      uint8_t _bssid[6]; // MAC-адрес точки доступа
      IPAddress _localIP;
      IPAddress _gateway;
      IPAddress _subnet;
      //unsigned long start;
    };
// Параметры соединения (хранятся только в оперативной памяти)
  static Data saved;  

    size_t bssidPrintTo(Print& p){
      size_t out= p.print("bssid=");
      for( int i = 0; i<6; i++){
        if ( i ) out += p.print(':');
        out += p.print(saved._bssid[i]);
      }
      return out;
    };
  
    size_t printTo(Print& p){
      size_t out = p.printf("Save parameters:\nName='%s', psk='%s'\n" \
        "channel=%u, ", saved.name, saved.psk );
      out += bssidPrintTo(p);
      out += p.print(", IP="); out += p.print(saved._localIP);
      out += p.print(", gw="); out += p.print(saved._gateway);
      out += p.print(", mask="); out += p.println(saved._subnet);
      return out;
    };
    static bool needResave = false;
    void save( const String& name = "", const String& psk="" /*const uint8_t ch, const uint8_t* bssid, 
      const IPAddress& ip, const IPAddress& gw, const IPAddress& sub*/)
    {
      if ( !name.isEmpty() ) strcpy(saved.name, name.c_str());
      if ( !psk.isEmpty() ) strcpy(saved.psk, psk.c_str());
      saved._channel = WiFi.channel();
      memcpy(saved._bssid, WiFi.BSSID(), 6); // Сохраняем BSSID
      saved._localIP = WiFi.localIP();
      saved._gateway = WiFi.gatewayIP();
      saved._subnet = WiFi.subnetMask();
      printTo(Serial);
    };

    
    static unsigned long start;
    static unsigned long timeout= 3000UL;
    bool inline waitTimeout(){ return (millis() - start > timeout); };

    bool connect(unsigned long _timeout = 0){
      if ( WiFi.isConnected() ) return true;
      if ( ! WiFi.mode(WIFI_STA ) ) return false;
      if ( _timeout ) timeout = _timeout;

      pointStop(0, "Start\n");
      start = millis();
      return (
        WiFi.config(saved._localIP, saved._gateway, saved._subnet) &&
        WiFi.begin(saved.name, saved.psk, saved._channel, saved._bssid ) 
      );
    };
    
  }; //namespace Reconnect

  #define SKIP_INIT true
  void connectToWiFi(Adafruit_PCD8544* display = nullptr , bool skipInit=false) {
    printDots(display, icon_wifi, 2);
    if ( ! WiFi.isConnected() ) { 
      Reconnect::connect();
      while( !WiFi.isConnected() && ! Reconnect::waitTimeout() ){
        delay (10);
      }    
    }
    if ( !skipInit){
      CaptivePortal::init();
    }

    if(  wm.autoConnect(CaptivePortal::name) && hasValidApiKey ){ //}, "atheration")){
      Serial.println("connected..."); 
    } else {
      //needExitConfigPortal = false; 
      Serial.println("Config portal running");
      
      while( !hasValidApiKey || WiFi.status() != WL_CONNECTED ){
        if ( ! wm.getConfigPortalActive()  ){
          Serial.println("Retart portal for wrong data");
          wm.startConfigPortal(CaptivePortal::name);
        }
        if ( wm.process() ){
          pointStop(0, "Status changed\n");
        }
        each(500, printDots(display, icon_wifi, 2));        
      }
      
      pointStop(0, "Key is %s and WiFi is %s\n", 
        hasValidApiKey ? "valid" : "invalid",
        WiFi.status() == WL_CONNECTED ? "connected" : "not connect");
    }
    if( WiFi.isConnected() ) Reconnect::save(WiFi.SSID(),  WiFi.psk());
  }
  