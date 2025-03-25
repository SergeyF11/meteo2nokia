#pragma once

#define NTP_SERVERS "ntp1.stratum2.ru" , "ru.pool.ntp.org", "pool.ntp.org"

//#include <WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>
#include "wifi_icon.h"
#include "eeprom_utils.h"
//#include "rtc_utils.h"
#include "html.h"
#include <MultiResetDetector.h>


WiFiManager wm;
WiFiManagerParameter openWeatherApiKey; //("apiKey", "OpenWeather API key", apiKey, 40, ""placeholder=\"visit OpenWeather.com for get your Api key\"")" );
WiFiManagerParameter geolocationApiKey; 
WiFiManagerParameter contrast1_param; 
WiFiManagerParameter contrast2_param; 


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

MultiResetDetector tripleReset;

// Глобальные переменные для хранения контраста
extern uint8_t displayContrast1;
extern uint8_t displayContrast2;


namespace CaptivePortal {
    static const char name[] /* PROGMEM */= "WEatherSTation";

    void saveParamsCallback () {
    
    // Получаем значения контраста
    displayContrast1 = atoi(contrast1_param.getValue());
    displayContrast2 = atoi(contrast2_param.getValue());
    
    
    EepromData eepromData( openWeatherApiKey.getValue(), geolocationApiKey.getValue(), displayContrast1, displayContrast2 );
    if ( eepromData.valid() && strlen(eepromData.getWeatherKey()) != 0 ){
      // Serial.printf("Get params: %s:%s, %s", openWeatherApiKey.getID(), apiKeys.getWeatherKey(), 
      //   strlen(geolocationApiKey.getValue()) == 0 ? "" : String(geolocationApiKey.getID()) +":" + apiKeys.getGeoKey());
      // apiKeys.printTo(Serial);
      // Serial.println("Get Params:");
      // Serial.print(openWeatherApiKey.getID());
      // Serial.print(" : ");
      // Serial.println(apiKeys.getWeatherKey());
      //hasValidApiKey = saveApiKeys(apiKeys );
      hasValidApiKey = saveEepromData(eepromData); 
      if ( hasValidApiKey ) {
        Serial.println("Save keys:"); eepromData.printTo(Serial);

        eepromData.copyWeatherKeyTo( ::openWeatherApiKeyStr );
        eepromData.copyGeoKeyTo( ::geolocationApiKeyStr );
        pointStop(0,"eeprom data saved.\n");
      }
    // need save to EEPROM
    } else {
      hasValidApiKey = false;
      Serial.println("error: save eeprom");
      Serial.printf( "Len %d, value '%s'\n", 
        strlen(openWeatherApiKey.getValue()), 
        openWeatherApiKey.getValue() );
      Serial.printf( "Len %d, value '%s', %s", 
        strlen( eepromData.getWeatherKey() ), 
        eepromData.getWeatherKey(), eepromData.valid() ? "valid" : "invalid" );
    }
  };

  inline void init(EepromData& loadedData ){
    wm.setHostname(name);
    wm.setConfigPortalBlocking(false);
    pointStop(100,"Load data\n");
    // EepromData loadedData;
    // hasValidApiKey = loadEepromData( loadedData  );  //loadApiKeys( openWeatherApiKeyStr, geolocationApiKeyStr );
    
    // if ( !hasValidApiKey ) {
    //   openWeatherApiKeyStr[0] = '\0';
    //   geolocationApiKeyStr[0] = '\0';
    // } 
    new (&openWeatherApiKey ) WiFiManagerParameter(
      "weatherKey", "OpenWeather API key", 
      loadedData.getWeatherKey() , API_KEY_SIZE+1, 
      "placeholder=\"получите ключ на OpenWeather.com\"" ); //visit OpenWeather.com for get your Api key\"" );
    //wm.addParameter(&openWeatherApiKey);

    new (&geolocationApiKey ) WiFiManagerParameter(
      "geoKey", "geolocation.io API key", 
      loadedData.getGeoKey(), API_KEY_SIZE+1, 
      "placeholder=\"для улучшения точности получите ключ на geolocation.io\""); //optional, for greater accuracy visit geolocation.io for get your Api key\"" );
  //  wm.addParameter(&geolocationApiKey);
  // Добавляем параметры контраста
  char contrast1_str[4], contrast2_str[4];
  snprintf(contrast1_str, 4, "%d", displayContrast1);
  snprintf(contrast2_str, 4, "%d", displayContrast2);

  new (&contrast1_param) WiFiManagerParameter(
      "contrast1", "Контраст дисплея 1 (0-100)", 
      contrast1_str, 4, "type=\"range\" min=\"0\" max=\"100\" step=\"1\"");

  new (&contrast2_param) WiFiManagerParameter(
      "contrast2", "Контраст дисплея 2 (0-100)", 
      contrast2_str, 4, "type=\"range\" min=\"0\" max=\"100\" step=\"1\"");

  // Добавляем все параметры в WiFiManager
  wm.addParameter(&openWeatherApiKey);
  wm.addParameter(&geolocationApiKey);
  wm.addParameter(&contrast1_param);
  wm.addParameter(&contrast2_param);

    wm.setSaveParamsCallback(saveParamsCallback); 
    wm.setTitle("WEatherSTation");
    
    wm.setConfigPortalTimeout(180);
    
  };
};

// Функция для применения настроек контраста к дисплеям
void applyDisplayContrast(Adafruit_PCD8544& display1, Adafruit_PCD8544& display2) {
  display1.setContrast(displayContrast1);
  display2.setContrast(displayContrast2);
  Serial.printf("Applied contrast: Display1=%d, Display2=%d\n", displayContrast1, displayContrast2);
}

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
      size_t out = p.printf("Save WiFi parameters:\nName='%s', psk='%s'\n" \
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
    char html[1024];
    snprintf(html, sizeof(html), slider_html, 
             displayContrast1, displayContrast1,
             displayContrast2, displayContrast2);
    
    wm.setCustomHeadElement(html);
    
    printDots(display, icon_wifi, 2);
    if ( ! WiFi.isConnected() ) { 
      Reconnect::connect();
      while( !WiFi.isConnected() && ! Reconnect::waitTimeout() ){
        delay (10);
      }    
    }
    if ( !skipInit){
      //CaptivePortal::init();
    }

    if(  wm.autoConnect(CaptivePortal::name) && hasValidApiKey && !tripleReset.isTriggered() ){ //}, "atheration")){
      Serial.println("connected..."); 
    } else {
      //needExitConfigPortal = false; 
      Serial.println("Config portal running");
      //wm.startConfigPortal(CaptivePortal::name);
      // if ( ! wm.getConfigPortalActive()  ){
      //   Serial.println("Retart portal on demand");
      //   wm.startConfigPortal(CaptivePortal::name);
      // }
      // if ( tripleReset ){
      //   wm.setAPCallback([](WiFiManager* wm) {
      //     tripleReset = MultyReset::reset();
      //     Serial.println("Config portal started");
      // });
      // }
      while( !hasValidApiKey || WiFi.status() != WL_CONNECTED || tripleReset.isTriggered() ){
        if ( ! wm.getConfigPortalActive()  ){
          if ( tripleReset.isTriggered() ){
            static bool resetTriple = false;;
            if ( resetTriple ) tripleReset.clearTrigger();
            else resetTriple = tripleReset.isTriggered();
          }
          Serial.println("Retart portal on demand");
          wm.startConfigPortal(CaptivePortal::name) ;  
          
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
  