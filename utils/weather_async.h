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
#include <partialHash32.h>


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
extern RefresherTicker weatherTick;

namespace Weather {
    // struct IconPtr {
    //     uint8_t code;
    //     bool isDay;
    // };
    // //const uint
    // constexpr IconPtr Decode(const char * codeStr){
    //     constexpr uint8_t code = (10*codeStr[0]-'0') + (codeStr[1]-'0');
    //     return (codeStr[2] == 'd' ) ? IconPtr{code, true} : IconPtr{code, false};  
    // };
    // constexpr IconPtr operator "" _decode(const char *str){
    //     return Decode(str);
    // }

    String getLang(GeoLocation::GeoData& location);

    AsyncRequest::State updateState = AsyncRequest::Idle;

    int hPa2MmHg(const float pressure) {
        return int(pressure * 0.750062);
    };

    // const IconPtr decode(const char * codeStr){
    //     IconPtr ptr;
    //     char dayNight = 0;
    //     sscanf("%02u%c", codeStr, &ptr.code, &dayNight);
    //     ptr.isDay = ( dayNight == 'd' );
    //     return ptr;
    // };

    // const uint8_t code2code(const char * codeStr){
    //     auto ptr = decode(codeStr);
    //     return ptr.isDay ? ptr.code : 100U+ptr.code;
    // };
    // Icons::Icon getIconByCodeStr(const char* code){
    //     auto ptr = decode(code);
    //     switch(ptr.code){
    //         case "01d"_decode.code: return Icons::icon_0_1;
    //         case "01n"_decode.code: return Icons::icon_2_2;
    //     }
    // };

    const uint8_t* getIconByCode(const char* code) {
    // Icons::Icon getIconByCode(const char* code) {
        auto codeH = Hash32::hash(code);
        switch(codeH){
            case "01d"_h:  return Icons::icon_0_1;
            case "01n"_h:  return Icons::icon_2_2;
            case "02d"_h:  return Icons::icon_0_2;
            case "02n"_h:  return Icons::icon_0_3;
            case "03d"_h:  return Icons::icon_0_0;
            case "03n"_h:  return Icons::icon_0_0;
            case "04d"_h:  return Icons::icon_0_0;
            case "04n"_h:  return Icons::icon_0_0;
            case "09d"_h:  return Icons::icon_1_1;
            case "09n"_h:  return Icons::icon_1_1;
            case "10d"_h:  return Icons::icon_0_4;
            case "10n"_h:  return Icons::icon_1_4;
            case "11d"_h:  return Icons::icon_1_0;
            case "11n"_h:  return Icons::icon_1_0;
            case "13d"_h:  return Icons::icon_1_2;
            case "13n"_h:  return Icons::icon_1_2;
            case "50d"_h:  return Icons::icon_2_1;
            case "50n"_h:  return Icons::icon_2_1;
        }
        // Сравниваем код и возвращаем указатель на соответствующую иконку
        // if (strcmp(code, "01d") == 0) return Icons::icon_0_1;
        // if (strcmp(code, "01n") == 0) return Icons::icon_2_2;
        // if (strcmp(code, "02d") == 0) return Icons::icon_0_2;
        // if (strcmp(code, "02n") == 0) return Icons::icon_0_3;
        // if (strcmp(code, "03d") == 0) return Icons::icon_0_0;
        // if (strcmp(code, "03n") == 0) return Icons::icon_0_0;
        // if (strcmp(code, "04d") == 0) return Icons::icon_0_0;
        // if (strcmp(code, "04n") == 0) return Icons::icon_0_0;
        // if (strcmp(code, "09d") == 0) return Icons::icon_1_1;
        // if (strcmp(code, "09n") == 0) return Icons::icon_1_1;
        // if (strcmp(code, "10d") == 0) return Icons::icon_0_4;
        // if (strcmp(code, "10n") == 0) return Icons::icon_1_4;
        // if (strcmp(code, "11d") == 0) return Icons::icon_1_0;
        // if (strcmp(code, "11n") == 0) return Icons::icon_1_0;
        // if (strcmp(code, "13d") == 0) return Icons::icon_1_2;
        // if (strcmp(code, "13n") == 0) return Icons::icon_1_2;
        // if (strcmp(code, "50d") == 0) return Icons::icon_2_1;
        // if (strcmp(code, "50n") == 0) return Icons::icon_2_1;

        // Если код не найден, возвращаем nullptr или иконку по умолчанию
        return nullptr;
    };

    struct Data {
        char iconCode[4] = {0};
        char cityName[128] = {0};
        int timeZone = 0;
        float temp = 0, tempFeel = 0;
        float humidity = 0;
        int pressure;
    };

    static Data data;
    static time_t updatedTime = 0;

    const char apiUrl[] PROGMEM = "http://api.openweathermap.org/data/2.5/weather?";

