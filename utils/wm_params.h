#pragma once

#include <WiFiManager.h>

#define POINT_STOP_WMPARAMETERS

#ifdef POINT_STOP_WMPARAMETERS
#define pointStop(ms, fmt, ...)                               \
  {                                                           \
    Serial.printf("[%d] %s ", __LINE__, __PRETTY_FUNCTION__); \
    Serial.printf(fmt, ##__VA_ARGS__);                        \
    Serial.flush();                                                          \
    delay(ms);                                                \
  }
#else
#define pointStop(ms, fmt, ...)
#endif


class SliderControl {
    private:
        const char* id;
        const char* label;
        uint8_t value;
        uint8_t minVal;
        uint8_t maxVal;
        char* htmlContent;
    
        void buildHTML() {
            String html;
            html.reserve(350);
            
            html += "<div class='slider-container' style='margin:15px 0'>";
            html += "<div style='display:flex;justify-content:space-between'>";
            html += "<label style='font-weight:500'>"; html += label; html += "</label>";
            html += "<span id='"; html += id; html += "-value'>"; html += value; html += "</span>";
            html += "</div>";
            html += "<input type='range' name='"; html += id; 
            html += "' min='"; html += minVal;
            html += "' max='"; html += maxVal;
            html += "' value='"; html += value;
            html += "' style='width:100%'";
            html += " oninput=\"document.getElementById('"; html += id; html += "-value').textContent=this.value\"";
            html += ">";
            html += "</div>";
    
            htmlContent = new char[html.length() + 1];
            strcpy(htmlContent, html.c_str());
        }
    
    public:
    static char *css;
        SliderControl(const char* _id, const char* _label, uint8_t _value,
                     uint8_t _minVal = 0, uint8_t _maxVal = 100)
            : id(_id), label(_label), value(_value), 
              minVal(_minVal), maxVal(_maxVal) {
            buildHTML();
        }
    
        ~SliderControl() {
            delete[] htmlContent;
        }
    
        const char* getHTML() const {
            return htmlContent;
        }
    
        uint8_t getValue(WiFiManager* wm) const {
            String val = wm->server->arg(id);
            return val.length() ? constrain(val.toInt(), minVal, maxVal) : value;
        }
    };
char * SliderControl::css = R"(<style>
.slider-container {
    background: #f8f8f8;
    padding: 10px;
    border-radius: 8px;
    margin-bottom: 15px;
}
input[type=range] {
    width: 100%;
    height: 6px;
    background: #ddd;
    border-radius: 3px;
    margin-top: 8px;
}
input[type=range]::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 18px;
    height: 18px;
    background: #4CAF50;
    border-radius: 50%;
    cursor: pointer;
}
</style>)";

class IntParameter : public WiFiManagerParameter {
    public:
        IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
            : WiFiManagerParameter("") {
            init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
        }
    
        long getValue() {
            return String(WiFiManagerParameter::getValue()).toInt();
        }
};


#pragma once
#include <WiFiManager.h>

class SliderParameter : public WiFiManagerParameter {
private:
    const uint8_t minVal;
    const uint8_t maxVal;
    char* htmlContent;
    char idCopy[16];

    void buildHtml(const char* id, const char* label, uint8_t value) {
        strncpy(idCopy, id, sizeof(idCopy)-1);
        
        String html;
        html.reserve(400);
        
        // Основной контейнер
        html += "<div class='wm-slider-container'>";
        
        // Лейбл и текущее значение
        html += "<div style='display:flex;justify-content:space-between;margin-bottom:5px'>";
        html += "<label style='font-weight:500'>"; html += label; html += "</label>";
        html += "<span id='"; html += id; html += "-value'>"; html += value; html += "</span>";
        html += "</div>";
        
        // Слайдер с правильным name
        html += "<input type='range' id='"; html += id; 
        html += "' name='"; html += id;
        html += "' min='"; html += minVal;
        html += "' max='"; html += maxVal;
        html += "' value='"; html += value;
        html += "' style='width:100%'";
        html += " oninput=\"document.getElementById('"; html += id; html += "-value').textContent=this.value\"";
        html += ">";
        
        html += "</div>";

        htmlContent = new char[html.length() + 1];
        strcpy(htmlContent, html.c_str());
    }

public:
    static const char* slider_js_css;
    SliderParameter()
        : WiFiManagerParameter(),
        minVal(0),
        maxVal(100) {};
    SliderParameter(const char* id, const char* label, uint8_t value,
                  uint8_t minVal = 0, uint8_t maxVal = 100)
        : WiFiManagerParameter(id, label, "", 0, "", WFM_NO_LABEL),
          minVal(minVal),
          maxVal(maxVal) {
        
        buildHtml(id, label, value);
    }

