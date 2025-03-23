#pragma once
#include <AsyncHTTPRequest_Debug_Generic.h>
#include <AsyncHTTPRequest_Generic.h>
#include <AsyncHTTPRequest_Impl_Generic.h>

#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "wifi_utils.h"
#include "geo_utils.h"
//#include "geo_async.h"
#include "display_utils.h"
#include "../icons/weather_icons.h"
#include "request_utils.h"

#define POINT_STOP_WEATHER

#ifdef POINT_STOP_WEATHER
#define pointStop(ms, fmt, ...) { Serial.printf( "[%d] %s ", __LINE__,  __PRETTY_FUNCTION__); Serial.printf(fmt, ## __VA_ARGS__); delay(ms); }
#else
#define pointStop(ms, fmt, ...)
#endif

#define retryWrongUpdateMs 5000UL


//extern const String apiKey;
//extern char * apiKey;
extern GeoLocation::GeoData myLocation;
extern unsigned long weatherUpdateInterval;
#include "ticker.h"
extern SimpleTicker weatherTick;

namespace Weather {

    // enum StateUpdate {
    //     Unknown,
    //     UpdateRunning,
    //     Updated,
    //     FailUpdate,
    // };
    AsyncRequest::State updateState = AsyncRequest::Unknown;

    int hPa2MmHg(const float pressure) {
        return int(pressure * 0.750062);
    };

    const uint8_t* getIconByCode(const char* code) {
        // Сравниваем код и возвращаем указатель на соответствующую иконку
        if (strcmp(code, "01d") == 0) return ::Icons::icon_0_1;
        if (strcmp(code, "01n") == 0) return ::Icons::icon_2_2;
        if (strcmp(code, "02d") == 0) return ::Icons::icon_0_2;
        if (strcmp(code, "02n") == 0) return ::Icons::icon_0_3;
        if (strcmp(code, "03d") == 0) return ::Icons::icon_0_0;
        if (strcmp(code, "03n") == 0) return ::Icons::icon_0_0;
        if (strcmp(code, "04d") == 0) return ::Icons::icon_0_0;
        if (strcmp(code, "04n") == 0) return ::Icons::icon_0_0;
        if (strcmp(code, "09d") == 0) return ::Icons::icon_1_1;
        if (strcmp(code, "09n") == 0) return ::Icons::icon_1_1;
        if (strcmp(code, "10d") == 0) return ::Icons::icon_0_4;
        if (strcmp(code, "10n") == 0) return ::Icons::icon_1_4;
        if (strcmp(code, "11d") == 0) return ::Icons::icon_1_0;
        if (strcmp(code, "11n") == 0) return ::Icons::icon_1_0;
        if (strcmp(code, "13d") == 0) return ::Icons::icon_1_2;
        if (strcmp(code, "13n") == 0) return ::Icons::icon_1_2;
        if (strcmp(code, "50d") == 0) return ::Icons::icon_2_1;
        if (strcmp(code, "50n") == 0) return ::Icons::icon_2_1;

        // Если код не найден, возвращаем nullptr или иконку по умолчанию
        return nullptr;
    };

    struct Data {
        char iconCode[16] = {0};
        char cityName[128] = {0};
        int timeZome = 0;
        float temp = 0, tempFeel = 0;
        float humidity = 0;
        int pressure;
    };

    static Data data;

    const char apiUrl[] PROGMEM = "http://api.openweathermap.org/data/2.5/weather?";

    AsyncHTTPRequest request;

    void onRequestComplete(void* optParm, AsyncHTTPRequest* request, int readyState) {
        if (readyState == readyStateDone) {
            if (request->responseHTTPcode() == 200) {
                updateState = AsyncRequest::SuccessRespond;

                String payload = request->responseText();

                JsonDocument doc;
                auto updated = deserializeJson(doc, payload);

                if (updated.code() == DeserializationError::Ok) {
                     
                    String iconCode = doc["weather"][0]["icon"].as<String>();
                    strcpy(Weather::data.iconCode, iconCode.c_str());

                    Weather::data.temp = doc["main"]["temp"].as<float>();
                    Weather::data.tempFeel = doc["main"]["feels_like"].as<float>();
                    Weather::data.humidity = doc["main"]["humidity"].as<float>();
                    Weather::data.pressure = hPa2MmHg(doc["main"]["pressure"].as<float>());
                    Weather::data.timeZome = doc["timezone"].as<int>();
                    configTime(Weather::data.timeZome, 0, NTP_SERVERS);

                    String _city = doc["name"].as<String>();
                    strcpy(Weather::data.cityName, _city.c_str());

                    Serial.println("Weather data updated:");
                    Serial.println("Description: " + doc["weather"][0]["main"].as<String>());
                    Serial.println("Temperature: " + String(Weather::data.temp) + "C\nFeels like " + String(Weather::data.tempFeel) + "C");
                    Serial.println("Humidity: " + String(Weather::data.humidity) + "%");
                    Serial.println("Pressure: " + String(Weather::data.pressure) + "mmHg");
                    Serial.printf("City name: '%s', tz=%d\n", data.cityName, data.timeZome);
                } else {
                    updateState = AsyncRequest::WrongPayload;
                    Serial.println("Error: updated weather data");
                }
            } else {
                updateState = AsyncRequest::FailRespond;
                Serial.println("Error: HTTP request failed");
            }
            request->setDebug(false);
        }
    };

