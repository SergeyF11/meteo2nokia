#pragma once

#include "AsyncHttpsClient.h"
#include <ArduinoJson.h>
#include "time_utils.h"
#include "wifi_utils.h"

#define TX_OFFSET_INVALID 99999

bool aproximateLocationAsync = true;
const unsigned long GEO_RETRY_INTERVAL = 5 MINUTES;

namespace RequestGeoAsync {
    enum Error {
        OK,
        NoConnection,
        NoResponse,
        NoData,
        ErrorData,
        Pending
    };
}

namespace GeoLocationAsync {
    static const size_t CountrySize = 32;
    static const size_t CitySize = 64;
    static const size_t TZSize = 48;
    static const size_t CountryCodeSize = 3;
    
    struct GeoData {
        char country[CountrySize] = {0};
        char city[CitySize] = {0};
        char timeZone[TZSize] = {0};
        char countryCode[CountryCodeSize] = {0};
        int tzOffset = TX_OFFSET_INVALID;
        float latitude;
        float longitude;
        time_t unixTime;
        
        bool valid() { return city[0] != '\0'; }
        bool validOffset() { return tzOffset != TX_OFFSET_INVALID; }

        size_t printTo(Print &p)
        {
            size_t printed = p.printf("CountryCode: %s, City: %s, Lat:%f, Long:%f\nTimeZone:%s\n",
                                      countryCode, city, latitude, longitude, timeZone);
            if ( validOffset() ) printed += p.printf("Tz offset = %d\n", tzOffset);
            return printed;
        };
    };

    // class GeoRequest {
    // private:
    //     AsyncHttpsClient httpsClient;
    //     GeoData* targetData;
    //     bool ipGeoCompleted = false;
    //     bool cloudflareCompleted = false;
    //     bool requestInProgress = false;
    //     bool useIpGeoService = true; // Начинаем с IPGeolocation
    //     unsigned long lastRetryTime = 0;
    //     const char* geoKey;

    //     void parseIpGeoResponse(const String& response) {
    //         JsonDocument doc;
    //         deserializeJson(doc, response);
            
    //         strlcpy(targetData->country, doc["country_name"], CountrySize);
    //         strlcpy(targetData->city, doc["city"], CitySize);
    //         strlcpy(targetData->countryCode, doc["country_code2"], CountryCodeSize);
    //         strlcpy(targetData->timeZone, doc["time_zone"]["name"], TZSize);
            
    //         targetData->tzOffset = doc["time_zone"]["offset_with_dst"] | TX_OFFSET_INVALID;
    //         targetData->latitude = doc["latitude"];
    //         targetData->longitude = doc["longitude"];
            
    //         float unixTime = doc["time_zone"]["current_time_unix"];
    //         if (!TimeUtils::isSynced() && unixTime != 0.0) {
    //             targetData->unixTime = static_cast<time_t>(unixTime);
    //             TimeUtils::setGMTTime(targetData->unixTime);
    //         }

    //         ipGeoCompleted = true;
    //         aproximateLocationAsync = false;
    //         requestInProgress = false;
    //     }

    //     void parseCloudflareResponse(const String& response) {
    //         JsonDocument doc;
    //         deserializeJson(doc, response);
            
    //         strlcpy(targetData->country, doc["country"], CountrySize);
    //         strlcpy(targetData->city, doc["city"], CitySize);
    //         targetData->latitude = doc["latitude"];
    //         targetData->longitude = doc["longitude"];

    //         cloudflareCompleted = true;
    //         requestInProgress = false;
    //     }

    //     void sendNextRequest() {
    //         if (useIpGeoService) {
    //             String url = "https://api.ipgeolocation.io/ipgeo?apiKey=" + String(geoKey);
    //             //Serial.println("Sending IPGeo request: " + url);
                
    //             httpsClient.get(url, 
    //                 nullptr,
    //                 nullptr,
    //                 [this]() {
    //                     if (httpsClient.getStatusCode() == 200) {
    //                         parseIpGeoResponse(httpsClient.getBody());
    //                     } else {
    //                         Serial.printf("IPGeo request failed: %d\n", httpsClient.getStatusCode());
    //                         useIpGeoService = false; // Переключаемся на Cloudflare
    //                         sendNextRequest();
    //                     }
    //                 },
    //                 [this](const String& error) {
    //                     Serial.println("IPGeo error: " + error);
    //                     useIpGeoService = false; // Переключаемся на Cloudflare
    //                     sendNextRequest();
    //                 }
    //             );
    //         } else {
    //             Serial.println("Sending Cloudflare request");
                
