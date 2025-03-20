#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

#include "wifi_utils.h"

namespace TimeZone {
    static const char url[] PROGMEM = 
    "https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.csv";
    
    static const char host[] PROGMEM = "raw.githubusercontent.com";
	static const char zoneData[] PROGMEM = "/nayarsystems/posix_tz_db/master/zones.csv";

    char data[32] = {0};
    bool valid(){ 
        return data[0] != '\0'; 
    };

    bool getRaw(){
        WiFiClientSecure client;
        client.setInsecure();
        bool res = client.connect( host, 443 );
		if ( !res )  {
            Serial.printf("Error connection: %s\n", host);
            return res;
		}
		// client.print(F("GET https://"));
		// client.print(TZDownloaderData::host); client.print(TZDownloaderData::zoneData); // url
		client.print(F("GET ")); client.print(zoneData);
		client.print( F(" HTTP/1.1\r\n" \
						"Host: "));
		client.println(host);
		client.println(F("User-Agent: ESP8266\r\n" \
               			"Connection: close\r\n"));
        int len = -1;
        while (client.connected()) {
        String line = client.readStringUntil('\n');
        line.toLowerCase();
        if (line.startsWith(F("content-length:"))) {
            len = line.substring(sizeof("content-length:")).toInt();
        }
        if (line == "\r") {
            break;
            }
        }
        res = (len > 0 || len == -1);
		// read all data from server
	    while (client.connected() && (len > 0 || len == -1)){
            String line = client.readStringUntil('\n');
            Serial.println(line);
            delay(0);
        }
        client.stop();
        return res;

    };



    bool get(const char* countryCity, char * dest = data, const char * srs = url)
    {
        bool res = false;
        if (WiFi.status() != WL_CONNECTED)
            return res;
        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();
        http.begin(client, srs);
        Serial.println("Send request for TimeZone");
        auto httpCode = http.GET();
        if ( httpCode == HTTP_CODE_OK ) {
            WiFiClient& stream = http.getStream();            
            while ( stream.available() ){
                auto line = stream.readStringUntil('\n');
                if ( line.substring(1).startsWith(countryCity)){
                    auto commaP = line.indexOf(',');
                    auto code = line.substring(commaP + 1);// +1, -1);
                    code.replace("\"","");
                    strcpy(dest, code.c_str());
                    Serial.printf("Get TZ: '%s'=>'%s'\n", countryCity, dest);
                    res = true;
                    break;
                } 
            }
        } else {
            Serial.print("http Error get Time Zone code="); Serial.println(httpCode);
        }
        if ( !valid ){
            Serial.printf("No code found for '%s'", countryCity);
        }
        http.end();
        return res;;
    }; //get

    void configTime( const char * countryCity, const char * server1, const char * server2=nullptr, const char * server3 =nullptr ){
        Serial.println(__PRETTY_FUNCTION__);
        Serial.printf("Has valid %s, ", valid()? "true" :"false");
        if ( valid() || get(countryCity)){
            ::configTime(data, server1, server2, server3 );
            Serial.print(" get "); Serial.print(data);
        }
        Serial.println();
    }; //configTime

};