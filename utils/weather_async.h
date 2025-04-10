#pragma once
#include "AsyncHttpsClient.h"
#include <ArduinoJson.h>
#include "wifi_utils.h"
//#include "geo_utils.h"
#include "geo_async.h"
#include "display_utils.h"
#include "../icons/weather_icons.h"
#include "request_utils.h"
#include <partialHash32.h>
#include "ticker.h"

#define POINT_STOP_WEATHER
#ifdef POINT_STOP_WEATHER
#define pointStop(ms, fmt, ...) { Serial.printf( "[%d] %s ", __LINE__,  __PRETTY_FUNCTION__); Serial.printf(fmt, ## __VA_ARGS__); delay(ms); }
#else
#define pointStop(ms, fmt, ...)
#endif

#define retryWrongUpdateMs 5000UL

//extern GeoLocationAsync::GeoData myLocation;

extern const unsigned long weatherUpdateInterval;

extern RefresherTicker weatherTick;
extern AsyncHttpsClient httpsClient;


namespace Weather {
    // const uint8_t* getIconByCode(const char* code) {
    //     switch( codeToKeyR(code)){
    //         case codeToKey("01d"):  return Icons::bmp_01d;
    //         case codeToKey("01n"):  return Icons::bmp_01n;
    //         case codeToKey("02d"):  return Icons::bmp_02d;
    //         case codeToKey("02n"):  return Icons::bmp_02n;
    //         case codeToKey("03d"):  
    //         case codeToKey("03n"):  return Icons::bmp_03d;
    //         case codeToKey("04d"): 
    //         case codeToKey("04n"):  return Icons::bmp_04d;
    //         case codeToKey("09d"):  
    //         case codeToKey("09n"):  return Icons::bmp_09d;
    //         case codeToKey("10d"):  return Icons::bmp_10d;
    //         case codeToKey("10n"):  return Icons::bmp_10n;
    //         case codeToKey("11d"):  
    //         case codeToKey("11n"):  return Icons::bmp_11d;
    //         //case codeToKey("13d"):  
    //         case "13d"_toUint32: 
    //         case codeToKey("13n"):  return Icons::bmp_13d;
    //         case codeToKey("50d"):  
    //         case codeToKey("50n"):  return Icons::bmp_50d;
    //     };
    //     return nullptr;
    // }
    
