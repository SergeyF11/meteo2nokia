#pragma once

#define NTP_SERVERS "ntp1.stratum2.ru", "ru.pool.ntp.org", "pool.ntp.org"

#include <WiFiManager.h>
#include <ESP8266WiFi.h>

// #include "wifi_icon.h"
#include "eeprom_utils.h"
#include "display_utils.h"
#include "wm_params.h"
#include <MultiResetDetector.h>
#include "ota_utils.h"
#include "AsyncHttpsClient.h"
#include "request_utils.h"
// #include "geo_async.h"
// #include "weather_async.h"
#include "request_utils.h"

// лучше всё это определить в setup
WiFiManager wm;
WiFiManagerParameter openWeatherApiKeyParam; //("apiKey", "OpenWeather API key", apiKey, 40, ""placeholder=\"visit OpenWeather.com for get your Api key\"")" );
WiFiManagerParameter geolocationApiKeyParam;

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

SliderControl *contrD1;
SliderControl *contrD2;

// #define POINT_STOP_WIFI

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

extern const char timeZone[];
extern WiFiManager wm;
const unsigned long connectionTime = 10000UL;
volatile bool isSettingsValid = false;
volatile bool isOpenWeatherKeyValid = false;

MultiResetDetector tripleReset;
extern AsyncHttpsClient httpsClient;

// Глобальные переменные для хранения настроек
extern EepromData eepromSets;
extern AsyncHttpsClient httpsClient;

namespace CaptivePortal
{
  static const char name[] /* PROGMEM */ = "WEatherSTation";

  bool validateOpenWeatherKey(const String &apiKey)
  {
    if (apiKey.isEmpty() || apiKey.length() < 32)
    {
      pointStop(0, "Wrong key length\n");
      return false; // Базовая проверка длины ключа
    }

    // Используем существующий клиент
    if (httpsClient.isBusy())
    {
      pointStop(0, "Https client busy\n");
      return false; // Клиент уже занят
    }

    // Создаем тестовый запрос (аналогично weather_async.h)
    String testUrl(OpenWeaterRequest::uri);
    testUrl += "q=London&appid=";
    testUrl += apiKey;

    bool validationResult = false;
    bool requestCompleted = false;

    pointStop(0, "Send request %s\n", testUrl.c_str());
    httpsClient.get(testUrl,
                    nullptr, // Обработчик заголовков не нужен
                    nullptr, // Обработчик чанков не нужен
                    [&]()
                    {
            // Коллбек успешного завершения
            if (httpsClient.getStatusCode() == 200) {

                String payload = httpsClient.getBody();
                Serial.println( payload);
                
                JsonDocument doc;
                auto err = deserializeJson(doc, payload);
                if ( err.code() == ArduinoJson::DeserializationError::Ok ) {
                 
                  validationResult = doc["weather"].is<JsonArray>(); 

                } else {
                  pointStop(0, "deserialize error: %s\n", err.c_str() );
                }
            } else {
              pointStop(0,"Error request: %d\n", httpsClient.getStatusCode());
            }
            pointStop(0, "Key is %s\n", validationResult ? "valid" : "invalid");
            requestCompleted = true; }, [&requestCompleted](const String &error)
                    {
            // Коллбек ошибки
            requestCompleted = true;
            pointStop(0, "Wrong response\n"); });

    // Ждем завершения запроса с таймаутом

    while (!requestCompleted)
    {
      httpsClient.update();
      delay(10);
    }

    httpsClient.reset(); // Очищаем состояние клиента
    return validationResult;
  }

