#pragma once

#include "AsyncHttpsClient.h"
#include <ArduinoJson.h>
#include "time_utils.h"
#include "wifi_utils.h"

//#define POINT_STOP_GEO
#ifdef POINT_STOP_GEO
#define pointStop(ms, fmt, ...) { Serial.printf( "[%d] %s ", __LINE__,  __PRETTY_FUNCTION__); Serial.printf(fmt, ## __VA_ARGS__); delay(ms); }
#else
#define pointStop(ms, fmt, ...)
#endif

extern AsyncHttpsClient httpsClient;

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
        Pending,
        NewData
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
        
        bool valid() const { return city[0] != '\0'; }
        bool validOffset() const { return tzOffset != TX_OFFSET_INVALID; }

        size_t printTo(Print &p) const 
        {
            size_t printed = p.printf("CountryCode: %s, City: %s, Lat:%f, Long:%f\nTimeZone:%s\n",
                                      countryCode, city, latitude, longitude, timeZone);
            if ( validOffset() ) printed += p.printf("Tz offset = %d\n", tzOffset);
            return printed;
        };
    };


    class GeoRequest {
        private:
            //AsyncHttpsClient httpsClient;
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
                    pointStop(0,"Failed to parse IPGeo response\n");
                    return;
                }
        
                strlcpy(targetData->country, doc["country_name"] | "", CountrySize);
                strlcpy(targetData->city, doc["city"] | "", CitySize);
                strlcpy(targetData->countryCode, doc["country_code2"] | "", CountryCodeSize);
                strlcpy(targetData->timeZone, doc["time_zone"]["name"] | "", TZSize);
                
                targetData->tzOffset = doc["time_zone"]["offset_with_dst"] | TX_OFFSET_INVALID;
                targetData->latitude = doc["latitude"].as<float>(); // | 0.0;
                targetData->longitude = doc["longitude"].as<float>(); // | 0.0;
                
                float unixTime = doc["time_zone"]["current_time_unix"].as<float>(); // | 0.0;
                if (!TimeUtils::isSynced() && unixTime != 0.0) {
                    targetData->unixTime = static_cast<time_t>(unixTime);
                    TimeUtils::setGMTTime(targetData->unixTime);
                }
        
                hasPreciseLocation = true;
                aproximateLocationAsync = false;
                requestInProgress = false;
                waitingResponse = false;
                
                pointStop(0, "Got precise location from IPGeo\n");
            }
        
            void parseCloudflareResponse(const String& response) {
                JsonDocument doc;
                if (deserializeJson(doc, response)) {
                    pointStop(0,"Failed to parse Cloudflare response\n");
                    return;
                }
        
                strlcpy(targetData->country, doc["country"] | "", CountrySize);
                strlcpy(targetData->city, doc["city"] | "", CitySize);
                targetData->latitude = doc["latitude"] | 0.0;
                targetData->longitude = doc["longitude"] | 0.0;
        
                requestInProgress = false;
                waitingResponse = false;
                aproximateLocationAsync = true;
                
               pointStop(0,"Got approximate location from Cloudflare\n");
            }
        
            void sendIpGeoRequest() {
                char url[150];
                strcpy(url, "https://api.ipgeolocation.io/ipgeo?apiKey=");
                strcat_P(url, geoKey);
                
                pointStop(0,"Sending IPGeo request: %s\n", url);
        
                httpsClient.get(url, 
                    nullptr,
                    nullptr,
                    [this]() {
                        if (httpsClient.getStatusCode() == 200) {
                            parseIpGeoResponse(httpsClient.getBody());
                        } else {
                            pointStop(0,"IPGeo request failed, trying Cloudflare...\n");
                            sendCloudflareRequest();
                        }
                    },
                    [this](const String& error) {
                        pointStop(0,"IPGeo request error: %s\n", error.c_str());
                        sendCloudflareRequest();
                    }
                );
                
                lastRequestTime = millis();
                requestInProgress = true;
                waitingResponse = true;
            }
        
            void sendCloudflareRequest() {
                pointStop(0,"Sending Cloudflare request\n");
                
                httpsClient.get("https://speed.cloudflare.com/meta",
                    nullptr,
                    nullptr,
                    [this]() {
                        if (httpsClient.getStatusCode() == 200) {
                            parseCloudflareResponse(httpsClient.getBody());
                        } else {
                            pointStop(0,"Cloudflare request failed\n");
                            requestInProgress = false;
                        }
                    },
                    [this](const String& error) {
                        pointStop(0,"Cloudflare request error: %s\n", error.c_str());
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
                // httpsClient.setInsecureMode(true);
                // httpsClient.setTimeout(15000);
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

    void handleTick(){
        static bool wifiIsOn= false;

        switch (geoRequester.update())
        {
        case RequestGeoAsync::OK:
        case RequestGeoAsync::Pending:
            break;
        case RequestGeoAsync::NoConnection:
            wifiIsOn = true;
            Reconnect::connect();
            break;        
        // case RequestGeoAsync::Pending:
            
        //     break;
        // case RequestGeoAsync::NoData:
        // case RequestGeoAsync::NewData:

        //    break;
        default:
            if ( wifiIsOn ){
                if ( wiFiSleep() ) wifiIsOn = false;
            }
            break;
        }
    }

    RequestGeoAsync::Error waitLocationReceived(GeoData &data, Adafruit_PCD8544 * display = nullptr)
    {        
        printDots(display);
        geoRequester.begin(&myLocation, eepromSets.getGeoKey());
        RequestGeoAsync::Error _lastStatus;
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
                    if ( _lastStatus != status) {
                        pointStop(0,"Waiting for network...\n");
                        _lastStatus = status;
                    }
                    break;
                    
                case RequestGeoAsync::Pending:
                    if ( _lastStatus != status) {
                        pointStop(0,"Waiting for response...\n");
                        _lastStatus = status;
                    }
                    break;
                    
                case RequestGeoAsync::NoData:
                    if ( _lastStatus != status) {
                        pointStop(0,"No location data yet\n");
                        _lastStatus = status;
                    }
                    break;
                default:
                    geoRequester.reset();
                    status = RequestGeoAsync::Error::NoConnection;
            }
            
            delay(100);
            printDots(display);
        }

    };


}