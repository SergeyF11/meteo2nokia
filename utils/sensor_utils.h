#pragma once

#include <Adafruit_HTU21DF.h> // Используем библиотеку Adafruit HTU21D
//#include <SparkFunHTU21D.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_PCD8544.h>
#include "display_utils.h"
//#include <Adafruit_PCD8544_multi.h>

// 'home_temp', 32x32px
const unsigned char icon_home_temp[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 
	0x07, 0x1c, 0x00, 0x00, 0x0e, 0x0e, 0x00, 0x00, 0x1c, 0x07, 0x00, 0x00, 0x38, 0xe3, 0x80, 0x00, 
	0x71, 0x11, 0xc0, 0x00, 0xe1, 0x10, 0xe0, 0x00, 0xc1, 0x10, 0x60, 0x00, 0xb1, 0x11, 0xa0, 0x00, 
	0x31, 0x11, 0x80, 0x00, 0x31, 0x11, 0x80, 0x00, 0x31, 0x51, 0x80, 0x00, 0x31, 0x51, 0x80, 0x00, 
	0x31, 0x51, 0x80, 0x00, 0x31, 0x51, 0x80, 0x00, 0x32, 0xe9, 0x80, 0x00, 0x32, 0xe9, 0x80, 0x00, 
	0x31, 0xf1, 0x80, 0x00, 0x30, 0x01, 0x80, 0x00, 0x3f, 0xff, 0x80, 0x00, 0x3f, 0xff, 0x80, 0x00
};

// Создаем объект для датчика HTU21D
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

extern tm* nowTm;

namespace I2C_Scan {
  size_t printTo(Print& p)
  {
    size_t size = 0;
    size += p.println("Scan I2c");
    int count = 0;
    for ( byte i=1; i<127; i++){
      Wire.beginTransmission(i);
      if (Wire.endTransmission() == 0) { 
        size += p.print("Found I2C device on 0x");
        size += p.println(i, HEX);
        count++;
      }
    }
    size += p.printf("Found %d devices\n", count );
    return size;
  };
};

namespace HtuSensor {

bool hasSensor = false;
inline bool isValid(float& val){
  return val <300;
};
float temperature, humidity;

bool updateData(){
  static float prevTemp = 999, prevHumid = 999;
    
    if ( hasSensor ){
      temperature = htu.readTemperature();
      humidity = htu.readHumidity();
        
        Serial.println("in home Temperature: " + String(temperature) + " C");
        Serial.println("in home Humidity: " + String(humidity) + " %");

        if ( ! isValid(temperature )) temperature = prevTemp;
        else prevTemp = temperature;
        
        if ( !isValid( humidity)) humidity = prevHumid;
        else prevHumid = humidity;
        
      return true;

    } else {
      temperature = prevTemp;
      humidity = prevHumid;
      return false;
    }
};

void printData(Adafruit_PCD8544& display, bool clear=false){
      if ( clear ) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.setTextColor(PRINT_COLOR);
      display.println(" --:--");
    }

    //display.setTextSize(2);
    //display.print("T:");
    auto yPos = display.getCursorY();
    {
      String temp("na");
      if ( isValid(temperature)) {
        temp = String( temperature, 1);
        // если дома минус, то округляем до целго, десятые уже без надобности
        // и в экран не влезут
        if ( temp.startsWith("-"))
          temp = String( temperature,0);
        temp += 'C'; //char(0x9); //'C';  
      }
      //else temp ="na";
    
    // display.setCursor( 
    //       Display::rightAdjast(display, temp), 
    //       display.getCursorY());  
    // display.print(temp);
      Display::printRightAdjast(display, temp, 2);
      display.println();
    }
    {
      String hum("na");
    //display.print("H:");
      if ( isValid(humidity)) {
        hum = String(humidity, 0);
        hum += '%';
      } //else display.print("na");
    Display::printRightAdjast(display, hum,2);
      
    }
    // домик
    //display.setCursor(0, yPos+8);
    display.drawBitmap(0, yPos, icon_home_temp ,32, 32, PRINT_COLOR);  
    //display.setFont();
    // display.setTextSize(3);
    // display.print( char(0x7f));
    display.display();
};

// void tick(Adafruit_PCD8544& display){
//     static int lastUpdate = -1;

//     bool needRenew = nowTm->tm_min != lastUpdate ;
//     sensorData(display, needRenew);
//     if ( needRenew ) lastUpdate = nowTm->tm_min;
// };

// void update(Adafruit_PCD8544& display){
//   static int lastUpdate = -1;

//   bool needRenew = nowTm->tm_min != lastUpdate ;
//   sensorData(display, needRenew);
//   if ( needRenew ) lastUpdate = nowTm->tm_min;
// };

};