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

    const char * toStr(char * buf, const tm * _tm, const char separator=':', bool spaceBeforeHour=false){
        sprintf(buf,"%s%2u%c%02u", ( spaceBeforeHour && _tm->tm_hour <10 )? " ": "", _tm->tm_hour, separator, _tm->tm_min);
        return buf;
    };
    const char * toStr(char * buf, const time_t * t = nullptr, const char separator=':', bool spaceBeforeHour=false){
        time_t _t =  t ? *t : time(nullptr);
        auto _tm = localtime( &_t);
        return toStr(buf, _tm, separator, spaceBeforeHour );
    };
    
    String toString(const tm * _tm, const char separator=':', bool spaceBeforeHour=false){
        String out;
        if ( spaceBeforeHour && _tm->tm_hour<10 ) out += ' ';
        out += _tm->tm_hour;
        out += separator;
        if ( _tm->tm_min <10 ) out += '0';
        out += _tm->tm_min;
        return out;
    };

    String toString(const time_t * t = nullptr, const char separator=':', bool spaceBeforeHour=false){
        String out;
        auto now = time(nullptr);
        auto _tm = localtime(t ? t : &now);
        return toString( _tm, separator, spaceBeforeHour);
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
            // display.println( toString(
            //     nowTm, (nowTm->tm_sec % 2) ? ' ' : ':', true));
            char timeBuf[6] = {0};
            display.println(
                toStr(
                    timeBuf, nowTm, (nowTm->tm_sec % 2) ? ' ' : ':', true)
                );
            return true;
        }
        return false;
    };
  
};