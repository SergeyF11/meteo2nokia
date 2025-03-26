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

// class SliderParameter : public WiFiManagerParameter { // WiFiManagerParameter(
//     // "contrast1", "Дисплей 1",
//     // contrast1_str, 4, "type=\"range\" min=\"20\" max=\"99\" step=\"1\"");
//     private:
//        // const char *type_tmpl = R"raw(type="range" min="%d" max="%d" step="%d")raw";
//         const uint8_t minVal;
//         const uint8_t maxVal;
//         uint8_t normalize(const long val){ 
//             if(  minVal > val ) return minVal;
//             if( maxVal < val ) return maxVal;
//             return uint8_t(val);
//         }
//     public:
//     //SliderParameter() : WiFiManagerParameter("") {};
//     SliderParameter(const char *id, const char *placeholder, uint8_t value, 
//         const uint8_t minVal = 0, const uint8_t maxVal = 100, const uint8_t step = 1 )
//             : minVal(minVal), maxVal(maxVal), WiFiManagerParameter("") {
//             String type("type=\"range\" min=\"");
//                 type += minVal;
//                 type += "\" max=\"";
//                 type += maxVal;
//                 type += "\" step=\"";
//                 type += step;
//                 type += '"';
//             init(id, placeholder, String(value).c_str(), 4, type.c_str(), WFM_LABEL_BEFORE);
//         }
    
//         uint8_t getValue() {
//             return normalize( String(WiFiManagerParameter::getValue()).toInt() );
//         }
// };

class SeparatorParameter : public WiFiManagerParameter {
    public:
      SeparatorParameter(const char *label)
        : WiFiManagerParameter() {
        char *custom = new char[55];
        strcpy(custom, "<div class=\"separator\">");
        strcat(custom, label);
        strcat(custom, "</div>");
        init(NULL, NULL, NULL, 1, custom, WFM_LABEL_BEFORE);
      }
};