#pragma once

#include <WiFiManager.h>

//#define POINT_STOP_WMPARAMETERS

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
        using ValueChangedCallback = std::function<void(uint8_t value)>;
        
        SliderControl(const String& id, const String& label, 
                    uint8_t value = 50, 
                    uint8_t min = 0, 
                    uint8_t max = 100)
            : id(id), label(label), value(value), minVal(min), maxVal(max) {
            list.push_back(this);   
            buildHTML();
        }
    
        ~SliderControl(){
            delete[] htmlContent;
            list.erase(
                std::remove(list.begin(), list.end(), this),
                list.end()
            );
            if ( list.empty() ) std::vector<SliderControl *>().swap(list);
        };
    
        void setCallback(ValueChangedCallback callback) {
            this->callback = callback;
        }
    
        const char* getHTML() const {
            return htmlContent;
        }
    
        uint8_t getValue() const {
            String val = _wm->server->arg(id);
            return val.length() ? constrain(val.toInt(), minVal, maxVal) : value;
        }
        // Статический метод для добавления CSS/JS
        static bool init(WiFiManager& wm) {
            if ( _wm != nullptr ){
                if( _wm != &wm ) return false;
                else return true;
            }
            _wm = &wm;
            wm.setCustomHeadElement(getSliderCssJs());
            return true;    
        }
    
         static void webServerCallback(){
            if (_wm->server->hasArg("id") && _wm->server->hasArg("value")) {
                String id = _wm->server->arg("id");
                uint8_t value = _wm->server->arg("value").toInt();
                bool isValidId = false;
                for( auto slider : list) {
                    if ( id.equals( slider->id)) {
                        isValidId = true;
                        slider->callback(value);
                        break;
                    }
                }
                if( isValidId )
                    _wm->server->send(200, "text/plain", "OK");
                else
                    _wm->server->send(404, "text/plain", "Wrong parameter");
            } else {
                _wm->server->send(400, "text/plain", "Bad Request");
            }
        }
    
        static void addWebServerCallback(){
            assert(_wm && "Call SliderControl::init(WiFiManager&) first" );
            _wm->setWebServerCallback( [](){
                _wm->server->on( _httpPath(), webServerCallback); }
            );
        };
    
    
        void process() {
            if (_wm->server->hasArg(id)) {
                uint8_t newValue = constrain(_wm->server->arg(id).toInt(), minVal, maxVal);
                if (callback && newValue != lastValue) {
                    callback( newValue);
                    lastValue = newValue;
                }
            }
        }
        
        
    private:
        String id;
        String label;
        uint8_t value;
        uint8_t minVal;
        uint8_t maxVal;
        uint8_t lastValue = 0;
        char* htmlContent = nullptr;
        ValueChangedCallback callback = nullptr;
        static std::vector<SliderControl *> list;
        static WiFiManager * _wm;
        static const char* _httpPath(){ return R"(/slider)";};
    
        void buildHTML() {
            String html;
            html.reserve(400);
            
            html += "<div class='slider-panel'>";
            html += "<div class='slider-header'>";
            html += "<label>" + label + "</label>";
            html += "<span class='slider-value' id='" + id + "-value'>";
            html += String(value);
            html += "</span>";
            html += "</div>";
            html += "<input type='range' class='styled-slider' name='" + id + "'";
            html += " min='" + String(minVal) + "'";
            html += " max='" + String(maxVal) + "'";
            html += " value='" + String(value) + "'";
            html += " oninput=\"updateSlider('" + id + "')\">";
            html += "</div>";
    
            htmlContent = new char[html.length() + 1];
            strcpy(htmlContent, html.c_str());
        }
    
        static const char* getSliderCssJs(){    
                return R"(
        <style>
        .slider-panel {
            background: #f8f9fa;
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 16px;
        }
        .slider-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
        }
        .styled-slider {
            width: 100%;
            height: 6px;
            background: #ddd;
            border-radius: 3px;
        }
    </style>
    <script>
        function updateSlider(id) {
        var slider = document.querySelector('input[name="' + id + '"]');
        var display = document.getElementById(id + '-value');
        if (slider && display) {
            display.textContent = slider.value;
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/slider?id=' + id + '&value=' + slider.value, true);
            xhr.send();
        }
    }
    </script>
    )"; }
        
    };
    WiFiManager * SliderControl::_wm = nullptr;
    std::vector<SliderControl *> SliderControl::list = {};
    