    //             httpsClient.get("https://speed.cloudflare.com/meta",
    //                 nullptr,
    //                 nullptr,
    //                 [this]() {
    //                     if (httpsClient.getStatusCode() == 200) {
    //                         parseCloudflareResponse(httpsClient.getBody());
    //                     } else {
    //                         Serial.printf("Cloudflare request failed: %d\n", httpsClient.getStatusCode());
    //                         requestInProgress = false;
    //                     }
    //                 },
    //                 [this](const String& error) {
    //                     Serial.println("Cloudflare error: " + error);
    //                     requestInProgress = false;
    //                 }
    //             );
    //         }
    //     }

    // public:
    //     void begin(GeoData* data, const char* key) {
    //         targetData = data;
    //         geoKey = key;
    //         httpsClient.setInsecureMode(true);
    //         httpsClient.setTimeout(10000); // 10 секунд таймаут
    //     }

    //     RequestGeoAsync::Error update() {
    //         if (WiFi.status() != WL_CONNECTED) return RequestGeoAsync::NoConnection;
            
    //         httpsClient.update();
            
    //         if (!requestInProgress && !ipGeoCompleted && !cloudflareCompleted) {
    //             requestInProgress = true;
    //             sendNextRequest();
    //             return RequestGeoAsync::Pending;
    //         }
            
    //         if (ipGeoCompleted || cloudflareCompleted) {
    //             return RequestGeoAsync::OK;
    //         }
            
    //         // Если запрос завис, попробуем снова после таймаута
    //         if (millis() - lastRetryTime > GEO_RETRY_INTERVAL) {
    //             lastRetryTime = millis();
    //             httpsClient.reset();
    //             requestInProgress = false;
    //             useIpGeoService = true; // Сброс к первоначальному сервису
    //             return update();
    //         }
            
    //         return RequestGeoAsync::Pending;
    //     }
    // };


    class GeoRequest {
        private:
            AsyncHttpsClient httpsClient;
            GeoData* targetData;
            const char* geoKey;
            unsigned long lastRequestTime = 0;
            bool requestInProgress = false;
            bool waitingResponse = false;
            bool hasPreciseLocation = false;
            const unsigned long RETRY_PRECISE_INTERVAL = 30 MINUTES; // Интервал для повторных попыток точного определения
        
            void parseIpGeoResponse(const String& response) {
                JsonDocument doc;
                if (deserializeJson(doc, response)) {
                    Serial.println("Failed to parse IPGeo response");
                    return;
                }
        
                strlcpy(targetData->country, doc["country_name"] | "", CountrySize);
                strlcpy(targetData->city, doc["city"] | "", CitySize);
                strlcpy(targetData->countryCode, doc["country_code2"] | "", CountryCodeSize);
                strlcpy(targetData->timeZone, doc["time_zone"]["name"] | "", TZSize);
                
                targetData->tzOffset = doc["time_zone"]["offset_with_dst"] | TX_OFFSET_INVALID;
                targetData->latitude = doc["latitude"] | 0.0;
                targetData->longitude = doc["longitude"] | 0.0;
                
                float unixTime = doc["time_zone"]["current_time_unix"] | 0.0;
                if (!TimeUtils::isSynced() && unixTime != 0.0) {
                    targetData->unixTime = static_cast<time_t>(unixTime);
                    TimeUtils::setGMTTime(targetData->unixTime);
                }
        
                hasPreciseLocation = true;
                aproximateLocationAsync = false;
                requestInProgress = false;
                waitingResponse = false;
                
                Serial.println("Got precise location from IPGeo");
            }
        
            void parseCloudflareResponse(const String& response) {
                JsonDocument doc;
                if (deserializeJson(doc, response)) {
                    Serial.println("Failed to parse Cloudflare response");
                    return;
                }
        
                strlcpy(targetData->country, doc["country"] | "", CountrySize);
                strlcpy(targetData->city, doc["city"] | "", CitySize);
                targetData->latitude = doc["latitude"] | 0.0;
                targetData->longitude = doc["longitude"] | 0.0;
        
                requestInProgress = false;
                waitingResponse = false;
                aproximateLocationAsync = true;
                
                Serial.println("Got approximate location from Cloudflare");
            }
        
