#pragma once

#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "time_utils.h"
#include "wifi_utils.h"


#define MY_CREDENTIAL
#include <my.h>

#define POINT_STOP_GEO
#ifdef POINT_STOP_GEO
#define pointStop(ms, fmt, ...) { Serial.printf( "[%d] %s ", __LINE__,  __PRETTY_FUNCTION__); Serial.printf(fmt, ## __VA_ARGS__); delay(ms); }
#else
#define pointStop(ms, fmt, ...)
#endif


namespace Request
{
    enum Error
    {
        OK,
        NoConnection,
        NoResponse,
        NoData,
        ErrorData,
    };
}

#define TX_OFFSET_INVALID 99999
namespace GeoLocation
{
    static const size_t CountrySize = 32;
    static const size_t CitySize = 64;
    static const size_t TZSize = 48;
    static const size_t CountryCodeSize=3;
    struct GeoData
    {

        char    country[CountrySize] = {0};
        char    city[CitySize] = {0};
        char    timeZone[TZSize] = {0};
        char    countryCode[CountryCodeSize] = {0};
        int     tzOffset = TX_OFFSET_INVALID;
        float   latitude;
        float   longitude;
        time_t  unixTime;
        enum FieldType
        {
            None,
            Country,
            CountryCode,
            City,
            TimeZone,
            TzOffset,
            Latitude,
            Longitude,
            UnixTime,
        };            

        bool valid() { return (city[0] != '\0'); };
        bool validOffset(){ return tzOffset != TX_OFFSET_INVALID; };
        size_t printTo(Print &p)
        {
            size_t printed = p.printf("CountryCode: %s, City: %s, Lat:%f, Long:%f\nTimeZone:%s\n",
                                      countryCode, city, latitude, longitude, timeZone);
            if ( validOffset() ) printed += p.printf("Tz offset = %d\n", tzOffset);
            return printed;
        };
        
        FieldType fromFieldName(const char *name)
        {
            if (strcmp(name, "country") == 0  ||
                strcmp(name, "country_name") == 0 )
            {
                return Country;
            }

            if (strcmp(name, "countryCode" ) == 0 ||
                strcmp(name, "country_code2") == 0 )
            {
                return CountryCode;
            }
            if (strcmp(name, "city") == 0)
            {
                return City;
            }
            if (strcmp(name, "tz") == 0 ||
                strcmp(name, "time_zone") == 0 ||
                strcmp(name, "timezone") == 0)
            {
                return TimeZone;
            }
            if (strcmp(name, "offset") == 0 ||
                strcmp(name, "offset_with_dst") == 0 )
            {
                return TzOffset;
            }
            if (strncmp(name, "lat", 3) == 0)
            {
                return Latitude;
            }
            if (strncmp(name, "lon", 3) == 0)
            {
                return Longitude;
            }
            return None;
        };
    };