    AsyncHTTPRequest request;

    void onRequestComplete(void* optParm, AsyncHTTPRequest* request, int readyState) {
        if (readyState == readyStateDone) {
            if (request->responseHTTPcode() == 200) {
                updateState = AsyncRequest::SuccessRespond;
                if ( request->respHeaderExists("Date")){
                    TimeUtils::setGMTTime( request->respHeaderValue("Date"));
                    pointStop(0, "Set time %s\n", request->respHeaderValue("Date") );
                }
                String payload = request->responseText();

                JsonDocument doc;
                auto updated = deserializeJson(doc, payload);

                if (updated.code() == DeserializationError::Ok) {
                    Weather::updatedTime = time(nullptr);
                    // String iconCode = doc["weather"][0]["icon"].as<String>();
                    // strcpy(Weather::data.iconCode, iconCode.c_str());
                    strncpy(Weather::data.iconCode, doc["weather"][0]["icon"], sizeof(data.iconCode));
                    Weather::data.temp = doc["main"]["temp"].as<float>();
                    Weather::data.tempFeel = doc["main"]["feels_like"].as<float>();
                    Weather::data.humidity = doc["main"]["humidity"].as<float>();
                    Weather::data.pressure = hPa2MmHg(doc["main"]["pressure"].as<float>());
                    Weather::data.timeZone = doc["timezone"].as<int>();
                    configTime(Weather::data.timeZone, 0, NTP_SERVERS);

                    String _city = doc["name"].as<String>();
                    //strcpy(Weather::data.cityName, _city.c_str());
                    if ( getLang(myLocation).isEmpty() )
                        strcpy(Weather::data.cityName, _city.c_str());
                    else
                        strcpy(Weather::data.cityName, utf8rus(_city).c_str());

                    Serial.println("Weather data updated:");
                    Serial.println("Description: " + doc["weather"][0]["main"].as<String>());
                    Serial.println("Temperature: " + String(Weather::data.temp) + "C\nFeels like " + String(Weather::data.tempFeel) + "C");
                    Serial.println("Humidity: " + String(Weather::data.humidity) + "%");
                    Serial.println("Pressure: " + String(Weather::data.pressure) + "mmHg");
                    Serial.printf("City name: '%s', tz=%d\n", _city.c_str(), data.timeZone);
                } else {
                    updateState = AsyncRequest::WrongPayload;
                    Serial.println("Error: wrong weather data JSON");
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

    bool waitConnection = false;
    AsyncRequest::Error updateData() {

        if( !WiFi.isConnected() ) return AsyncRequest::NoConnection;

         
        String requestUri(apiUrl);

        if (myLocation.valid()) {
            requestUri += "lat=";
            requestUri += String(myLocation.latitude, 5);
            requestUri += "&lon=";
            requestUri += String(myLocation.longitude, 5);
            requestUri += getLang(myLocation);
            
        } else {
            requestUri += "q=";
            requestUri += myLocation.city;
            requestUri += ',';
            requestUri += myLocation.country;
        }
        requestUri += "&units=metric&appid=";
        requestUri += set.getWeatherKey();
        pointStop(0,"Request:\n%s\n", requestUri.c_str());

        if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone) {
            request.setDebug(true);
            request.onReadyStateChange(onRequestComplete);
            request.open("GET", requestUri.c_str());
            if ( request.send() ) {
                updateState = AsyncRequest::RespondWaiting;
                return AsyncRequest::OK;
            } else { 
                pointStop(0,"Error: [%d] %s\n", request.responseHTTPcode(), request.responseHTTPString());   
                return AsyncRequest::Error::WrongRequest; 
            }
        }

        return AsyncRequest::SendedAlready;
    }

    void drawIcon(Adafruit_PCD8544& display, const uint8_t* bitmap, const uint8_t width = 32, const uint8_t height = 32) {
        display.clearDisplay();
        if (bitmap)
            display.drawBitmap(0, 7, bitmap, width, height, PRINT_COLOR);
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
            //Display::setFontSize(display, 1);
            if ( aproximateLocation ) display.print('~');
            display.print(data.cityName);
            //Display::setFontSize(display);
        }
        else display.print(myLocation.city);

        display.setTextSize(2);
        String str(data.temp, 0);
        str += 'C';
        Display::printRightAdjast(display, str, 2, 8);

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
        auto nowMs = millis();
        auto newSet = nowMs - (weatherUpdateInterval - renewMs);
        pointStop(0,"current=%lu, new set=%lu\n", nowMs, newSet);
        return newSet;
    };

    void printWifiOn(Adafruit_PCD8544& display) {
        display.setCursor(0, 0);
        Display::printRightAdjast(display, String(char(0xAD))); // антенна
    };

    void printUpdateStatus(Adafruit_PCD8544& display, bool wifi) {
        display.setCursor(0, 0);
        if ( wifi ) Display::printRightAdjast(display, String(char(0xAD))); // антенна
        else {
            // String updatedTimeS;
            // auto ut = localtime( &updatedTime);
            // updatedTimeS += ut->tm_hour;
            // updatedTimeS += ':';
            // updatedTimeS += ut->tm_min;
            // Display::printRightAdjast(display, updatedTimeS);
            char buf[6] = {0};
            Display::printRightAdjast(display, TimeUtils::toStr(buf, &updatedTime ));//TimeUtils::toString(&updatedTime));
        }
    };

    void update(Adafruit_PCD8544& display, bool wifi = false) {
        auto icon = Weather::getIconByCode(Weather::data.iconCode);
        Weather::drawIcon(display, icon, Icons::width, Icons::height);
        //if (wifi) Weather::printWifiOn(display);
        printUpdateStatus(display, wifi);
        Weather::printData(display, data, true);
    };

    void updateDataMy(Adafruit_PCD8544& display){

    switch ( updateState ){
        case AsyncRequest::Idle:
          break;
        
        case AsyncRequest::WaitWiFiConnection:
          update(display, true);
          if( WiFi.isConnected() ) {
            waitConnection = false;
            if( updateData() != AsyncRequest::Error::OK ){
              weatherTick.reset( wrongUpdateInterval( 10 SECONDS ) );
              //waitConnection = true;
              updateState = AsyncRequest::State::FailRespond;
              Serial.println("Fail responde");
              break;
            }
          } else if ( Reconnect::waitTimeout() ) {
              waitConnection = false;
              weatherTick.reset( wrongUpdateInterval( 60 SECONDS ) );
              updateState = AsyncRequest::State::FailRespond;
          } 
          //if ( waitConnection)
          // ответа ещё нет !!!
          updateState = AsyncRequest::State::RespondWaiting;
          break;

        case AsyncRequest::State::FailRespond:
        case AsyncRequest::State::SuccessRespond:
          WiFi.disconnect(true,false);
          delay(1);
          //WiFi.mode(WIFI_OFF);
          Serial.println("WiFi disconnect and off");
          update(display);
          updateState = AsyncRequest::State::Idle;
          break;
        case AsyncRequest::State::RespondWaiting:
          update(display, true);
          //updateState = AsyncRequest::State::Unknown;
          break;
        default:
        //  case Weather::FailUpdate:
          update(display, true); //nowTm->tm_sec == 0);
          break;
      }
    };

    void updateDataDS(Adafruit_PCD8544& display) {
        switch (updateState) {
            // case AsyncRequest::Idle:
            // if ( weatherTick.refresh() ) update.display();
            //     // Просто отображаем текущие данные
            //     //update(display);
            //     break;
                
            case AsyncRequest::WaitWiFiConnection:
                update(display, true); // С иконкой WiFi
                if (WiFi.isConnected()) {
                    if (updateData() == AsyncRequest::OK) {
                        updateState = AsyncRequest::RespondWaiting;
                    } else {
                        updateState = AsyncRequest::FailRespond;
                    }
                } else if (Reconnect::waitTimeout()) {
                    
                    weatherTick.reset( wrongUpdateInterval( 60 SECONDS ) );
                    updateState = AsyncRequest::FailRespond;
                }
                break;
                
            case AsyncRequest::RespondWaiting:
                update(display, true); // С иконкой WiFi
                break;
                
            case AsyncRequest::SuccessRespond:
                WiFi.disconnect(true, false);
                update(display);
                updateState = AsyncRequest::Idle;
                break;
                
            case AsyncRequest::FailRespond:
                update(display, false);
                weatherTick.reset( wrongUpdateInterval((5U*60*1000)));
                updateState = AsyncRequest::Idle;
//                 if (weatherTick.tick()) {
// //////           надо проверять
//                     updateState = AsyncRequest::Idle;
//                     weatherTick.reset(-weatherUpdateInterval);

//                 }
                break;
            default:
                if ( weatherTick.refresh() ) update(display);
                    // Просто отображаем текущие данные
                 
        }
    };

    void handleTick() {
        if (weatherTick.tick()) {
            if (updateState == AsyncRequest::Idle) {
                if (WiFi.isConnected()) {
                    if (updateData() == AsyncRequest::OK) {
                        updateState = AsyncRequest::RespondWaiting;
                    } else {
                        updateState = AsyncRequest::FailRespond;
                        weatherTick.reset(wrongUpdateInterval(10 SECONDS));
                    }
                } else {
                    updateState = AsyncRequest::WaitWiFiConnection;
                    Reconnect::connect(5000);
                }
            } else {    // not Idle ?

            }
        }
    }
}; //namespace