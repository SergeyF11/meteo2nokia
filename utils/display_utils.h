#pragma once
// #include <MultyPCD8544.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>

#include "wifi_icon.h"
// #include "FontsRus/FreeMonoBold6.h"
// #include "FontsRus/FreeMonoBold12.h"
#include "FontsEn/FreeMonoBold24pt7b.h"  //MonoBoldOblique24pt7b.h"
#include "FontsEn/FreeMonoBold12pt7b.h"
// #include "FontsRus/"
// #include "FontsRus/CrystalNormal14.h"
#include "eeprom_utils.h"
#include "icons_sign.h"

extern EepromData eepromSets;

/* Recode russian fonts from UTF-8 to Windows-1251 */
char *utf8rusTo(const String &source, char *target, const int maxString)
{
  int i, j, k;
  unsigned char n;
  char m[2] = {'0', '\0'};

  // strcpy(target, "");
  target[0] = '\0';
  k = source.length();
  i = j = 0;

  while (i < k)
  {
    n = source[i];
    i++;

    if (n >= 0xC0)
    {
      switch (n)
      {
      case 0xD0:
      {
        n = source[i];
        i++;
        if (n == 0x81)
        {
          n = 0xA8;
          break;
        }
        if (n >= 0x90 && n <= 0xBF)
          n = n - 1 + 0x30;
        break;
      }
      case 0xD1:
      {
        n = source[i];
        i++;
        if (n == 0x91)
        {
          n = 0xB8;
          break;
        }
        if (n >= 0x80 && n <= 0x8F)
          n = n - 1 + 0x70;
        break;
      }
      }
    }

    m[0] = n;
    strcat(target, m);
    j++;
    if (j >= maxString)
      break;
  }
  return target;
};

String utf8rus(const String &source)
{
  int i, k;
  String target;
  unsigned char n;
  char m[2] = {'0', '\0'};

  k = source.length();
  i = 0;

  while (i < k)
  {
    n = source[i];
    i++;

    if (n >= 0xC0)
    {
      // if (n >= 0xBF ) {
      switch (n)
      {
      case 0xD0:
      {
        n = source[i];
        i++;
        if (n == 0x81)
        {
          n = 0xA8;
          break;
        }
        if (n >= 0x90 && n <= 0xBF)
          n = n - 1 + 0x30;
        break;
      }
      case 0xD1:
      {
        n = source[i];
        i++;
        if (n == 0x91)
        {
          n = 0xB8;
          break;
        }
        if (n >= 0x80 && n <= 0x8F)
          n = n - 1 + 0x70;
        break;
      }
      }
    }
    m[0] = n;
    target = target + String(m);
  }
  return target;
}

// Функция для применения настроек контраста к дисплеям
void applyDisplayContrast(Adafruit_PCD8544 &display1, Adafruit_PCD8544 &display2)
{
  display1.setContrast(eepromSets.getContrast1());
  display2.setContrast(eepromSets.getContrast2());
  Serial.printf("Applied contrast: Display1=%d, Display2=%d\n", eepromSets.getContrast1(), eepromSets.getContrast2());
}

struct FontSize
{
  int width;
  int height;
};
FontSize fromSize(const int textSize)
{
  return FontSize{6 * textSize, 8 * textSize};
};

void printDots(Adafruit_PCD8544 &display, const byte *icon = WiFi_Icon::_bmp, const int textSize = 2)
{

  static int dots = 0;        // Счетчик точек
  static bool toggle = false; // Переключатель для анимации
  const char dot = '.';       // Символ точки
  const char space = ' ';     // Символ пробела

  display.clearDisplay();
  const int alignCenterX = (display.width()-WiFi_Icon::width)/2;
  //const int alignCenterY = ( display.height() - WiFi_Icon::height)/2;

  display.drawBitmap(alignCenterX, 2, icon, WiFi_Icon::width, WiFi_Icon::height, 1); // Отрисовка иконки

  display.setTextSize(textSize);          // Установка размера текста
  FontSize fontSize = fromSize(textSize); // Получение размера шрифта

  // Установка курсора внизу экрана
  display.setCursor(0, display.height() - fontSize.height);

  // Вычисление максимального количества точек, которые поместятся на экране
  int maxDots = display.width() / fontSize.width;

  for (int i = 0; i < maxDots; i++)
  {
    if (i < dots)
    {
      display.print(toggle ? space : dot); // Чередование точек и пробелов
    }
    else
    {
      display.print(toggle ? dot : space); // Чередование точек и пробелов
    }
  }

  // Обновление счетчика и переключателя
  dots++;
  if (dots >= maxDots)
  {
    dots = 0;
    toggle = !toggle; // Переключение состояния
  }

  display.display(); // Обновление дисплея
};