    //void getTimezoneData() {
    inline Request::Error getLocation_IpGeo(GeoData& data, const char * key) {
        pointStop(0, "Key='%s'\n", key);
       if (WiFi.status() != WL_CONNECTED) return Request::NoConnection;

        WiFiClientSecure client;
        HTTPClient http;
        client.setInsecure();

        // Формируем URL для запроса
        String url = "https://api.ipgeolocation.io/ipgeo?apiKey=";// + String(apiKey);
        url += key;
        //url += "&fields=city";

        pointStop(0, "Request: %s\n", url.c_str());
        // Начинаем HTTP-запрос
        http.begin(client, url);
    
        // Указываем, что мы хотим обрабатывать chunked encoding
        http.setReuse(true); // Позволяет повторно использовать соединение
        http.setTimeout(1000); // Устанавливаем таймаут
        int httpCode = http.GET();

        // Проверяем код ответа
        if (httpCode != HTTP_CODE_OK) {
            pointStop( 0, "Error: %s\n", http.errorToString(httpCode).c_str());
            http.end();
            return Request::NoResponse;
        }

    // Читаем ответ по частям (chunked encoding)
        WiFiClient* stream = http.getStreamPtr();
        String payload;

        bool startJson = false;
        // Читаем данные, пока соединение активно или есть доступные данные
        while (stream->connected() || stream->available()){

            if (stream->available()) {
            char c = stream->read(); // Читаем по одному символу
            if ( startJson ){
                if ( c == '\r') break;
                payload += c;
            } else if ( c =='\n' ) {
                startJson = true;
            }
            
            }
            delay(1);
        }
        http.end();
        client.stop();
            
//            Serial.println("Response: " + payload);
        if ( payload.isEmpty() ) return Request::NoData;
        pointStop(0, "JSON: %s\n", payload.c_str());

        // Парсим JSON ответ
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
    
        // Проверяем, успешно ли распарсен JSON
        if (error) {
            pointStop(0, "Failed to parse JSON: %s\n", error.c_str());
            return Request::ErrorData;
        }
      
        const char * country = doc["country_name"];
        pointStop(0, "Field: %s\n", country); 
        auto len = min( sizeof(data.country), strlen(country));
        strncpy ( data.country, country, len );

        const char * city = doc["city"];
        pointStop(0, "Field: %s\n", city);
        len = min( sizeof(data.city), strlen(city));
        strncpy ( data.city, city, len );

        const char * countryCode = doc["country_code2"];
        pointStop(0, "Field: %s\n", countryCode);
        len = min( sizeof(data.countryCode), strlen(countryCode));
        strncpy ( data.countryCode, countryCode, len );

        const char * tz = doc["time_zone"]["name"];
        pointStop(0, "Field: %s\n", tz);
        len = min( sizeof(data.timeZone), strlen(tz));
        strncpy ( data.timeZone, tz, len );

        if ( doc["time_zone"]["offset_with_dst"].is<int>() ){
            data.tzOffset = doc["time_zone"]["offset_with_dst"].as<int>();
            configTime( 3600L * data.tzOffset, 0 , NTP_SERVERS );
            pointStop(0, "Set time zone %s[%d]\n", data.timeZone, data.tzOffset);
        }

        if ( doc["time_zone"]["current_time_unix"].is<float>() ) {
            auto unixTime = doc["time_zone"]["current_time_unix"].as<float>();
            if (  !TimeUtils::isSynced() && unixTime != 0.0 ){
                data.unixTime = (time_t ) (unixTime ) ;
                TimeUtils::setGMTTime( data.unixTime );
                pointStop(0, "Set time %lld\n", data.unixTime );
            }
        }

        data.latitude = doc["latitude"].as<float>();
        data.longitude = doc["longitude"].as<float>();
        
  
        return Request::OK;
      };

    // // получает точную инфо за 15 секунд !!!!
    // // не работает
    // JsonRequest::Error getLocatiinton_IpGeo(GeoData& data, const char * key) {
    //     static const char apiUrl[] PROGMEM = "/ipgeo?apiKey=";
    //     static const char host[] PROGMEM = "api.ipgeolocation.io";
    //     if (WiFi.status() != WL_CONNECTED) return JsonRequest::NoConnection;

    //       WiFiClientSecure client;
    //       client.setInsecure();
    //       client.setTimeout(15000);

    //     //  HTTPClient http;
    //       String uri(apiUrl);

    //       uri += key;
    //         Serial.print("Send: "); Serial.println(uri);

    //         int retry = 20;
    //         while((!client.connect( host, 80)) && (--retry)){
    //             delay(100);
    //             Serial.print('.');
    //         }
    //         Serial.println();
    //         Serial.print(String("GET ") + uri + " HTTP/1.1\r\n" +
    //         "Host: " + host + "\r\n" +
    //         "User-Agent: esp8288" + "\r\n" +
    //         "Accept: */*\r\n" +
    //         "Connection: close\r\n\r\n");

    //         client.print(String("GET ") + uri + " HTTP/1.1\r\n" +
    //                "Host: " + hostAdafruit_PCD8544 + "\r\n" +
    //                "User-Agent: esp8288" + "\r\n" +
    //                "Accept: */*\r\n" +
    //                "Connection: close\r\n\r\n");

    //         auto timeout = [](){
    //             static unsigned long start = millis();
    //             return millis()-start >=15000;
    //         };
    //         int httpCode =-1;
    //         while (client.connected() || !timeout() ) {
    //             String _Response = client.readStringUntil('\n');
    //             if ( _Response.startsWith("HTTP/1.1")){
    //                 httpCode = _Response.substring( sizeof("HTTP/1.1")).toInt();
    //             }        // WiFiClientSecure client;
        // if ( url.startsWith("https:\\"))
        //     client.setInsecure();
    //             if (_Response.equals("\r")) {long
    //                 Serial.println(_Response);
    //                 Serial.println("break");
    //                 break;
    //             }
    //         }

