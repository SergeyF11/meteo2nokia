#include <OneButton.h>

// Настройка кнопки
#define BUTTON_PIN 3  // Пин кнопки RX
#define LIGHT_PIN 2 // D4

struct Lightning {
    uint8_t pin;
    bool onState;
    Lightning(uint8_t pin, bool onState=LOW):
    pin(pin), onState(onState)
    {
        pinMode(pin, OUTPUT);
        off();
    }
    void off() const { digitalWrite(pin, !onState); };
    void on() const { digitalWrite(pin, onState); };
    void toggle() const { digitalWrite( pin, !digitalRead(pin));}
};


OneButton button(BUTTON_PIN, true);  // true = INPUT_PULLUP
const Lightning lightning(LIGHT_PIN);

enum DisplayStyles {
    Normal,
    BigClock,
    FullInfo,
};
// Переменная для стилей отображения
DisplayStyles displayStyle = Normal;
DisplayStyles next(DisplayStyles& s){
    switch (s)
    {
    case FullInfo: 
        s = Normal;
        break;
    default:
        s = DisplayStyles( s + 1);
    } 
    return s;
};

namespace Button {
    void lightToggle(){ lightning.toggle();  }
    void nextStyle(){ next(displayStyle); }

    void init() {
        button.attachClick( lightToggle );    // Одиночное нажатие
        //button.attachDoubleClick(doubleClick);  // Двойное нажатие
        button.attachLongPressStart( nextStyle ); // Долгое нажатие
    }
};