  void saveParamsCallback()
  {
    pointStop(0, "Start\n");

    bool keyValid = false;
    isSettingsValid = eepromSets.init(
        openWeatherApiKeyParam.getValue(),
        geolocationApiKeyParam.getValue(),
        contrD1->getValue(), contrD2->getValue());

    if (isSettingsValid)
    {
      isOpenWeatherKeyValid = validateOpenWeatherKey(eepromSets.getWeatherKey());
    }

    if (isOpenWeatherKeyValid)
    {
      isSettingsValid = eepromSets.save();
    }
    else
    {
      isSettingsValid = false;
    }

    if (!isSettingsValid)
    {
      Serial.println("error: save eeprom");
      Serial.printf("Len %d, value '%s'\n",
                    strlen(openWeatherApiKeyParam.getValue()),
                    openWeatherApiKeyParam.getValue());
      Serial.printf("Len %d, value '%s', %s",
                    strlen(eepromSets.getWeatherKey()),
                    eepromSets.getWeatherKey(), isOpenWeatherKeyValid ? "valid" : "invalid");
    }
  };

  inline void init(EepromData &loadedData)
  {

    pointStop(0, "Style setted\n");

    wm.setHostname(name);

    wm.setConfigPortalBlocking(false);

    new (&openWeatherApiKeyParam) WiFiManagerParameter(
        "weatherKey", "OpenWeather API key",
        loadedData.getWeatherKey(), API_KEY_SIZE + 1,
        "placeholder=\"получите ключ на OpenWeather.com\""); // visit OpenWeather.com for get your Api key\"" );
    // wm.addParameter(&openWeatherApiKey);

    new (&geolocationApiKeyParam) WiFiManagerParameter(
        "geoKey", "geolocation.io API key (опционально)",
        loadedData.getGeoKey(), API_KEY_SIZE + 1,
        "placeholder=\"для улучшения точности получите ключ на geolocation.io\""); // optional, for greater accuracy visit geolocation.io for get your Api key\"" );

    // Добавляем все параметры в WiFiManager
    wm.addParameter(&openWeatherApiKeyParam);
    wm.addParameter(&geolocationApiKeyParam);

    pointStop(0, "Try to set WiFiManager to sliders\n");

    // Добавляем параметры контраста
    SliderControl::init(wm);

    pointStop(0, "Try to create sliders\n");
    contrD1 = new SliderControl("d1ctr", "дисплей погоды", loadedData.getContrast1(), 30, 90);
    contrD2 = new SliderControl("d2ctr", "дисплей часы/датчик", loadedData.getContrast2(), 30, 90);
    contrD1->setCallback([](uint8_t c)
                         { 
      pointStop(0,"Display 1 set contrast=%u\n", c);
      display1.setContrast(c); });
    contrD2->setCallback([](uint8_t c)
                         { 
      pointStop(0,"Display 2 set contrast=%u\n", c);
      display2.setContrast(c); });

    pointStop(0, "Try to add web CB\n");
    // SliderControl::setupHTTPHandler(wm, {contrD1, contrD2});

    pointStop(0, "Try to add contrast sliders\n");
    wm.addParameter(new SeparatorParameter("<hr><h3>Контраст</h3>"));
    wm.addParameter(new WiFiManagerParameter(contrD1->getHTML()));
    wm.addParameter(new WiFiManagerParameter(contrD2->getHTML()));

    pointStop(0, "Add handler\n");
    // SliderControlI::init(wm, {contrD1, contrD2});
    SliderControl::addWebServerCallback();

    wm.setSaveParamsCallback(saveParamsCallback);

    wm.setTitle(name);

    wm.setConfigPortalTimeout(180);
  };

  void processPortal(Adafruit_PCD8544 *display)
  {
    ArduinoOTA.setHostname(CaptivePortal::name);
    OTA::setup();

    while (true)
    {
      delay(0);
      if (!wm.getConfigPortalActive())
      {
        if (tripleReset.isTriggered())
        {
          tripleReset.clearTrigger();
        }
        wm.startConfigPortal(CaptivePortal::name);
      }

      wm.process();
      OTA::handle();

      // Проверяем условия выхода
      if (WiFi.status() == WL_CONNECTED &&
          isSettingsValid &&
          validateOpenWeatherKey(openWeatherApiKeyParam.getValue()))
      {
        break;
      }

      each(500, printDots(display, WiFi_Icon::_bmp, 2));
    }
  };
}; // namespace CaptivePortal

bool wifiInSleepMode = false;