    const uint8_t* getIconByCode(const char* code) {
        // Icons::Icon getIconByCode(const char* code) {
            auto codeH = Hash32::hash(code);
            switch(codeH){
        #ifdef NEW_BMP
                case "01d"_h:  return Icons::bmp_01d;
                case "01n"_h:  return Icons::bmp_01n;
                case "02d"_h:  return Icons::bmp_02d;
                case "02n"_h:  return Icons::bmp_02n;
                case "03d"_h:  
                case "03n"_h:  return Icons::bmp_03d;
                case "04d"_h: 
                case "04n"_h:  return Icons::bmp_04d;
                case "09d"_h:  
                case "09n"_h:  return Icons::bmp_09d;
                case "10d"_h:  return Icons::bmp_10d;
                case "10n"_h:  return Icons::bmp_10n;
                case "11d"_h:  
                case "11n"_h:  return Icons::bmp_11d;
                case "13d"_h:  
                case "13n"_h:  return Icons::bmp_13d;
                case "50d"_h:  
                case "50n"_h:  return Icons::bmp_50d;
            #else
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
            #endif
            }
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
    

    String getLang(GeoLocationAsync::GeoData& location);

    inline int hPa2MmHg(const float pressure) {
        return int(pressure * 0.750062);
    };

    AsyncRequest::State updateState = AsyncRequest::Idle;

    void onRequestComplete() {
        if (httpsClient.getStatusCode() == 200) {
            updateState = AsyncRequest::SuccessRespond;
            String payload = httpsClient.getBody();
            
            JsonDocument doc;
            auto updated = deserializeJson(doc, payload);

            if (updated.code() == DeserializationError::Ok) {
                Weather::updatedTime = time(nullptr);
                strncpy(Weather::data.iconCode, doc["weather"][0]["icon"], sizeof(data.iconCode));
                Weather::data.temp = doc["main"]["temp"].as<float>();
                Weather::data.tempFeel = doc["main"]["feels_like"].as<float>();
                Weather::data.humidity = doc["main"]["humidity"].as<float>();
                Weather::data.pressure = hPa2MmHg(doc["main"]["pressure"].as<float>());
                Weather::data.timeZone = doc["timezone"].as<int>();
                configTime(Weather::data.timeZone, 0, NTP_SERVERS);

                String _city = doc["name"].as<String>();
                if ( getLang(GeoLocationAsync::myLocation).isEmpty() )
                    strcpy(Weather::data.cityName, _city.c_str());
                else
                    strcpy(Weather::data.cityName, utf8rus(_city).c_str());
                
                const char * desc =  doc["weather"][0]["description"];
                Serial.println("Weather data updated");
                pointStop(0, "\tDescription: %s [%s]\n",  desc, data.iconCode);
                pointStop(0, "\n\tTemperature: %.1fC\n\tFeels like %.1fC\n", Weather::data.temp , Weather::data.tempFeel);
                pointStop(0, "\n\tHumidity: %.0f%%\n\tPressure: %dmmHg\n",Weather::data.humidity, Weather::data.pressure);
                pointStop(0, "\n\tCity name: '%s', tz=%d\n", _city.c_str(), data.timeZone);

            } else {
                updateState = AsyncRequest::WrongPayload;
                pointStop(0,"Error: wrong weather data JSON\n");
            }
        } else {
            updateState = AsyncRequest::FailRespond;
            pointStop(0,"Error: HTTP request failed [%d] %s\n", 
                         httpsClient.getStatusCode(), httpsClient.getError().c_str());
        }
    };

    String getLang(GeoLocationAsync::GeoData& location) {
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


    AsyncRequest::Error updateData() {
        if(!WiFi.isConnected()) return AsyncRequest::NoConnection;

        String requestUri(OpenWeaterRequest::uri);

        if (GeoLocationAsync::myLocation.valid()) {
            requestUri += "lat=";
            requestUri += String(GeoLocationAsync::myLocation.latitude, 5);
            requestUri += "&lon=";
            requestUri += String(GeoLocationAsync::myLocation.longitude, 5);
            requestUri += getLang(GeoLocationAsync::myLocation);
        } else {
            requestUri += "q=";
            requestUri += GeoLocationAsync::myLocation.city;
            requestUri += ',';
            requestUri += GeoLocationAsync::myLocation.country;
        }
        
        requestUri += "&units=metric&appid=";
        requestUri += eepromSets.getWeatherKey();
        pointStop(0,"Request:\n%s\n", requestUri.c_str());

        if (!httpsClient.isBusy()) {
            httpsClient.get(requestUri, 
                // [](int statusCode, const String& headers) {
                //     // Обработка заголовков (если нужно)
                // },
                // [](const String& chunk) {
                //     // Обработка данных по мере поступления (если нужно)
                // },
                nullptr, nullptr,
                onRequestComplete,
                [&](const String& error) {
                    pointStop(0,"Request error: %s\n", error.c_str());
                    updateState = AsyncRequest::FailRespond;
                    httpsClient.reset();
                }
            );
            updateState = AsyncRequest::RespondWaiting;
            return AsyncRequest::OK;
        }
        return AsyncRequest::SendedAlready;
    }

    void drawIcon(Adafruit_PCD8544& display, const uint8_t* bitmap, const uint8_t width = 32, const uint8_t height = 32) {
        display.clearDisplay();
        if (bitmap)
            display.drawBitmap(2, 9, bitmap, width, height, PRINT_COLOR);
    }

    void printRightAdjast(Adafruit_PCD8544& display, const String& str, const uint8_t textSize = 1, const int16_t _yPos = -999) {
        int16_t yPos;
        if (_yPos == -999) {
            yPos = display.getCursorY();
        } else {
            yPos = _yPos;
        }
        const uint charWidth = 6U * textSize;
        // uint charWidth;
        // switch (textSize) {
        // case 1: charWidth = 6;
        //     break;
        // case 2: charWidth = 12;
        //     break;
        // case 3: charWidth = 18;
        //     break;
        // }
        auto pos = display.width() - (charWidth * (str.length()));
        display.setCursor(pos, yPos);
        display.print(str);
    };

    void printData(Adafruit_PCD8544& display, const Data& data, bool show = false) {
        display.setTextColor(PRINT_COLOR);
        display.setCursor(0, 1);
        display.setTextSize(1);

        if ( aproximateLocationAsync ) display.print('~');
        if (data.cityName[0] != 0) {
            display.print(data.cityName);
        }
        else {
            display.print(GeoLocationAsync::myLocation.city);
        }

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

//        if (show) display.display();
    };

    unsigned long wrongUpdateInterval(unsigned int renewMs) {
        auto nowMs = millis();
        auto newSet = nowMs - (weatherUpdateInterval - renewMs);
        pointStop(0,"current=%lu, new set=%lu\n", nowMs, newSet+weatherUpdateInterval);
        return newSet;
    };


    // void printWifiIndicator(Adafruit_PCD8544& display, bool showWifi) {
    //     display.setCursor(0, 0);
    //     if (showWifi) {
    //         Display::printRightAdjast(display, String(char(0xAD))); // WiFi icon
    //     } else {
    //         char buf[6] = {0};
    //         Display::printRightAdjast(display, TimeUtils::toStr(buf, &updatedTime));
    //     }
    // };

    // void printWifiOn(Adafruit_PCD8544& display) {
    //     display.setCursor(0, 0);
    //     Display::printRightAdjast(display, String(char(0xAD))); // антенна
    // };

    void printUpdateStatus(Adafruit_PCD8544& display, bool wifi) {
        display.setCursor(0, 0);
        if ( wifi ) Display::printRightAdjast(display, String(char(0xAD))); // антенна
        else {
            char buf[6] = {0};
            Display::printRightAdjast(display, TimeUtils::toStr(buf, &updatedTime ));//TimeUtils::toString(&updatedTime));
        }
    };

    void update(Adafruit_PCD8544& display, bool wifi = false) {
        auto icon = Weather::getIconByCode(Weather::data.iconCode);
        Weather::drawIcon(display, icon, Icons::width, Icons::height);

        printUpdateStatus(display, wifi);
        Weather::printData(display, data, true);
        display.display();
    };

    // void handleDataUpdate() {
    //     if (weatherTick.tick()) {
    //         if (updateState == AsyncRequest::Idle) {
    //             if (WiFi.isConnected()) {
    //                 if (updateData() == AsyncRequest::OK) {
    //                     updateState = AsyncRequest::RespondWaiting;
    //                 } else {
    //                     updateState = AsyncRequest::FailRespond;
    //                     weatherTick.reset(wrongUpdateInterval(10 SECONDS));
    //                 }
    //             } else {
    //                 updateState = AsyncRequest::WaitWiFiConnection;
    //                 Reconnect::connect(5000);
    //             }
    //         }
    //         httpsClient.update();
    //     }
    // }

    // void handleDisplayUpdate(Adafruit_PCD8544& display) {
    //     switch (updateState) {
    //         case AsyncRequest::WaitWiFiConnection:     
    //         case AsyncRequest::RespondWaiting:
    //             update(display, true); // Show with WiFi indicator
    //             break;
                
    //         case AsyncRequest::SuccessRespond:
    //             //WiFi.disconnect(true, false);
    //             if ( wiFiSleep() ){
    //                 update(display);
    //             }   
    //             updateState = AsyncRequest::Idle;
    //             break;
                
    //         case AsyncRequest::FailRespond:
    //             update(display);
    //             weatherTick.reset(wrongUpdateInterval(5 MINUTES));
    //             updateState = AsyncRequest::Idle;
    //             break;
                
    //         default:
    //             if (weatherTick.refresh()) {
    //                 pointStop(0, "Refresh display\n");
    //                 if( wifiInSleepMode )   
    //                         update(display);
    //                     else
    //                         update(display, true);
    //                 }
    //             //update(display);
    //             // }
    //             // break;
    //     }
    // }
    
    void updateDataDS(Adafruit_PCD8544& display) {
        
        switch (updateState) {
             case AsyncRequest::Idle:
                if ( ! wifiInSleepMode ){
                    wiFiSleep();
                }
                if ( weatherTick.refresh() ) {
                    pointStop(0, "Refresh display\n"); 
                    if( wifiInSleepMode )   
                        update(display);
                    else
                        update(display, true);
                }
                break;
                
            case AsyncRequest::WaitWiFiConnection:
                //update(display, true); // С иконкой WiFi
                if (WiFi.isConnected()) {
                    if (updateData() == AsyncRequest::OK) {
                        updateState = AsyncRequest::RespondWaiting;
                    } else {
                        updateState = AsyncRequest::FailRespond;
                    }
                } else if (Reconnect::waitTimeout()) {
                    pointStop(0, "Timeout reconnect. Try in 60 sec\n");
                    weatherTick.reset( wrongUpdateInterval( 60 SECONDS ) );
                    wiFiSleep();
                    updateState = AsyncRequest::Idle;
                }
                break;
                
            case AsyncRequest::RespondWaiting:
                //update(display, true); // С иконкой WiFi
                break;
                
            case AsyncRequest::SuccessRespond:
                //WiFi.disconnect(true, false);
                //wiFiSleep();
                updateState = AsyncRequest::Idle;
                break;
                
            case AsyncRequest::FailRespond:
                //update(display, false);
                //wiFiSleep();
                pointStop(0, "Fail respond. Try in 5 min\n");
                weatherTick.reset( wrongUpdateInterval( 5 MINUTES ) );
                updateState = AsyncRequest::Idle;

                break;
            //default:
                //break;
                // if ( weatherTick.refresh() ) {
                //     pointStop(0, "Refresh display\n"); 
                //     if( wifiInSleepMode )   
                //         update(display);
                //     else
                //         update(display, true);
                // }
                    // Просто обновляем отображение текущих данных
                 
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
            }
                        //httpsClient.update();
        }
        if( WiFi.isConnected() ) httpsClient.update();
    }


    #ifdef WEATER_TEST
    void test(){
        String url("https://api.openweathermap.org/data/2.5/weather?lat=55.76176&lon=37.86130&lang=ru&units=metric&appid=");
        url += eepromSets.getWeatherKey();
        if (!httpsClient.isBusy()) {
            Serial.println(url);
            
            httpsClient.get(url, 
                [](int statusCode, const String& headers) {
                    // Обработка заголовков (если нужно)
                    Serial.println( statusCode);
                },
                [](const String& chunk) {
                    // Обработка данных по мере поступления (если нужно)
                },
                onRequestComplete,
                [](const String& error) {
                    Serial.printf("Request error: %s\n", error.c_str());
                    updateState = AsyncRequest::FailRespond;
                }
            );
            updateState = AsyncRequest::RespondWaiting;
            //return AsyncRequest::OK;
        }
        ClientState state;
        while(1) {
            if( state == ClientState::COMPLETE ){

                Serial.println("Test done");
                delay(10000);
            } else {
                state = httpsClient.update();
            }

            delay(1);
        }

    }
    #endif
}; // namespace Weather