    //         if (httpCode != HTTP_CODE_OK) {
    //             Serial.print("Error:"); Serial.println(httpCode);
    //               client.stop();setGMTTime
    //               return JsonRequest::NoResponse;  // Проверяем успешность запроса
    //           }

    //         //client.readShttp://ip-api.com/jsontringUntil('\n');
    //         String payload = client.readStringUntil('\n');

    //         while(client.connecintted() || !timeout()){
    //             Serial.println(payload);
    //             payload = client.readString();
    //             if ( payload.length() > 10 ) break;
    //           }
    //         client.stop();
    //     //  http.setTimeout(15000);
    //     //  http.begin(client, uri);  // Начинаем HTTP-запрос
    //     //  int httpCode = http.GET();  // Отправляем GET-запрос
    //     //   String payload = http.getString();  // Получаем ответ
    //     //   http.end();

    //       if ( payload.isEmpty() ) return JsonRequest::NoData;/* * 1000LL *

    //       Serial.println("Ответ от сервера:");
    //       Serial.println(payload);

    //       // Парсим JSON-ответ
    //       JsonDocument doc;
    //       //DynamicJsonDocument doc(1024);
    //       auto result = deserializeJson(doc, payload);

    //       // Извлекаем данные о местоположении
    //       //if ( !doc.containsKey("ip")) return JsonRequest::ErrorData;
    //       if ( result.code() !=  DeserializationError::Ok ) return JsonRequest::ErrorData;
    //       {
    //           String country = doc["country_name"].as<String>();
    //           auto len = min( sizeof(data.country), country.length());setGMTTime
    //           strncpy ( data.country, country.c_str(), len );
    //           data.country[len] = '\0';
    //       }
    //       {
    //           String city = doc["c            ity"].as<Str#include "MultyPCD8544.hing>();
    //           auto len = min( sizeof(data.city), city.length());
    //           strncpy ( data.city, city.c_str(), len );
    //           data.city[len] = '\0';
    //       }
    //       {
    //           String timeZone = doc["time_zone"]["name"].as<String>();
    //           auto len = min( sizeof(data.timeZone), timeZone.length());
    //           strncpy ( data.timeZone, timeZone.c_str(), len );
    //           data.timeZone[len] = '\0';
    //       }

    //       data.latitude = doc["latitude"].as<float>();;
    //       data.longitude = doc["longitude"].as<float>();;
    //       return JsonRequest::OK;
    //   };


    // auto list = { "city", "country" };

    int jsonTo(String &payload, GeoData &data, std::initializer_list<const char *> fields)
    {
        int decoded = 0;
        JsonDocument doc;
        auto result = deserializeJson(doc, payload);
        if (result.code() == DeserializationError::Ok)
        { // return fields.size();

            for (auto fieldName : fields)
            {
                auto type = data.fromFieldName(fieldName);
                switch (type)
                {
                case GeoData::Country:
                {
                    String country = doc[fieldName].as<String>();
                    auto len = min(CountrySize, country.length());
                    strncpy(data.country, country.c_str(), len);
                    data.country[len] = '\0';
                    decoded++;
                }
                break;
                case GeoData::CountryCode:
                {
                    String code = doc[fieldName].as<String>();
                    code.toLowerCase();
                    auto len = min(CountryCodeSize, code.length());
                    strncpy(data.countryCode, code.c_str(), len);
                    data.countryCode[len] = '\0';
                    decoded++;
                }
                break;
                case GeoData::City:
                {
                    String city = doc[fieldName].as<String>();
                    auto len = min(CitySize, city.length());
                    strncpy(data.city, city.c_str(), len);
                    data.city[len] = '\0';
                    decoded++;
                }
                break;
                case GeoData::TimeZone:
                {
                    String tz = doc[fieldName].as<String>();
                    auto len = min(TZSize, tz.length());
                    strncpy(data.timeZone, tz.c_str(), len);
                    data.timeZone[len] = '\0';
                    decoded++;
                }
                break;
                case GeoData::Longitude:     
                {
                    data.longitude = doc[fieldName].as<float>();
                    decoded++;
                }
                break;
                case GeoData::Latitude:
                {
                    data.latitude = doc[fieldName].as<float>();
                    decoded++;
                }
                break;
                }
            }            

        }
        return decoded;
    };

