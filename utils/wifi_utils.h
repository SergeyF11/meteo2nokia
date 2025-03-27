#pragma once

#define NTP_SERVERS "ntp1.stratum2.ru", "ru.pool.ntp.org", "pool.ntp.org"

#include <WiFiManager.h>
#include <ESP8266WiFi.h>

// #include "wifi_icon.h"
#include "eeprom_utils.h"
#include "display_utils.h"
#include "wm_params.h"
#include <MultiResetDetector.h>

// лучше всё это определить в setup
WiFiManager wm;
WiFiManagerParameter openWeatherApiKeyParam; //("apiKey", "OpenWeather API key", apiKey, 40, ""placeholder=\"visit OpenWeather.com for get your Api key\"")" );
WiFiManagerParameter geolocationApiKeyParam;


SliderControl *contrD1;
SliderControl *contrD2;

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

extern const char timeZone[];
extern WiFiManager wm;
const unsigned long connectionTime = 10000UL;
volatile bool isSettingsValid = false;

MultiResetDetector tripleReset;

// Глобальные переменные для хранения настроек
extern EepromData set;

namespace CaptivePortal
{
  static const char name[] /* PROGMEM */ = "WEatherSTation";
  static const char contrD1_id[] = "d1contr";
  static const char contrD2_id[] = "d2contr";
  

  void contrastHandler(){
      if (::wm.server->hasArg("id") && ::wm.server->hasArg("value")) {
        String id = ::wm.server->arg("id");
        uint8_t value = ::wm.server->arg("value").toInt();
        
        if (id.equals(contrD1_id)) {
          displays::setContrast( +1, value, &Serial); // знак нужен для правильной перезагрузки функции
        }
        if (id.equals(contrD2_id)) {
          displays::setContrast( +2, value, &Serial);
        }
        ::wm.server->send(200, "text/plain", "OK");
      } else {
        ::wm.server->send(400, "text/plain", "Bad Request");
      }
    }
    void sliderCallback(){
      wm.server->on("/slider", HTTP_GET, contrastHandler);
    }

 void slidersGetHandler(){
    wm.server->on("/slider", HTTP_GET, contrastHandler);
  } 
  void addSliderHandler(WiFiManager& wm, const char* handlerName,
        const char* id1,  
        const char* id2 )
  {

    wm.server->on(handlerName, HTTP_GET, [id1, id2](){
      if (::wm.server->hasArg("id") && ::wm.server->hasArg("value")) {
        String id = ::wm.server->arg("id");
        uint8_t value = ::wm.server->arg("value").toInt();
        
        if (id.equals(id1)) {
          displays::setContrast( +1, value, &Serial); // знак нужен для правильной перезагрузки
        }
        if (id.equals(id2)) {
          displays::setContrast( +2, value, &Serial);
        }
        ::wm.server->send(200, "text/plain", "OK");
      } else {
        ::wm.server->send(400, "text/plain", "Bad Request");
      }
    });
  }

  void saveParamsCallback()
  {

    isSettingsValid = set.init(
                openWeatherApiKeyParam.getValue(),
                geolocationApiKeyParam.getValue(),
                contrD1->getValue(&wm), contrD2->getValue(&wm)) &&
                // contrastD1.getValue(),
                // contrastD2.getValue() ) &&
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

        // addSliderHandler(wm, "/slider", contrD1_id, contrD2_id);
        // pointStop(0, "Handler setted\n");
 

    //wm.setCustomHeadElement(SliderControl::get);
    SliderControl::setupStyle(wm);

    pointStop(0, "Style setted\n");

    wm.setHostname(name);


    wm.setConfigPortalBlocking(false);

  
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

        pointStop(0, "Try add contrast sliders\n");
  // Добавляем параметры контраста
    contrD1 = new SliderControl(contrD1_id, "дисплей погоды", loadedData.getContrast1(), 30, 90 );
    contrD2 = new SliderControl(contrD2_id, "дисплей часы/датчик", loadedData.getContrast2(), 30, 90 );


    // Добавляем все параметры в WiFiManager
    wm.addParameter(&openWeatherApiKeyParam);
    wm.addParameter(&geolocationApiKeyParam);

    wm.addParameter(new SeparatorParameter("<hr><h3>Контраст</h3>"));
    wm.addParameter(new WiFiManagerParameter(contrD1->getHTML()));
    wm.addParameter(new WiFiManagerParameter(contrD2->getHTML()));
    
    wm.setSaveParamsCallback(saveParamsCallback);

    pointStop(0, "Add handler\n");
    
    wm.setWebServerCallback(sliderCallback);
    // contrD1->setCallback([](uint8_t c){ display1.setContrast(c); });
    // contrD2->setCallback([](uint8_t c){ display2.setContrast(c); });
    // wm.setWebServerCallback([](){
    //   wm.server->on("/slider", HTTP_GET, [](){ 
    //     SliderControl::httpHandler(&wm, {contrD1, contrD2});
    //   });
    // });

    wm.setTitle(name);
    
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
