#pragma once

#include <WiFiManager.h>

// class IntParameter : public WiFiManagerParameter {
//     public:
//         IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
//             : WiFiManagerParameter("") {
//             init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
//         }
    
//         long getValue() {
//             return String(WiFiManagerParameter::getValue()).toInt();
//         }
// };

class SliderParameter : public WiFiManagerParameter { 
    private:
        const uint8_t minVal;
        const uint8_t maxVal;
        char _type[100] = {0};
        uint8_t toValue(const char * str) const { 
            auto val = atoi(str);
            if(  minVal > val ) return minVal;
            if( maxVal < val ) return maxVal;
            return uint8_t(val);
        }
    public:
    static const char *tultip_js;

    SliderParameter() : minVal(0U), maxVal(100U), WiFiManagerParameter("") {};
    SliderParameter(const char *id, const char *placeholder, const uint8_t value, 
        const uint8_t minVal = 0, const uint8_t maxVal = 100, const uint8_t step = 1 )
            : minVal(minVal), maxVal(maxVal), WiFiManagerParameter("") {
            String type("type=\"range\" min=\"");
                type += minVal;
                type += "\" max=\"";
                type += maxVal;
                type += "\" step=\"";
                type += step;
                type += "\" oninput=\"updateTooltip(this)\""; // Добавляем обработчик
//                type += " title=\"Текущее значение: ";
                type += " title=\"";
                type += value;
                type += "\""; // Подсказка при наведении
//                type += " style=\"width: 100%;\""; // Опционально: растягиваем на всю ширину
            strcpy(_type, type.c_str());
            _type[ type.length()] = '\0';
            init(id, placeholder, String(value).c_str(), 4, _type, WFM_LABEL_BEFORE);
        }
    
        uint8_t getValue() {
            return toValue( WiFiManagerParameter::getValue() );
        }
};
const char * SliderParameter::tultip_js = R"(
<script>
function updateTooltip(slider) {
 slider.setAttribute('title', slider.value);
}
</script>
)";

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
        //init("", htmlContent, "", 0, "", WFM_LABEL_BEFORE); 
        //        init("", "", "", 0, htmlContent, WFM_LABEL_BEFORE);
        init(NULL, NULL, NULL, 0, htmlContent, WFM_LABEL_BEFORE);
    }
    ~SliderParameterTultip() {
        delete[] htmlContent; // Освобождаем память в деструкторе
    }

    SliderParameterTultip(const SliderParameterTultip&) = delete; // Запрещаем копирование
    SliderParameterTultip& operator=(const SliderParameterTultip&) = delete;
    uint8_t getValue() {
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
        delete[] custom ;
      }
};