            void sendIpGeoRequest() {
                char url[150];
                strcpy(url, "https://api.ipgeolocation.io/ipgeo?apiKey=");
                strcat_P(url, geoKey);
                
                Serial.print("Sending IPGeo request: ");
                Serial.println(url);
        
                httpsClient.get(url, 
                    nullptr,
                    nullptr,
                    [this]() {
                        if (httpsClient.getStatusCode() == 200) {
                            parseIpGeoResponse(httpsClient.getBody());
                        } else {
                            Serial.println("IPGeo request failed, trying Cloudflare...");
                            sendCloudflareRequest();
                        }
                    },
                    [this](const String& error) {
                        Serial.print("IPGeo request error: ");
                        Serial.println(error);
                        sendCloudflareRequest();
                    }
                );
                
                lastRequestTime = millis();
                requestInProgress = true;
                waitingResponse = true;
            }
        
            void sendCloudflareRequest() {
                Serial.println("Sending Cloudflare request");
                
                httpsClient.get("https://speed.cloudflare.com/meta",
                    nullptr,
                    nullptr,
                    [this]() {
                        if (httpsClient.getStatusCode() == 200) {
                            parseCloudflareResponse(httpsClient.getBody());
                        } else {
                            Serial.println("Cloudflare request failed");
                            requestInProgress = false;
                        }
                    },
                    [this](const String& error) {
                        Serial.print("Cloudflare request error: ");
                        Serial.println(error);
                        requestInProgress = false;
                    }
                );
                
                lastRequestTime = millis();
                requestInProgress = true;
                waitingResponse = true;
            }
        
        public:
            void begin(GeoData* data, const char* key) {
                targetData = data;
                geoKey = key;
                httpsClient.setInsecureMode(true);
                httpsClient.setTimeout(15000);
            }
        
            RequestGeoAsync::Error update() {
                if (hasPreciseLocation) {
                    return RequestGeoAsync::OK; // Уже есть точные данные
                }
        
                if (WiFi.status() != WL_CONNECTED) {
                    return RequestGeoAsync::NoConnection;
                }
        
                httpsClient.update();
        
                // Первый запрос при инициализации
                if (!requestInProgress && !waitingResponse && lastRequestTime == 0) {
                    if (geoKey != nullptr) {
                        sendIpGeoRequest();
                    } else {
                        sendCloudflareRequest();
                    }
                    return RequestGeoAsync::Pending;
                }
        
                // Периодическая проверка для точного определения (если есть ключ)
                if (!requestInProgress && !waitingResponse && geoKey != nullptr && 
                    aproximateLocationAsync && 
                    millis() - lastRequestTime > RETRY_PRECISE_INTERVAL) {
                    sendIpGeoRequest();
                    return RequestGeoAsync::Pending;
                }
        
                if (waitingResponse) {
                    return RequestGeoAsync::Pending;
                }
        
                return targetData->valid() ? RequestGeoAsync::OK : RequestGeoAsync::NoData;
            }
        
            void reset() {
                httpsClient.reset();
                requestInProgress = false;
                waitingResponse = false;
                hasPreciseLocation = false;
                lastRequestTime = 0;
            }
        };


    GeoRequest geoRequester;
    
    GeoData myLocation;

    RequestGeoAsync::Error getLocation(GeoData &data) {
        static bool initialized = false;
        if (!initialized ) {
            geoRequester.begin(&data, eepromSets.getGeoKey());
            initialized = true;
        }
        
        return geoRequester.update();
    }


