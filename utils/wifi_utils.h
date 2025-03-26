#pragma once

#define NTP_SERVERS "ntp1.stratum2.ru", "ru.pool.ntp.org", "pool.ntp.org"

// #include <WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_PCD8544_multi.h>
// #include "wifi_icon.h"
#include "eeprom_utils.h"
#include "display_utils.h"
#include "wm_params.h"
#include <MultiResetDetector.h>

WiFiManager wm;
WiFiManagerParameter openWeatherApiKeyParam; //("apiKey", "OpenWeather API key", apiKey, 40, ""placeholder=\"visit OpenWeather.com for get your Api key\"")" );
WiFiManagerParameter geolocationApiKeyParam;
// WiFiManagerParameter contrast1Param;
// WiFiManagerParameter contrast2Param;


SeparatorParameter separator("<hr><h3>Контраст</h3>");

SliderParameter contrastD1;
SliderParameter contrastD2;

#define POINT_STOP_WIFI

#ifdef POINT_STOP_WIFI
#define pointStop(ms, fmt, ...)                               \
  {                                                           \
    Serial.printf("[%d] %s ", __LINE__, __PRETTY_FUNCTION__); \
    Serial.printf(fmt, ##__VA_ARGS__);                        \
    delay(ms);                                                \
  }
#else
#define pointStop(ms, fmt, ...)
#endif

// extern const char* ssid;
// extern const char* password;
extern const char timeZone[];
extern WiFiManager wm;
const unsigned long connectionTime = 10000UL;
// extern char * apiKey;
//  char openWeatherApiKeyStr[API_KEY_SIZE+1] = {0};
//  char geolocationApiKeyStr[API_KEY_SIZE+1] = {0};
volatile bool isSettingsValid = false;

MultiResetDetector tripleReset;

// Глобальные переменные для хранения контраста
// extern uint8_t displayContrast1;
// extern uint8_t displayContrast2;
extern EepromData set;

namespace CaptivePortal
{
  static const char name[] /* PROGMEM */ = "WEatherSTation";

  void saveParamsCallback()
  {

    isSettingsValid = set.init(
                openWeatherApiKeyParam.getValue(),
                geolocationApiKeyParam.getValue(),
                contrastD1.getValue(),
                contrastD2.getValue() ) &&
                // atoi(contrast1Param.getValue()),
                // atoi(contrast2Param.getValue()) ) &&
                set.save();

    if ( ! isSettingsValid ){
      Serial.println("error: save eeprom");
      Serial.printf("Len %d, value '%s'\n",
                    strlen(openWeatherApiKeyParam.getValue()),
                    openWeatherApiKeyParam.getValue());
      Serial.printf("Len %d, value '%s', %s",
                    strlen(set.getWeatherKey()),
                    set.getWeatherKey(), set.valid() ? "valid" : "invalid");
    }
  };

  inline void init(EepromData &loadedData)
  {
        // Полная очистка предыдущих параметров
        //wm.resetSettings();
    wm.setCustomHeadElement(SliderParameter::tultip_js);
    wm.setHostname(name);

    wm.setConfigPortalBlocking(false);
    pointStop(0, "Load data\n");
    //char html[1024];
    // snprintf(portalHtml, sizeof(portalHtml), slider_html,
    //          loadedData.getContrast1(), loadedData.getContrast1(),
    //          loadedData.getContrast2(), loadedData.getContrast2());
  

    new (&openWeatherApiKeyParam) WiFiManagerParameter(
        "weatherKey", "OpenWeather API key",
        loadedData.getWeatherKey(), API_KEY_SIZE + 1,
        "placeholder=\"получите ключ на OpenWeather.com\""); // visit OpenWeather.com for get your Api key\"" );
    // wm.addParameter(&openWeatherApiKey);

    new (&geolocationApiKeyParam) WiFiManagerParameter(
        "geoKey", "geolocation.io API key (опционально)",
        loadedData.getGeoKey(), API_KEY_SIZE + 1,
        "placeholder=\"для улучшения точности получите ключ на geolocation.io\""); // optional, for greater accuracy visit geolocation.io for get your Api key\"" );

  // Добавляем параметры контраста
    new (&contrastD1) SliderParameter(
        "contrast1", "дисплей 1",
        loadedData.getContrast1(), 0, 100 ); // "type=\"range\" min=\"0\" max=\"100\" step=\"1\"");

    new (&contrastD2) SliderParameter(
        "contrast2", "дисплей 2",
        loadedData.getContrast2(), 0, 100); //"type=\"range\" min=\"0\" max=\"100\" step=\"1\"");

        
    // Добавляем все параметры в WiFiManager
    wm.addParameter(&openWeatherApiKeyParam);
    wm.addParameter(&geolocationApiKeyParam);
    //wm.setCustomHeadElement("");
    // wm.setCustomHeadElement(portalHtml);
    // wm.addParameter(&contrast1Param);
    // wm.addParameter(&contrast2Param);
    
    wm.addParameter(&separator);

    
    wm.addParameter(&contrastD1);
    wm.addParameter(&contrastD2);

    wm.setSaveParamsCallback(saveParamsCallback);
    
    wm.setTitle("Settings");
    
    wm.setConfigPortalTimeout(180);
  };
};


#define each(ms, func)                       \
  {                                          \
    static unsigned long startMs = millis(); \
    if (millis() - startMs >= (ms))          \
    {                                        \
      startMs = millis();                    \
      {                                      \
        func;                                \
      }                                      \
    }                                        \
  }

// hasValidApiKey
namespace Reconnect
{
  struct Data
  {
    char name[64] = {0};
    char psk[64] = {0};
    uint8_t _channel;
    uint8_t _bssid[6]; // MAC-адрес точки доступа
    IPAddress _localIP;
    IPAddress _gateway;
    IPAddress _subnet;
    // unsigned long start;
  };
  // Параметры соединения (хранятся только в оперативной памяти)
  static Data saved;

  size_t bssidPrintTo(Print &p)
  {
    size_t out = p.print("bssid=");
    for (int i = 0; i < 6; i++)
    {
      if (i)
        out += p.print(':');
      out += p.print(saved._bssid[i]);
    }
    return out;
  };

  size_t printTo(Print &p)
  {
    size_t out = p.printf("Keep WiFi parameters:\nName='%s', psk='%s'\n"
                          "channel=%u, ",
                          saved.name, saved.psk);
    out += bssidPrintTo(p);
    out += p.print(", IP=");
    out += p.print(saved._localIP);
    out += p.print(", gw=");
    out += p.print(saved._gateway);
    out += p.print(", mask=");
    out += p.println(saved._subnet);
    return out;
  };
  static bool needResave = false;
  void save(const String &name = "", const String &psk = "" /*const uint8_t ch, const uint8_t* bssid,
     const IPAddress& ip, const IPAddress& gw, const IPAddress& sub*/
  )
  {
    if (!name.isEmpty())
      strcpy(saved.name, name.c_str());
    if (!psk.isEmpty())
      strcpy(saved.psk, psk.c_str());
    saved._channel = WiFi.channel();
    memcpy(saved._bssid, WiFi.BSSID(), 6); // Сохраняем BSSID
    saved._localIP = WiFi.localIP();
    saved._gateway = WiFi.gatewayIP();
    saved._subnet = WiFi.subnetMask();
    printTo(Serial);
  };

  static unsigned long start;
  static unsigned long timeout = 3000UL;
  bool inline waitTimeout() { return (millis() - start > timeout); };

  bool connect(unsigned long _timeout = 0)
  {
    if (WiFi.isConnected())
      return true;
    if (!WiFi.mode(WIFI_STA))
      return false;
    if (_timeout)
      timeout = _timeout;

    pointStop(0, "Start\n");
    start = millis();
    return (
        WiFi.config(saved._localIP, saved._gateway, saved._subnet) &&
        WiFi.begin(saved.name, saved.psk, saved._channel, saved._bssid));
  };

}; // namespace Reconnect

void connectToWiFi(Adafruit_PCD8544 *display = nullptr)
{


  printDots(display, icon_wifi, 2);
  if (!WiFi.isConnected())
  {
    Reconnect::connect();
    while (!WiFi.isConnected() && !Reconnect::waitTimeout())
    {
      delay(10);
    }
  }

  if (wm.autoConnect(CaptivePortal::name) && isSettingsValid && !tripleReset.isTriggered())
  { //}, "atheration")){
    Serial.println("connected...");
  }
  else
  {
    // needExitConfigPortal = false;
    Serial.println("Config portal running");
    // wm.startConfigPortal(CaptivePortal::name);
    //  if ( ! wm.getConfigPortalActive()  ){
    //    Serial.println("Retart portal on demand");
    //    wm.startConfigPortal(CaptivePortal::name);
    //  }
    //  if ( tripleReset ){
    //    wm.setAPCallback([](WiFiManager* wm) {
    //      tripleReset = MultyReset::reset();
    //      Serial.println("Config portal started");
    //  });
    //  }
    while (!isSettingsValid || WiFi.status() != WL_CONNECTED || tripleReset.isTriggered())
    {
      if (!wm.getConfigPortalActive())
      {
        if (tripleReset.isTriggered())
        {
          static bool resetTriple = false;
          ;
          if (resetTriple)
            tripleReset.clearTrigger();
          else
            resetTriple = tripleReset.isTriggered();
        }
        Serial.println("Retart portal on demand");
        wm.startConfigPortal(CaptivePortal::name);
      }

      if (wm.process())
      {
        pointStop(0, "Status changed\n");
      }
      each(500, printDots(display, icon_wifi, 2));
    }

    pointStop(0, "Key is %s and WiFi is %s\n",
      isSettingsValid ? "valid" : "invalid",
              WiFi.status() == WL_CONNECTED ? "connected" : "not connect");
  }
  if (WiFi.isConnected())
    Reconnect::save(WiFi.SSID(), WiFi.psk());
}