void inline printDots(Adafruit_PCD8544 *display, const byte *icon =  WiFi_Icon::_bmp, const int textSize = 2)
{
  if (display != nullptr)
    printDots(*display, icon, textSize);
};

#define DISPLAY_TEST
#ifdef DISPLAY_TEST
namespace TestChars
{
  static long delayMs = 1000;
  void setDelay(const long _delay)
  {
    delayMs = _delay;
  };

  void run(Adafruit_PCD8544 &display, uint8_t size = 1, const char start = 0, const char finish = 255)
  {
    constexpr int fullScreen1 = (84 / 6) * (48 / 8); // символов на экран
    int fullScreen = fullScreen1 / (size * 2);
    display.setTextSize(size);
    auto i = start;
    while (1)
    {
      int charsOnScreen = 0;
      display.clearDisplay();
      display.setCursor(0, 0);
      while (charsOnScreen < fullScreen)
      {
        display.print(i);
        charsOnScreen++;
        Serial.print(int(i), HEX), Serial.print(", ");
        if (i == finish)
          break;
        i++;
      }
      Serial.println();
      display.display();
      delay(delayMs);
      if (i == finish)
        break;
    }
    delay(delayMs);
  };
  void run(Adafruit_PCD8544 &display, const GFXfont &font, uint8_t size = 1, const char start = 0, const char finish = 255)
  {
    display.setFont(&font);
    run(display, size, start, finish);
  };

};


#endif

extern Adafruit_PCD8544 display1;
extern Adafruit_PCD8544 display2;
// extern MultiPCD8544 display1;
// extern MultiPCD8544 display2;

inline uint16_t getFontHeight(Adafruit_PCD8544& display, const GFXfont* font, const uint8_t size = 1){
  int16_t x = 0, y = 0;
  uint16_t w, h;
  display.setFont(font);
  display.setTextSize(size);
  display.getTextBounds("0", 0, 0, &x, &y, &w, &h); // Получаем высоту 'A'
  return h;
}

namespace displays
{
  void setContrast(const int idx, const uint8_t c, Print* p=nullptr)
  {
    switch(idx){
      case 1:
        display1.setContrast(c);
        break;
      case 2:
        display2.setContrast(c);
        break;
      default:
        if(p) p->println("incorrect display index");
        return;
    }
    if( p ) {
      p->print("display "); p->print(idx);
      p->print("set contrast="); p->println(c);
    }
  };
  void setContrast(const uint8_t c1 = 50, const uint8_t c2 = 50, Print* p=nullptr)
  {
    display1.setContrast(c1);
    display2.setContrast(c2);
    if( p ) {
      p->print("Contrast1="); p->println(c1);
      p->print("Contrast2="); p->println(c2);
    }
  };
  void init(EepromData* settings = nullptr)
  {
    display1.begin();

    display1.clearDisplay();
    display1.display();

    display2.begin();
    if( settings ) {
      setContrast(settings->getContrast1(), settings->getContrast2());
    }

    display2.clearDisplay();
    display2.setTextSize(1);


    display2.setTextSize(1);
    display2.setCursor(10, 20);
    display2.println(utf8rus("ПОгодная"));
    display2.setCursor(10, display2.getCursorY());
    display2.print(utf8rus("СТанция"));

    display2.setFont();
    display2.display();
  };

};

namespace Display
{
  int16_t rightAdjast(Adafruit_PCD8544 &display, const String &str, const int textSize = 1)
  {
    uint charWidth = 6 * textSize;
    auto xPos = display.width() - (charWidth * (str.length()));
    return xPos;
  };

  void printRightAdjast(Adafruit_PCD8544 &display, const String &str, const int textSize = 1, const int16_t _yPos = -10000)
  {
    auto yPos = (_yPos == -10000) ? display.getCursorY() : _yPos;
    display.setCursor(
        rightAdjast(display, str, textSize), yPos);

    display.setTextSize(textSize);
    if( textSize == 2 ){  
      auto endPtr = str.length()-1;
      auto endChar = str.charAt(endPtr);

      if( endChar == 'C' || endChar == '%'){

        display.print(str.substring(0, endPtr));

        BigSign::print(display, endChar == 'C' ? BigSign::_celsium : BigSign::_percent );
        return;
    }
  }
      display.print(str);
  };

 
};