#pragma once
//#include <MultyPCD8544.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>
#include "FontsRus/FreeMonoBold6.h"
#include "FontsRus/FreeMonoBold12.h"


extern Adafruit_PCD8544 display1;
extern Adafruit_PCD8544 display2;
// extern MultiPCD8544 display1;
// extern MultiPCD8544 display2;

namespace TestChars {
    static long delayMs = 1000;
    void setDelay(const long _delay){
        delayMs = _delay;
    };
    void run(Adafruit_PCD8544& display, uint8_t size=1, const char start=0, const char finish=255){
        constexpr int fullScreen1=(84/6) * (48/8); 
        int fullScreen = fullScreen1/(size*2);
        display.setTextSize(size);
        auto i = start;
        while(1){
            int charsOnScreen=0;
            display.clearDisplay();         
            display.setCursor(0,0);   
            while( charsOnScreen <= fullScreen){
                display.print(i);
                charsOnScreen++;
                Serial.print( int(i),HEX), Serial.print(", " );
                if ( i == finish ) break;
                i++;
            }
            Serial.println();
            display.display();
            delay(delayMs);    
            if ( i == finish ) break;
        }
        delay(delayMs);
    };
    void run(Adafruit_PCD8544& display, const GFXfont& font, uint8_t size=1, const char start=0, const char finish=255){
        display.setFont(&font);
        run(display,size,start,finish);
    };

};

namespace displays {
void init(){
    display1.begin();
    display1.setContrast(CONTRAST1); // Установка контраста (0-127)
    display1.clearDisplay();

    display1.display();
  
    display2.begin();
    display2.setContrast(CONTRAST2); // Установка контраста (0-127)
    display2.clearDisplay();
    display2.setTextSize(1);
    display2.setFont(&FreeMonoBold6pt8b);
    display2.setCursor(10,20);
    display2.println("ПОгодная");
    display2.setCursor(10, display2.getCursorY());
    display2.print("СТанция");
    display2.setFont();
    display2.display();
};

};

namespace Display {
    int16_t rightAdjast(Adafruit_PCD8544& display, const String& str, const int textSize =1 )
    {      
        uint charWidth = 6 * textSize;
        auto xPos = display.width() - ( charWidth* (str.length()));
        return xPos;
    };

    void printRightAdjast(Adafruit_PCD8544& display, const String& str, const int textSize =1 , const int16_t _yPos = -10000){
        auto yPos = ( _yPos == -10000 ) ? display.getCursorY() : _yPos;
        display.setCursor( 
            rightAdjast(display, str, textSize), yPos);  
        display.setTextSize(textSize);
        display.print(str);
    };

    void setFontSize(Adafruit_PCD8544&display, const int size=0){
        switch(size){
        
            case 1 :
            display.setTextSize(1);
            display.setFont(&FreeMonoBold6pt8b);
            break;
            
            case 2:
            display.setTextSize(1);
            display.setFont(&FreeMonoBold12pt8b);
            break;

            case 3:
            display.setTextSize(2);
            display.setFont(&FreeMonoBold12pt8b);
            break;

            default:
            display.setTextSize(1);
            display.setFont();

        }
    }

};