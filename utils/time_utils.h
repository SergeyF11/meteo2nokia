#pragma once
#include <Arduino.h>
#include <TimeLib.h>

extern tm* nowTm;

namespace  TimeUtils {

    time_t fromHttpHeader(const char * dateHeader = nullptr){

        static unsigned long responseReceivedTime = 0; // Статическая переменная для фиксации момента получения HTTP-ответа
        static time_t httpTime = 0;

        auto receivedTime=millis();

        unsigned long secondsFromResponse = ( receivedTime-responseReceivedTime) / 1000;
        time_t expectedNow = httpTime + secondsFromResponse;
               
        //else
        struct tm tm;        
        if (dateHeader && strptime(dateHeader, "%a, %d %b %Y %H:%M:%S GMT", &tm)) {
            responseReceivedTime = receivedTime;
            httpTime = mktime(&tm);
            time_t now = time(nullptr);
            return now;
        }
        return expectedNow;

    };


    constexpr long likeSync = 3600*2*60;
    bool inline _isSynced(){
        return time(nullptr) > likeSync;
    };

    bool inline  isSynced(const time_t expectedTime, const unsigned long delta=1000){
        return ( abs( time(nullptr) - expectedTime )  < delta ) ;
    };


    bool isSynced(const char *dateHeader=nullptr){
        if ( !dateHeader) return _isSynced();

        auto t = fromHttpHeader(dateHeader);
        return _isSynced() && isSynced(t);
    };
    
    void inline  setGMTTime(time_t now){
        setTime(now);
    };
    void inline setGMTTime(const char * str){
        time_t now = fromHttpHeader(str);;
        setGMTTime(now);
    };


    bool printTo(Adafruit_PCD8544& display){
        static int lastPrint = -1;
    
        auto now = time(nullptr);
        nowTm = localtime( &now );
        if ( isSynced() && lastPrint != nowTm->tm_sec ) { 
            if ( -1 == lastPrint || nowTm->tm_sec == 0 )
                Serial.printf("Current time %2u:%02u:%02u\n", 
                    nowTm->tm_hour, nowTm->tm_min, nowTm->tm_sec );
            lastPrint = nowTm->tm_sec;
            display.clearDisplay();
            display.setCursor(10,0);
            display.setTextSize(2);
            if ( nowTm->tm_hour < 10 ) display.print(" ");
            display.print( nowTm->tm_hour);
            if ( nowTm->tm_sec % 2 == 0 )
                display.print(":");
            else
            display.print(" ");
            if ( nowTm->tm_min < 10 ) display.print("0");
            display.println( nowTm->tm_min);
            //display.display();
            return true;
        }
        return false;
    };
  
};