    RequestGeoAsync::Error getLocation(GeoData &data, Adafruit_PCD8544 * display = nullptr)
    {        
        printDots(display);
        geoRequester.begin(&myLocation, eepromSets.getGeoKey());
    
        while (true) {
            auto status = geoRequester.update();
            
            switch (status) {
                case RequestGeoAsync::OK:
                    if (aproximateLocationAsync) {
                        Serial.print("Approximate location: ");
                    } else {
                        Serial.print("Precise location: ");
                    }
                    myLocation.printTo(Serial);
                    return status;
                    
                case RequestGeoAsync::NoConnection:
                    Serial.println("Waiting for network...");
                    break;
                    
                case RequestGeoAsync::Pending:
                    Serial.println("Waiting for response...");
                    break;
                    
                case RequestGeoAsync::NoData:
                    Serial.println("No location data yet");
                    break;
            }
            
            delay(100);
            printDots(display);
        }

        // Request::Error err = Request::Error::ErrorData;
        // // if ( Key::has() ){
        // //     err = getLocation_IpGeo(data, Key::get() );
        // if ( eepromSets.getGeoKey() != nullptr ){
        //     err = getLocation_IpGeo(data, eepromSets.getGeoKey() );
        // }

        // if ( err != Request::Error::OK ) {
        //     err =  getLocation_speedCloudflare(data);
        // } else {
        //     aproximateLocation = false;
        // }
        // //err =  getLocation_speedCloudflare(data);
        // if (!err && data.valid())
        // {
        //     data.printTo(Serial);
        // } else {
        //     Serial.print("Error:"); Serial.println(err);
        // }
        
        // printDots(display);


        // Serial.println("Result geo info:");
        // data.printTo(Serial);

        // return err;
    };

    // RequestGeoAsync::Error getLocation(GeoData &data, const char * key) {
    //     static bool initialized = false;
    //     if (!initialized) {
    //         geoRequester.begin(&data, key);
    //         initialized = true;
    //     }
        
    //     return geoRequester.update();
    // }

  

    // #define MY_CREDENTIAL
    // #include <my.h>

    // void test() {
    //     static const char* apiKey = "ewrwqr45234r32423523"; //APP_IPGEOLOCATION_IO_KEY; // Может быть nullptr
    
    //     if (!WiFi.isConnected()) {
    //         while (!wm.autoConnect(CaptivePortal::name)) {
    //             delay(100);
    //         }
    //     }
        
    //     Serial.println("Start location detection");
    //     geoRequester.begin(&myLocation, apiKey);
        
    //     while (true) {
    //         auto status = geoRequester.update();
            
    //         switch (status) {
    //             case RequestGeoAsync::OK:
    //                 if (aproximateLocationAsync) {
    //                     Serial.print("Approximate location: ");
    //                 } else {
    //                     Serial.print("Precise location: ");
    //                 }
    //                 myLocation.printTo(Serial);
    //                 return;
                    
    //             case RequestGeoAsync::NoConnection:
    //                 Serial.println("Waiting for network...");
    //                 break;
                    
    //             case RequestGeoAsync::Pending:
    //                 Serial.println("Waiting for response...");
    //                 break;
                    
    //             case RequestGeoAsync::NoData:
    //                 Serial.println("No location data yet");
    //                 break;
    //         }
            
    //         delay(1000);
    //     }
    // }

    // /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // void parseIpGeoResponse(const String& response, GeoData* targetData) {
    //     JsonDocument doc;
    //     deserializeJson(doc, response);
        
    //     strlcpy(targetData->country, doc["country_name"], CountrySize);
    //     strlcpy(targetData->city, doc["city"], CitySize);
    //     strlcpy(targetData->countryCode, doc["country_code2"], CountryCodeSize);
    //     strlcpy(targetData->timeZone, doc["time_zone"]["name"], TZSize);
        
    //     targetData->tzOffset = doc["time_zone"]["offset_with_dst"] | TX_OFFSET_INVALID;
    //     targetData->latitude = doc["latitude"];
    //     targetData->longitude = doc["longitude"];
        
    //     float unixTime = doc["time_zone"]["current_time_unix"];
    //     if (!TimeUtils::isSynced() && unixTime != 0.0) {
    //         targetData->unixTime = static_cast<time_t>(unixTime);
    //         TimeUtils::setGMTTime(targetData->unixTime);
    //     }

    //     //ipGeoCompleted = true;
    //     aproximateLocationAsync = false;
    //     //requestInProgress = false;
    // }


    // void parseCloudflareResponse(const String& response, GeoData* targetData) {
    //     JsonDocument doc;
    //     deserializeJson(doc, response);
        
    //     strlcpy(targetData->country, doc["country"], CountrySize);
    //     strlcpy(targetData->city, doc["city"], CitySize);
    //     targetData->latitude = doc["latitude"];
    //     targetData->longitude = doc["longitude"];

