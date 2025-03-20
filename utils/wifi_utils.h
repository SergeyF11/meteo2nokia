#pragma once

#define NTP_SERVERS "ntp1.stratum2.ru" , "ru.pool.ntp.org", "pool.ntp.org"

#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>
#include "wifi_icon.h"
#include "eeprom_utils.h"

WiFiManager wm;
WiFiManagerParameter openWeatherApiKey; //("apiKey", "OpenWeather API key", apiKey, 40, ""placeholder=\"visit OpenWeather.com for get your Api key\"")" );

//#define POINT_STOP_WIFI
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
  
  }
  