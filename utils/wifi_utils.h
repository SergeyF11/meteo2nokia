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
char apiKey[API_KEY_SIZE+1] = {0};
volatile bool hasValidApiKey = false;


extern WiFiManagerParameter openWeatherApiKey;

namespace CaptivePortal {
    void saveParamsCallback () {
    EEEPROM_keyCrc apiKey( openWeatherApiKey.getValue() );
    if ( apiKey.valid() ){
      Serial.println("Get Params:");
      Serial.print(openWeatherApiKey.getID());
      Serial.print(" : ");
      Serial.println(apiKey.get());
      hasValidApiKey = saveApiKey(apiKey );
      if ( hasValidApiKey ) {
        apiKey.copyTo( ::apiKey );
        pointStop(0,"ApiKey saved. %s\n", apiKey);
      }
    // need save to EEPROM
    } else {
      hasValidApiKey = false;
      Serial.println("error: save eeprom");
      Serial.printf( "Len %d, value '%s'\n", 
        strlen(openWeatherApiKey.getValue()), 
        openWeatherApiKey.getValue() );
      Serial.printf( "Len %d, value '%s', %s", 
        strlen( apiKey.get() ), 
        apiKey.get(), apiKey.valid() ? "valid" : "invalid" );
    }
  };

  inline void init(){
    wm.setConfigPortalBlocking(false);
    hasValidApiKey = loadApiKey( apiKey );
    new (&openWeatherApiKey ) WiFiManagerParameter(
      "apiKey", "OpenWeather API key", 
      apiKey, API_KEY_SIZE+1, 
      "placeholder=\"visit OpenWeather.com for get your Api key\"" );
    wm.addParameter(&openWeatherApiKey);
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
}


// void testPrintDots(Adafruit_PCD8544& display, int count ){
//   for ( int i = 0; i<count; i++){
//     printDots(display);
//     delay(200);
//   }
// };

// void connectToWiFiOld(Adafruit_PCD8544* display = nullptr ) {

//     if (WiFi.getPersistent() == true  ) WiFi.persistent(false);//{
//       Serial.print("Connecting to new Wi-Fi");
//       WiFi.mode(WIFI_STA );
//       WiFi.begin(ssid, password);

//     while (WiFi.status() != WL_CONNECTED) {
//       delay(500);
//       printDots(display, icon_wifi, 2);
//       Serial.print('.');
//     } 
//     Serial.println();
  
//   }
  
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
  static Data data;  

    size_t bssidPrintTo(Print& p){
      size_t out= p.print("bssid=");
      for( int i = 0; i<6; i++){
        if ( i ) out += p.print(':');
        out += p.print(data._bssid[i]);
      }
      return out;
    };
  
    size_t printTo(Print& p){
      size_t out = p.printf("Save parameters:\nName='%s', psk='%s'\n" \
        "channel=%u, ", data.name, data.psk );
      out += bssidPrintTo(p);
      out += p.print(", IP="); out += p.print(data._localIP);
      out += p.print(", gw="); out += p.print(data._gateway);
      out += p.print(", mask="); out += p.print(data._subnet);
      return out;
    };
    static bool needResave = false;
    void save( const String& name = "", const String& psk="" /*const uint8_t ch, const uint8_t* bssid, 
      const IPAddress& ip, const IPAddress& gw, const IPAddress& sub*/)
    {
      if ( !name.isEmpty() ) strcpy(data.name, name.c_str());
      if ( !psk.isEmpty() ) strcpy(data.psk, psk.c_str());
      data._channel = WiFi.channel();
      memcpy(data._bssid, WiFi.BSSID(), 6); // Сохраняем BSSID
      data._localIP = WiFi.localIP();
      data._gateway = WiFi.gatewayIP();
      data._subnet = WiFi.subnetMask();
      printTo(Serial);
    }
    bool connect(unsigned long timeout = 10000UL){
      WiFi.mode(WIFI_STA );
      static unsigned long start;
      pointStop(0, "Start\n");
      start = millis();
      //WiFi.config(data._localIP, data._gateway, data._subnet);
      //pointStop(0, "Configured.\n");
      //WiFi.begin(data.name, data.psk, data._channel, data._bssid );
      WiFi.begin(data.name, data.psk);
      pointStop(0, "Begin...\n");
      while(  WiFi.status() != WL_CONNECTED && millis() - start <= timeout){
        delay(0);
        // if ( millis()- start > 3000UL && !needResave ){
        //   pointStop(0, "Try to reconnect without saved params\n");
        //   needResave = true;
        //   WiFi.begin(data.name, data.psk);
        //}
//        each(500, printDots(display, icon_wifi, 2));        
      }
      
      // pointStop(0, "Key is %s and WiFi is %s\n", 
      //   hasValidApiKey ? "valid" : "invalid",
      //   WiFi.status() == WL_CONNECTED ? "connected" : "not connect");
      if ( WiFi.isConnected() ){
        if ( needResave ) { 
          pointStop(0, "Connected with new parameters");
          save();
        } else {
          pointStop(0, "Connected with saved parameters");
        }
      }
      return WiFi.status() == WL_CONNECTED;
    }
    
  };

  void connectToWiFi(Adafruit_PCD8544* display = nullptr ) {
    printDots(display, icon_wifi, 2);
    WiFi.mode(WIFI_STA );
    CaptivePortal::init();

    if( wm.autoConnect("WEatherSTation") && hasValidApiKey ){ //}, "atheration")){
      Serial.println("connected...");
      
    } else {
      //needExitConfigPortal = false;
      if ( ! wm.getConfigPortalActive() && !hasValidApiKey ){
        wm.startConfigPortal("WEatherSTation");
      }
      Serial.println("Config portal running");
      
      while( !hasValidApiKey || WiFi.status() != WL_CONNECTED ){

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
  