    //     // cloudflareCompleted = true;
    //     // requestInProgress = false;
    // }
    // void test2(){
    //     constexpr unsigned long RequestPeriod = 1000U*60*5; //5 minutes
    //     static auto lastRequest = millis()-RequestPeriod;
    //     ClientState lastState;
    //     ClientState state;


    //     static const char apiKey[] PROGMEM = APP_IPGEOLOCATION_IO_KEY;
    //     //static const char *apiKey = nullptr;

    //     if (!WiFi.isConnected()) {
    //         while (!wm.autoConnect(CaptivePortal::name)) {
    //             delay(100);
    //         }
    //     }
        
    //     //static unsigned long startGeoRequest = millis();
    //     Serial.println("Start get location");

    //     static AsyncHttpsClient httpsClient;
    //     httpsClient.setInsecureMode();
    //     httpsClient.setTimeout(5000);

    //     bool requestTimeout = (millis()-lastRequest) >= RequestPeriod;

    //     Serial.printf("Api key=%s\nclient %s\n%s\n", apiKey, 
    //         httpsClient.isBusy() ? "busy" : "free",
    //         requestTimeout ? "start request" : "wait request time");

    //     while (  aproximateLocationAsync ){
    //         delay(1);
    //         requestTimeout = (millis()-lastRequest) >= RequestPeriod;

    //         if ( apiKey != nullptr &&  ! httpsClient.isBusy() && requestTimeout ){
    //             //get ipGeo
    //             Serial.println("REquest ipgeolocation");
    //             delay(10);


    //             static String url;
    //             url = "https://api.ipgeolocation.io/ipgeo?apiKey=";
    //             url += FPSTR( apiKey );
    //             Serial.println( url );

    //             httpsClient.get(url, 
    //                 nullptr,
    //                 nullptr,
    //                 [&]() {
    //                     if (httpsClient.getStatusCode() == 200) {
    //                         Serial.println( httpsClient.getBody());
    //                         parseIpGeoResponse(httpsClient.getBody(), &myLocation);
    //                     } else {
    //                         Serial.printf("IPGeo request failed: %d\n", httpsClient.getStatusCode());
    //                         // useIpGeoService = false; // Переключаемся на Cloudflare
    //                         // sendNextRequest();
    //                     }
    //                 },
    //                 [&](const String& error) {
    //                     Serial.println("IPGeo error: " + error);
    //                     // useIpGeoService = false; // Переключаемся на Cloudflare
    //                     // sendNextRequest();
    //                 }
    //             );
    //             lastRequest=millis();

    //         } 
            
    //         if( ! myLocation.valid() && ! httpsClient.isBusy() ) {
    //                 // get cloudflare
    //                 Serial.println( "cloudflare " );
    //                 httpsClient.get("https://speed.cloudflare.com/meta",
    //                     nullptr, nullptr,
    //                     [&]() {
    //                         Serial.printf("Cloudflare response: %d, Body: %s\n", 
    //                             httpsClient.getStatusCode(), 
    //                             httpsClient.getBody().c_str());
    //                        Serial.printf("Headers: %s\n", httpsClient.getHeaders().c_str());
               
    //                         if (httpsClient.getStatusCode() == 200) {
                                
    //                             parseCloudflareResponse(httpsClient.getBody(), &myLocation);
                                
    //                         } else {
    //                             Serial.printf("Cloudflare request failed: %d\n", httpsClient.getStatusCode());
                                
    //                         }
    //                     },
    //                     [&](const String& error) {
    //                         Serial.println("Cloudflare error: " + error);
                            
    //                         // useIpGeoService = false; // Переключаемся на Cloudflare
    //                         // sendNextRequest();
    //                     }
    //                 );
                
    //         }
            
    //         state = httpsClient.update();
    //         if ( state != lastState ){
    //             Serial.print("State="); Serial.println( (int)state);
    //             lastState = state;
    //         }


    //         if ( state == ClientState::COMPLETE ) {
    //             //httpsClient.update();
    //             myLocation.printTo(Serial);

    //             //httpsClient.reset();
    //             Serial.println("Test done");
    //             //while(1) delay(1);
    //             //return;
    //             state = ClientState::IDLE;
    //             httpsClient.reset();
    //         }
    //     }

    // }
}