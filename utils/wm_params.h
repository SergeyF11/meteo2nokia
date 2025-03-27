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


#pragma once
#include <WiFiManager.h>


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


class SliderControl {
public:
    // Конструктор с настройками
    SliderControl(const char* id, const char* label, uint8_t value = 50, 
                 uint8_t min = 0, uint8_t max = 100)
        : id(id), label(label), value(value), minVal(min), maxVal(max) {
        buildHTML();
    }

    // Получение HTML для вставки
    const char* getHTML() const { return htmlContent; }

    // Получение значения после отправки формы
    uint8_t getValue(WiFiManager* wm) const {
        String val = wm->server->arg(id);
        return val.length() ? constrain(val.toInt(), minVal, maxVal) : value;
    }

    // Статический метод для добавления CSS/JS
    static void setupStyle(WiFiManager& wm) {
        wm.setCustomHeadElement(getStyleCSS());
    }

private:
    const char* id;
    const char* label;
    uint8_t value;
    uint8_t minVal;
    uint8_t maxVal;
    char* htmlContent = nullptr;

    void buildHTML() {
        String html;
        html.reserve(350);
        
        html += "<div class='slider-panel'>";
        html +=   "<div class='slider-header'>";
        html +=     "<label>"; html += label; html += "</label>";
        html +=     "<span class='slider-value' id='"; html += id; html += "-value'>";
        html +=       value;
        html +=     "</span>";
        html +=   "</div>";
        html +=   "<input type='range' class='styled-slider' name='"; html += id; html += "'";
        html +=     " min='"; html += minVal;
        html +=     "' max='"; html += maxVal;
        html +=     "' value='"; html += value;
        html +=     "' oninput=\"updateSliderValue('"; html += id; html += "')\">";
        html += "</div>";

        htmlContent = new char[html.length() + 1];
        strcpy(htmlContent, html.c_str());
    }

    static const char* getStyleCSS() {
        return R"(
<style>
.slider-panel {
    background: #f8f9fa;
    padding: 12px;
    border-radius: 8px;
    margin-bottom: 16px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}
.slider-header {
    display: flex;
    justify-content: space-between;
    margin-bottom: 8px;
}
.slider-header label {
    font-weight: 500;
    color: #333;
}
.slider-value {
    font-family: monospace;
    font-size: 1.1em;
}
.styled-slider {
    width: 100%;
    height: 6px;
    background: #dee2e6;
    border-radius: 3px;
    outline: none;
    -webkit-appearance: none;
}
.styled-slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 18px;
    height: 18px;
    background: #4caf50;
    border-radius: 50%;
    cursor: pointer;
    transition: background 0.15s;
}
.styled-slider::-webkit-slider-thumb:hover {
    background: #3e8e41;
}
</style>
<script>
function updateSliderValue(id) {
    const slider = document.querySelector(`input[name="${id}"]`);
    const output = document.getElementById(`${id}-value`);
    if(slider && output) output.textContent = slider.value;
}
</script>
)";
    }
};