    Request::Error _getLocation(const String& url, GeoData & data, std::initializer_list<const char *> fields, const char * urlOptions = nullptr ){
        if (WiFi.status() != WL_CONNECTED)
            return Request::NoConnection;
        
        String requestUri(url);
        if (urlOptions)
            requestUri += urlOptions;

        HTTPClient http;
        // 
        int httpCode;
        const char * _headers[] = {"Date"}; 
        http.collectHeaders( _headers, 1);

        if ( url.startsWith("https:")) {
            WiFiClientSecure secureClient;
            secureClient.setInsecure();
            http.begin(secureClient, requestUri);   // Начинаем HTTP-запрос
            //httpCode = http.GET(); // Отправляем GET-запрос
        } else {
            WiFiClient client;
            http.begin(client, requestUri);   // Начинаем HTTP-запрос
            //httpCode = http.GET(); // Отправляем GET-запрос
        }
        
        httpCode = http.GET(); // Отправляем GET-запрос
        if (httpCode != HTTP_CODE_OK)
        {
            pointStop( 0, "Error: %s\n", http.errorToString(httpCode).c_str());
            http.end();
            return Request::NoResponse; // Проверяем успешность запроса
        }
        if (  !TimeUtils::isSynced() &&  http.hasHeader("Date")){
            String headerDate( http.header("Date" ));
            pointStop(0, "Header date: %s\n", headerDate.c_str());
            TimeUtils::setGMTTime( headerDate.c_str());
        }
        String payload = http.getString(); // Получаем ответ
        http.end();

        if (payload.isEmpty())
            return Request::NoData;

            pointStop(0, "Ответ от сервера '%s':\n%s\n", 
                requestUri.c_str(), payload.c_str());

        // Парсим JSON-ответ
        auto decoded = jsonTo(payload, data, fields);
        pointStop(0, "Decoded %d JSON fields. Expected %d.\n", decoded, fields.size() );

        if (fields.size() != decoded)
            Request::ErrorData;

        return Request::OK;
    };

    inline Request::Error getLocation_speedCloudflare(GeoData &data, const char *options = nullptr)
    {
        static const char apiUrl[] PROGMEM = "https://speed.cloudflare.com/meta";
        //Serial.print("Get info: "); Serial.println(apiUrl);

        pointStop(0, "Before request %s\n", apiUrl);

        auto list = {"country", "city", "latitude", "longitude"};
        
        return _getLocation( apiUrl, data, list, options);
    };

    // inline Request::Error getLocation_IpApi(GeoData &data, const char *options = nullptr)
    // {
        
    //     static const char apiUrl[] PROGMEM = "http://ip-api.com/json";

    //     //Serial.print("Get info: "); Serial.println(apiUrl);
    //     pointStop(0, "Before request %s\n", apiUrl);
        
    //     auto list = {"country", "countryCode", "city", "timezone", "lat", "lon"};
    //     return _getLocation( apiUrl, data, list, options);
    // };

    namespace Key {
        //bool has(){ return true; };
        #ifdef APP_IPGEOLOCATION_IO_KEY
        #pragma message ("USING IpGeolocationIO key")
        const char * get(){ return APP_IPGEOLOCATION_IO_KEY; };
        #else
        const char * get(){ return nullptr; };
        #endif
        // const char _key[] PROGMEM = APP_IPGEOLOCATION_IO_KEY; 
        // #else
        // const char * _key = nullptr; 
        // #endif
        // const char * get(){ return _key; };
         bool has(){ return ( get() && get()[0] != 0); };
    };
    

    Request::Error getLocation(GeoData &data, Adafruit_PCD8544 * display = nullptr)
    {        

        printDots(display);
        Request::Error err;
        if ( Key::has() ){
            err = getLocation_IpGeo(data, Key::get() );
        } else {
            err =  getLocation_speedCloudflare(data);
        }
        //err =  getLocation_speedCloudflare(data);
        if (!err && data.valid())
        {
            data.printTo(Serial);
        } else {
            Serial.print("Error:"); Serial.println(err);
        }
        
        printDots(display);


        Serial.println("Result geo info:");
        data.printTo(Serial);

        return err;
    };


}; // namespace

GeoLocation::GeoData myLocation;