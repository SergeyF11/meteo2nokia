#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>

// 'wifi', 56x38px
namespace BigSign {
const uint width = 12;
const uint height = 16;
const unsigned char _percent[] PROGMEM = {	
	0x70, 0xc0, 0xf8, 0xc0, 0xd9, 0x80, 0xd9, 0x80, 0xfb, 0x00, 0x73, 0x00, 0x06, 0x00, 0x06, 0x00, 
	0x0c, 0xe0, 0x0d, 0xf0, 0x19, 0xb0, 0x19, 0xb0, 0x31, 0xf0, 0x30, 0xe0, 0x00, 0x00, 0x00, 0x00
};
const unsigned char _celsium[] PROGMEM = {
	0x61, 0xc0, 0x93, 0xe0, 0x97, 0x70, 0x96, 0x30, 0x66, 0x30, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 
	0x06, 0x00, 0x06, 0x30, 0x06, 0x30, 0x07, 0x70, 0x03, 0xe0, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00
};

size_t print(Adafruit_PCD8544& display, const unsigned char * sign, uint16_t color = 1 ){
    auto posX = display.getCursorX();
    auto posY = display.getCursorY();
    display.drawBitmap(posX, posY, sign, width, height, color);
    auto newX = posX+width;
    auto newY = posY;
    // if ( newX > display.width() ){
    //     newX = 0;
    //     newY += height;
    // }
    display.setCursor(newX, newY);
    return 1;
};
size_t println(Adafruit_PCD8544& display, const unsigned char * sign, uint16_t color = 1 ){
    auto size = print(display, sign, color);
    size += display.println();
    return size;
};

void test(Adafruit_PCD8544& display){
    display.setContrast(50);
    display.clearDisplay();
    display.setTextSize(2);
    display.print("T: 10");
    println(display, _celsium );
    display.print("H: 47");
    println(display, _percent );
    display.display();
};

};