    ~SliderParameter() {
        delete[] htmlContent;
    }

    const char* getCustomHTML() const {
        return htmlContent;
    }

    uint8_t getValue() const {
        const char* val = WiFiManagerParameter::getValue();
        if(val && strlen(val) > 0) {
            return constrain(atoi(val), minVal, maxVal);
        }
        return (minVal + maxVal) / 2;
    }
};

const char* SliderParameter::slider_js_css = R"(
<script>
document.addEventListener('DOMContentLoaded', function() {
    const sliders = document.querySelectorAll('input[type=range]');
    sliders.forEach(slider => {
        const output = document.getElementById(slider.id + '-value');
        if(output) output.textContent = slider.value;
        
        slider.addEventListener('input', function() {
            if(output) output.textContent = this.value;
        });
    });
});
</script>
<style>
    .wm-slider-container {
      background: #f8f8f8;
      padding: 10px;
      border-radius: 8px;
      margin-bottom: 15px;
    }
    input[type=range] {
      width: 100%;
      height: 6px;
      background: #ddd;
      border-radius: 3px;
      margin: 8px 0;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 18px;
      height: 18px;
      background: #4CAF50;
      border-radius: 50%;
      cursor: pointer;
    }
    </style>
)";

// class SliderParameter : public WiFiManagerParameter {
//     private:
//         const uint8_t minVal;
//         const uint8_t maxVal;
//         char* htmlContent;
//         char idCopy[32]; // Храним ID для JavaScript
//         char valueStr[5];

//         void buildHtml(const char* id, const char* label, uint8_t value) {
//             strncpy(idCopy, id, sizeof(idCopy)-1);
            
//             String html;
//             html.reserve(350);
            
//             // Основной контейнер
//             html += "<div class='wm-slider-container' style='margin:15px 0'>";
            
//             // Строка с названием и значением
//             html += "<div style='display:flex;justify-content:space-between;align-items:center;margin-bottom:8px'>";
//             html += "<label for='"; html += id; html += "' style='font-weight:500'>"; html += label; html += "</label>";
//             html += "<output id='"; html += id; html += "-value' style='font-family:monospace;font-size:1.1em'>";
//             html += value; html += "</output>";
//             html += "</div>";
            
//             // Сам слайдер
//             html += "<input type='range' id='"; html += id;
//             html += "' name='"; html += id;
//             html += "' min='"; html += minVal;
//             html += "' max='"; html += maxVal;
//             html += "' value='"; html += value;
//             html += "' style='width:100%;height:8px'";
//             html += " oninput=\"document.getElementById('"; html += id; html += "-value').value=this.value\"";
//             html += ">";
            
//             html += "</div>";
    
//             htmlContent = new char[html.length() + 1];
//             strcpy(htmlContent, html.c_str());
//         }
        
//         uint8_t toValue(const char* str) const {
//             if (!str || strlen(str) == 0) return (maxVal-minVal)/2;
//             int val = atoi(str);
//             if (val < minVal) return minVal;
//             if (val > maxVal) return maxVal;
//             return static_cast<uint8_t>(constrain(val, minVal, maxVal));
//             //return static_cast<uint8_t>(val);
//         }

//     public:
//     static const char *tultip_js;
//         SliderParameter()
//         : WiFiManagerParameter(""),
//         minVal(0U), maxVal(100U), 
//         idCopy{0}, valueStr{0}
//          {};
        
//         SliderParameter(const char* id, const char* label, const uint8_t value,
//                       const uint8_t minVal = 0, const uint8_t maxVal = 100)
//             : //WiFiManagerParameter(""),
//             WiFiManagerParameter("", "", "", 0, "", WFM_LABEL_BEFORE),
//             minVal(minVal), maxVal(maxVal), htmlContent(nullptr),
//             idCopy{0}, valueStr{0} {
                
//             buildHtml(id, label, value);
            
//             // Ключевое изменение: используем кастомный HTML как весь параметр
//             //init("", htmlContent, "", 0, "", WFM_LABEL_BEFORE);
//             init(id, htmlContent, valueStr, sizeof(valueStr)-1, "", WFM_LABEL_BEFORE);
//         }
    
//         ~SliderParameter() {
//             delete[] htmlContent;
//         }
        
//         // Запрещаем копирование
//         SliderParameter(const SliderParameter&) = delete;
//         SliderParameter& operator=(const SliderParameter&) = delete;