// hasValidApiKey
namespace Reconnect
{
  class Led
  {
    const uint8_t _pin;
    const bool _onLevel;

  public:
    Led(const uint8_t pin = BUILTIN_LED, bool onLevel = LOW) : _pin(pin), _onLevel(onLevel)
    {
      pinMode(_pin, OUTPUT);
      off();
    };
    void on() const { digitalWrite(_pin, _onLevel); };
    void off() const { digitalWrite(_pin, !_onLevel); };
    bool isOn() const { return digitalRead(_pin) == _onLevel; };
    void toggle() const { digitalWrite(_pin, !digitalRead(_pin)); };
  };
  static Led led;

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

  bool isSaved() { return saved._localIP.isSet(); };
  // bool isValid(){ return saved._localIP.isSet(); };

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
  // static bool needResave = false;
  bool save(const String &name = "", const String &psk = "", bool print = false /*const uint8_t ch, const uint8_t* bssid,
     const IPAddress& ip, const IPAddress& gw, const IPAddress& sub*/
  )
  {
    if (name.isEmpty() || name.length() > 63 ||
        psk.isEmpty() || psk.length() > 63)
      return false;
    strcpy(saved.name, name.c_str());
    strcpy(saved.psk, psk.c_str());
    saved._channel = WiFi.channel();
    memcpy(saved._bssid, WiFi.BSSID(), 6); // Сохраняем BSSID
    saved._localIP = WiFi.localIP();
    saved._gateway = WiFi.gatewayIP();
    saved._subnet = WiFi.subnetMask();
    if (print)
      printTo(Serial);
    return true;
  };

  static unsigned long start;
  static unsigned long timeout = 5000UL;
  bool inline waitTimeout() { return (millis() - start) > timeout; };

  bool connect(unsigned long _timeout = 0, bool reuseSaved = true)
  {
    if (WiFi.isConnected())
      return true;

    if (!WiFi.forceSleepWake() || !WiFi.mode(WIFI_STA))
      return false;
    Reconnect::led.on();
    wifiInSleepMode = false;

    if (_timeout != 0)
      timeout = _timeout;

    pointStop(0, "Start\n");
    start = millis();
    if (reuseSaved){
      return (
          WiFi.config(saved._localIP, saved._gateway, saved._subnet) &&
          WiFi.begin(saved.name, saved.psk, saved._channel, saved._bssid));
    } else { 
      if (WiFi.begin(saved.name, saved.psk))
      {
        return save(saved.name, saved.psk);
      }
      else
      {
        return false;
      }
    }
  };

}; // namespace Reconnect

void connectToWiFi(Adafruit_PCD8544 *display = nullptr)
{
  printDots(display, WiFi_Icon::_bmp, 2);
  Reconnect::led.on();
  // Попытка быстрого подключения к сохраненной сети
  if (!WiFi.isConnected() && Reconnect::isSaved())
  {
    Reconnect::connect();
    while (!WiFi.isConnected() && !Reconnect::waitTimeout())
    {
      delay(10);
      each(500, printDots(display, WiFi_Icon::_bmp, 2));
    }
  }

  // Если быстрое подключение не удалось или нужна настройка
  if (!wm.autoConnect(CaptivePortal::name) || !isSettingsValid || tripleReset.isTriggered())
  {
    CaptivePortal::processPortal(display);
  }

  // Сохраняем параметры подключения
  if (WiFi.isConnected())
  {
    Reconnect::save(WiFi.SSID(), WiFi.psk());
  }
}

extern const unsigned long weatherUpdateInterval;
bool wiFiSleep()
{
  wifiInSleepMode = (WiFi.disconnect(true, false));
  if (wifiInSleepMode)
    pointStop(0, "WiFi disconnected\n");

  wifiInSleepMode &= WiFi.forceSleepBegin(1000UL * weatherUpdateInterval); // Отключить Wi-Fi
  if (wifiInSleepMode)
  {
    Serial.println("WiFi sleeped"); //(0, "WiFi Sleeped\n");
    Reconnect::led.off();
  }
  return wifiInSleepMode;
}