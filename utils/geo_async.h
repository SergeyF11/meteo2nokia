#pragma once

#include <AsyncHTTPRequest_Generic.h>
#include <ArduinoJson.h>
#include "time_utils.h"
#include "wifi_utils.h"
#define TX_OFFSET_INVALID 99999

bool aproximateLocationAsync = true;
const unsigned long GEO_RETRY_INTERVAL = 5 MINUTES; // 5 минут в мс

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
    };

    class GeoRequest {
    private:
        AsyncHTTPRequest ipGeoRequest;
        AsyncHTTPRequest cloudflareRequest;
        GeoData* targetData;
        bool ipGeoCompleted = false;
        bool cloudflareCompleted = false;
        unsigned long lastRetryTime = 0;
        const char* geoKey;

        void processIpGeoResponse() {
            if (ipGeoRequest.readyState() == readyStateDone) {
                if (ipGeoRequest.responseHTTPcode() == 200) {
                    parseIpGeoResponse(ipGeoRequest.responseText());
                    ipGeoCompleted = true;
                    aproximateLocationAsync = false;
                }
                ipGeoRequest.abort();
            }
        }

        void processCloudflareResponse() {
            if (cloudflareRequest.readyState() == readyStateDone) {
                if (cloudflareRequest.responseHTTPcode() == 200 && !ipGeoCompleted) {
                    parseCloudflareResponse(cloudflareRequest.responseText());
                }
                cloudflareRequest.abort();
            }
        }

        void parseIpGeoResponse(const String& response) {
            JsonDocument doc;
            deserializeJson(doc, response);
            
            strlcpy(targetData->country, doc["country_name"], CountrySize);
            strlcpy(targetData->city, doc["city"], CitySize);
            strlcpy(targetData->countryCode, doc["country_code2"], CountryCodeSize);
            strlcpy(targetData->timeZone, doc["time_zone"]["name"], TZSize);
            
            targetData->tzOffset = doc["time_zone"]["offset_with_dst"] | TX_OFFSET_INVALID;
            targetData->latitude = doc["latitude"];
            targetData->longitude = doc["longitude"];
            
            float unixTime = doc["time_zone"]["current_time_unix"];
            if (!TimeUtils::isSynced() && unixTime != 0.0) {
                targetData->unixTime = static_cast<time_t>(unixTime);
                TimeUtils::setGMTTime(targetData->unixTime);
            }
        }

        void parseCloudflareResponse(const String& response) {
            JsonDocument doc;
            deserializeJson(doc, response);
            
            strlcpy(targetData->country, doc["country"], CountrySize);
            strlcpy(targetData->city, doc["city"], CitySize);
            targetData->latitude = doc["latitude"];
            targetData->longitude = doc["longitude"];
        }

    public:
        void begin(GeoData* data, const char* key) {
            targetData = data;
            geoKey = key;
            ipGeoRequest.setDebug(false);
            cloudflareRequest.setDebug(false);
        }

        RequestGeoAsync::Error update() {
            if (WiFi.status() != WL_CONNECTED) return RequestGeoAsync::NoConnection;
            
            // Если IPGeo еще не завершен и не в процессе
            if (!ipGeoCompleted && ipGeoRequest.readyState() == readyStateUnsent) {
                String url = "https://api.ipgeolocation.io/ipgeo?apiKey=" + String(geoKey);
                ipGeoRequest.open("GET", url.c_str());
                ipGeoRequest.send();
                return RequestGeoAsync::Pending;
            }
            
            // Если Cloudflare еще не завершен и не в процессе, и IPGeo не удался
            if (!cloudflareCompleted && !ipGeoCompleted && 
                cloudflareRequest.readyState() == readyStateUnsent) {
                cloudflareRequest.open("GET", "https://speed.cloudflare.com/meta");
                cloudflareRequest.send();
                return RequestGeoAsync::Pending;
            }
            
            // Обработка ответов
            processIpGeoResponse();
            processCloudflareResponse();
            
            // Если IPGeo не удался, планируем повтор
            if (!ipGeoCompleted && millis() - lastRetryTime > GEO_RETRY_INTERVAL) {
                lastRetryTime = millis();
                ipGeoRequest.abort();
                return update(); // Начнем заново
            }
            
            // Если какой-то запрос завершился успешно
            if (ipGeoCompleted || cloudflareCompleted) {
                return RequestGeoAsync::OK;
            }
            
            return RequestGeoAsync::Pending;
        }
    };

    GeoRequest geoRequester;
    GeoData myLocation;

    RequestGeoAsync::Error getLocation(GeoData &data) {
        static bool initialized = false;
        if (!initialized) {
            geoRequester.begin(&data, eepromSets.getGeoKey());
            initialized = true;
        }
        
        return geoRequester.update();
    }

    void test(){
        if( !WiFi.isConnected() ){
           while(  wm.autoConnect(CaptivePortal::name)){
            delay(100);
           }
        }
        static unsigned long startGeoRequest = millis();
        static RequestGeoAsync::Error err = GeoLocationAsync::getLocation(GeoLocationAsync::myLocation);
        while( err == RequestGeoAsync::Error::Pending ){
            delay(0);
            err = GeoLocationAsync::getLocation(GeoLocationAsync::myLocation);
        }
        if( err != RequestGeoAsync::Error::OK ){
            Serial.printf("Error: %d\n", err );
        } else {
            Serial.printf("%s location is: %s\n", aproximateLocationAsync ? "Aproximate" : "Fine", myLocation.city);
            while ( aproximateLocationAsync ){
                delay(0);
                err = GeoLocationAsync::getLocation(GeoLocationAsync::myLocation);     
            }
            Serial.printf("%s location is: %s\n", aproximateLocationAsync ? "Aproximate" : "Fine", myLocation.city);
        }
    }
}