    String getLang(GeoLocation::GeoData& location) {
        String code;
        if ( location.country[0] != '\0' ) {
            code = String(location.country);
        } else if ( location.countryCode[0] != '\0' ){
            code = String(location.countryCode);
        }
        code.toLowerCase();
        if ( code.equals("russia") || code.equals("ru")) return String("&lang=ru");

        else code.clear();
        return code;
    };

    // bool tryUpdateData(){
    //     if (!WiFi.isConnected() && ! Reconnect::connect() ) return false;
    //     updateData();
    // };
    AsyncRequest::Error updateData();
    static unsigned long startUpdate;
    bool tryUpdateData(){
        static bool updateStarted = false;
        if (!WiFi.isConnected() && ! Reconnect::connect() ) return false;
        if ( WiFi.isConnected() && !updateStarted ) {
            startUpdate = millis();
            updateData();
        }
        if( updateStarted && Reconnect::waitTimeout() )
            return true;
        else return false;
    };

    AsyncRequest::Error updateData() {
        
        //if ( ! Reconnect::connect() ) return AsyncRequest::Error::NoConnection;
        //  {
        //     // WiFi.mode(WIFI_STA);
        //     // delay(1);
        //     // if( WiFi.begin()){
        //     //     pointStop(0, "Wait connection...\n");
        //     //     WiFi.waitForConnectResult(10000);
        //     // } else {
        //     //     pointStop(0, "Can't to begin WiFi\n");
        //     // }
        //     ///connectToWiFi();
        //     if (! Reconnect::connect() ) return AsyncRequest::Error::NoConnection;
            
        // }
        if( !WiFi.isConnected() ) {
            if ( updateState != AsyncRequest::ConnectionWaiting) 
                pointStop(0, "Wait connection...\n");
            updateState = AsyncRequest::ConnectionWaiting;
            return AsyncRequest::Error::WaitConnection;
        }

        if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone) {
            request.setDebug(false);
            request.onReadyStateChange(onRequestComplete);

            String requestUri(apiUrl);

            if (myLocation.valid()) {
                requestUri += "lat=";
                requestUri += myLocation.latitude;
                requestUri += "&lon=";
                requestUri += myLocation.longitude;
                requestUri += getLang(myLocation);
                
            } else {
                requestUri += "q=";
                requestUri += myLocation.city;
                requestUri += ',';
                requestUri += myLocation.country;
            }
            requestUri += "&units=metric&appid=";
            requestUri += apiKey;
            
            request.open("GET", requestUri.c_str());
            if ( request.send() ) {
                pointStop(0,"Request:\n%s\n", requestUri.c_str());
                updateState = AsyncRequest::RespondWaiting;
                return AsyncRequest::OK;
            }
            else {
                pointStop(0,"Wrong Request:\n%s\n", requestUri.c_str());
                updateState = AsyncRequest::State::FailRespond;
                return AsyncRequest::Error::WrongRequest;
            }
        }
        if ( updateState != AsyncRequest::SendedAlready) 
            pointStop(0, "Waiting respode...\n");
        return AsyncRequest::SendedAlready;
    }

    void drawIcon(Adafruit_PCD8544& display, const uint8_t* bitmap, const uint8_t width = 32, const uint8_t height = 32) {
        display.clearDisplay();
        if (bitmap)
            display.drawBitmap(0, 5, bitmap, width, height, PRINT_COLOR);
    }

    void printRightAdjast(Adafruit_PCD8544& display, const String& str, const int textSize = 1, const int16_t _yPos = -999) {
        int16_t yPos;
        if (_yPos == -999) {
            yPos = display.getCursorY();
        } else {
            yPos = _yPos;
        }

        uint charWidth;
        switch (textSize) {
        case 1: charWidth = 6;
            break;
        case 2: charWidth = 12;
            break;
        case 3: charWidth = 18;
            break;
        }
        auto pos = display.width() - (charWidth * (str.length()));
        display.setCursor(pos, yPos);
        display.print(str);
    };

    void printData(Adafruit_PCD8544& display, const Data& data, bool show = false) {
        display.setTextColor(PRINT_COLOR);
        display.setCursor(0, 1);
        display.setTextSize(1);
        if (data.cityName[0] != 0) {
            Display::setFontSize(display, 1);
            display.print(data.cityName);
            Display::setFontSize(display);
        }
        else display.print(myLocation.city);

        display.setTextSize(2);
        String str(data.temp, 0);
        str += 'C';
        Display::printRightAdjast(display, str, 2, 7);

        str = String(data.humidity, 0);
        str += '%';
        Display::printRightAdjast(display, str, 2, 24);

        display.setTextSize(1);
        display.setCursor(0, 40);
        display.print(data.pressure);
        display.print("mmHg");

        str = String(data.tempFeel, 0);
        str += 'C';
        Display::printRightAdjast(display, str);

        if (show) display.display();
    };

    unsigned long wrongUpdateInterval(unsigned int renewMs) {
        return millis() - weatherUpdateInterval + renewMs;
    };

    void printWifiOn(Adafruit_PCD8544& display) {
        display.setCursor(0, 0);
        Display::printRightAdjast(display, String(char(0x24)));
    };

    void update(Adafruit_PCD8544& display, bool wifi = false) {
        auto icon = Weather::getIconByCode(Weather::data.iconCode);
        Weather::drawIcon(display, icon, Icons::width, Icons::height);
        if (wifi) Weather::printWifiOn(display);
        Weather::printData(display, data, true);
    };

}; //namespace