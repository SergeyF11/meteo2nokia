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
        char _type[45] = {0};
        uint8_t toValue(const char * str) const { 
            auto val = atoi(str);
            if(  minVal > val ) return minVal;
            if( maxVal < val ) return maxVal;
            return uint8_t(val);
        }
    public:
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
                type += '"';
            strcpy(_type, type.c_str());
            _type[ type.length()] = '\0';
            init(id, placeholder, String(value).c_str(), 4, _type, WFM_LABEL_BEFORE);
        }
    
        uint8_t getValue() {
            return toValue( WiFiManagerParameter::getValue() );
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
        delete( custom );
      }
};