//         uint8_t getValue() {
//             const char* val = WiFiManagerParameter::getValue();
//             return toValue(val ? val : valueStr);
//         }
//         // uint8_t getValue() {
//         //     // Получаем значение через JavaScript-совместимый способ
//         //     String val = WiFiManagerParameter::getValue();
//         //     if(val.length() == 0) {
//         //         // Если значение пустое, возвращаем минимум
//         //         return (maxVal -minVal)/2;
//         //     }
//         //     return toValue(val.c_str());
//         // }
//     };

// class SliderParameter : public WiFiManagerParameter {
//     private:
//         const uint8_t minVal;
//         const uint8_t maxVal;
//         char* htmlContent;
//         char idCopy[16]; // Храним ID для доступа

    
//         void buildHtml(const char* id, const char* label, uint8_t value) {
//             strncpy(idCopy, id, sizeof(idCopy)-1);
            
//             String html;
//             html.reserve(400);
            
//             // Контейнер
//             html += "<div style='margin:15px 0'>";
            
//             // Лейбл и значение
//             html += "<div style='display:flex;justify-content:space-between;margin-bottom:5px'>";
//             html += "<label style='font-weight:500'>"; html += label; html += "</label>";
//             html += "<span id='"; html += id; html += "-value'>"; html += value; html += "</span>";
//             html += "</div>";
            
//             // Слайдер
//             html += "<input type='range' id='"; html += id; 
//             html += "' name='"; html += id;
//             html += "' min='"; html += minVal;
//             html += "' max='"; html += maxVal;
//             html += "' value='"; html += value;
//             html += "' style='width:100%'";
//             html += " oninput=\"document.getElementById('"; html += id; html += "-value').textContent=this.value\"";
//             html += ">";
            
//             html += "</div>";
    
//             htmlContent = new char[html.length() + 1];
//             strcpy(htmlContent, html.c_str());
//         }

//     public:
//         static const char * tultip_js;
//         SliderParameter() : WiFiManagerParameter(""),
//             minVal(0), maxVal(100) {};
//         SliderParameter(const char* id, const char* label, uint8_t value,
//                         uint8_t minVal = 0, uint8_t maxVal = 100)
//             : WiFiManagerParameter(id, "", "", 0, "", WFM_NO_LABEL), // Ключевое изменение!
//             minVal(minVal),
//             maxVal(maxVal) {
            
//             buildHtml(id, label, value);
//             init("", htmlContent, "", 0, "", WFM_NO_LABEL); // Важно!
//         }

    
//         ~SliderParameter() {
//             delete[] htmlContent;
//         }
    
//         uint8_t getValue() {
//             // Получаем значение напрямую из запроса
//             auto val = WiFiManagerParameter::getValue();
//             if( strlen(val) > 0) {
//                 return constrain( atoi(val), minVal, maxVal);
//             }
//             return (maxVal- minVal)/2; // Значение по умолчанию
//         }
//     };
// const char * SliderParameter::tultip_js = R"(
// <style>
// input[type=range] {
//     -webkit-appearance: none;
//     width: 100%;
//     height: 6px;
//     background: #ddd;
//     border-radius: 3px;
//     margin: 10px 0;
// }
// input[type=range]::-webkit-slider-thumb {
//     -webkit-appearance: none;
//     width: 18px;
//     height: 18px;
//     background: #4CAF50;
//     border-radius: 50%;
//     cursor: pointer;
// }
// </style>
// <script>
// function updateAllSliders() {
//     document.querySelectorAll('input[type=range]').forEach(slider => {
//         const output = document.getElementById(slider.id + '-value');
//         if(output) output.textContent = slider.value;
//     });
// }
// window.addEventListener('load', updateAllSliders);
// </script>
// )";

// class SliderParameter : public WiFiManagerParameter { 
//     private:
//         const uint8_t minVal;
//         const uint8_t maxVal;
//         char _type[100] = {0};

//         uint8_t toValue(const char * str) const { 
//             auto val = atoi(str);
//             if(  minVal > val ) return minVal;
//             if( maxVal < val ) return maxVal;
//             return uint8_t(val);
//         }
//     public:
//     static const char *tultip_js;

//     SliderParameter() : minVal(0U), maxVal(100U), WiFiManagerParameter("") {};
//     SliderParameter(const char *id, const char *placeholder, const uint8_t value, 
//         const uint8_t minVal = 0, const uint8_t maxVal = 100, const uint8_t step = 1 )
//             : minVal(minVal), maxVal(maxVal), WiFiManagerParameter("") {
//             String type("type=\"range\" min=\"");
//                 type += minVal;
//                 type += "\" max=\"";
//                 type += maxVal;
//                 type += "\" step=\"";
//                 type += step;
//                 type += "\" oninput=\"updateTooltip(this)\""; // Добавляем обработчик
// //                type += " title=\"Текущее значение: ";
//                 type += " title=\"";
//                 type += value;
//                 type += "\""; // Подсказка при наведении
// //                type += " style=\"width: 100%;\""; // Опционально: растягиваем на всю ширину
//             strcpy(_type, type.c_str());
//             _type[ type.length()] = '\0';
//             init(id, placeholder, String(value).c_str(), 4, _type, WFM_LABEL_BEFORE);
//         }
    
