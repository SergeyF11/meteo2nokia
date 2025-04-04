#pragma once
#include <Arduino.h>


struct SimpleTicker {
  protected:
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
  unsigned long msFromLastTick() const {
    return (millis() - lastTick);
  };


//   void set(const unsigned long ms){
//     lastTick = ms;
//   };
  bool tick(){
    bool res = ( msFromLastTick() >= interval );
    if (res ) reset();
    return res;
  };
 
};

struct RefresherTicker : SimpleTicker {
  private:
  unsigned long lastRefresh = 0;
  const unsigned long refreshInterval;

  public:
  RefresherTicker(const unsigned long interval, const unsigned long refreshMs=300000UL /*5 min*/)
  : SimpleTicker(interval), refreshInterval(refreshMs)
  { 
    lastRefresh = lastTick; 
  };
  // void reset(const unsigned long ms=0)  {
  //   SimpleTicker::reset(ms);
  //   //lastRefresh = lastTick;
  // };
  bool needsRefresh() const {
    return (millis() - lastRefresh) >= refreshInterval;
  };
  unsigned long msToNextRefresh() const {
    return refreshInterval - (millis() - lastRefresh);
  };

  bool refresh() {
    if (needsRefresh() ) {
      //if( interval - msFromLastTick() < refreshInterval ) return false;
        lastRefresh = millis();
        return true;
    }
    return false;
  };
};
