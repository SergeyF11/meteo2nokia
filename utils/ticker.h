#pragma once
#include <Arduino.h>


struct SimpleTicker {
  private:
  unsigned long lastTick = 0;
  public:
  const unsigned long interval;
  SimpleTicker(const unsigned long interval=1000) :
    interval(interval)
    { 
      reset();
    };
  void reset(const unsigned long ms=0){
    lastTick = ms ? ms : millis();
  };
  unsigned long msToNextTick() const {
    return (millis() - lastTick);
  }
//   void set(const unsigned long ms){
//     lastTick = ms;
//   };
  bool tick(){
    bool res = ( msToNextTick() >= interval );
    if (res ) reset();
    return res;
  };
 
};

// bool tick(){
//     unsigned long currentMillis = millis();
//     bool res = (currentMillis - lastUpdate >= weatherUpdateInterval);
//     if ( res ) currentMillis = millis();
//     return res;
//   };