//         uint8_t getValue() {
//             pointStop(0,"getValue\n");
//             return toValue( WiFiManagerParameter::getValue() );
//         }
// };
// const char * SliderParameter::tultip_js = R"(
// <script>
// function updateTooltip(slider) {
//  slider.setAttribute('title', slider.value);
// }
// </script>
// )";

class SliderParameterTultip : public WiFiManagerParameter { 
    private:
        const uint8_t minVal;
        const uint8_t maxVal;
        char * htmlContent;

        uint8_t toValue(const char * str) const { 
            auto val = atoi(str);
            if(  minVal > val ) return minVal;
            if( maxVal < val ) return maxVal;
            return uint8_t(val);
        }

        void buildHtml(const char* id, const char* label, uint8_t value) {
            String html;
            html.reserve(256); // Предварительное выделение памяти
            
            html += "<div style='margin:12px 0;width:100%'>";
            html += "<div style='display:flex;justify-content:space-between;margin-bottom:4px'>";
            html += "<label style='font-weight:bold'>";
            html += label;
            html += "</label>";
            html += "<span id='";
            html += id;
            html += "-value' style='font-family:monospace'>";
            html += value;
            html += "</span>";
            html += "</div>";
            html += "<input type='range' id='";
            html += id;
            html += "' name='";
            html += id;
            html += "' min='";
            html += minVal;
            html += "' max='";
            html += maxVal;
            html += "' value='";
            html += value;
            html += "' style='width:100%' oninput=\"document.getElementById('";
            html += id;
            html += "-value').textContent=this.value;updateTooltip(this)\">";
            html += "</div>";
    
            htmlContent = new char[html.length() + 1];
            strcpy(htmlContent, html.c_str());
        }
    public:
    static const char *tultip_js;
SliderParameterTultip() : minVal(0U), maxVal(100U), WiFiManagerParameter("") {};
SliderParameterTultip(const char *id, const char *label, const uint8_t value, 
    const uint8_t minVal = 0, const uint8_t maxVal = 100, const uint8_t step = 1 )
        : minVal(minVal), maxVal(maxVal), WiFiManagerParameter("") {
               // Формируем HTML во временном String
        buildHtml(id, label, value);
        //init(NULL, htmlContent, NULL, 0, NULL, WFM_LABEL_BEFORE);
        init("", htmlContent, "", 0, "", WFM_LABEL_BEFORE); 
        //        init("", "", "", 0, htmlContent, WFM_LABEL_BEFORE);
        //init(NULL, NULL, NULL, 0, htmlContent, WFM_LABEL_BEFORE);
    }
    ~SliderParameterTultip() {
        pointStop(0,"Destructor\n");
        delete[] htmlContent; // Освобождаем память в деструкторе
    }

    SliderParameterTultip(const SliderParameterTultip&) = delete; // Запрещаем копирование
    SliderParameterTultip& operator=(const SliderParameterTultip&) = delete;

    uint8_t getValue() {
        pointStop(0,"GetValue\n");
        return toValue( WiFiManagerParameter::getValue() );
    }
};

const char * SliderParameterTultip::tultip_js = R"(
<script>
function updateTooltip(slider) {
    slider.setAttribute('title', slider.value);
}
</script>
<style>
input[type=range] {
  -webkit-appearance: none;
  height: 6px;
  background: #ddd;
  border-radius: 3px;
}
input[type=range]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 18px;
  height: 18px;
  background: #4CAF50;
  border-radius: 50%;
  cursor: pointer;
}
</style>;
)";
/*  slider.setAttribute('title', 'Текущее значение: ' + slider.value); */
/*    const tooltip = document.getElementById(slider.id + '-tooltip');
    if (tooltip) tooltip.textContent = slider.value;*/

class SeparatorParameter : public WiFiManagerParameter {
    private:
    char *custom;
    public:
      SeparatorParameter(const char *label)
        : WiFiManagerParameter("") {
        custom = new char[30+strlen(label)];
        strcpy(custom, "<div class=\"separator\">");
        strcat(custom, label);
        strcat(custom, "</div>");
        init(NULL, NULL, NULL, 1, custom, WFM_LABEL_BEFORE);
      }
      ~SeparatorParameter(){
        pointStop(0,"Destructor\n");
        delete[] custom ;
      }
};