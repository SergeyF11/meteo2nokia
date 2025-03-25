#pragma once

#include <EEPROM.h>
#include <Arduino.h>
#include "crc32_tmpl.h"

// Размер ключа API (32 символа)
#define API_KEY_SIZE 32U
// Размер контрольной суммы (CRC32)
#define CRC_SIZE 4U

struct EepromData {
  private:
    char openWeatherApi[API_KEY_SIZE] = {0};
    char geolocationApi[API_KEY_SIZE] = {0};
    uint8_t contrast1 = 50;  // Значение контраста по умолчанию
    uint8_t contrast2 = 50;  // Значение контраста по умолчанию
    uint32_t _crc;

    size_t printBytes(Print& p, const char * bytes, const size_t len){
      for( size_t i=0; i<len; i++){
        if ( bytes[i] == '\0' ) break;
        p.print( bytes[i]);
      }
      return  p.println() + len;
    };
    uint32_t crc() const { 
      return CRC32T::calculate( (*this), offsetof(EepromData, _crc) ); 
     };
  public:
   size_t printTo(Print& p){
    size_t out = p.print("Weather key=");
    out += printBytes(p, openWeatherApi, API_KEY_SIZE);
    out = p.print("Geo key=");
    out += printBytes(p, geolocationApi, API_KEY_SIZE);
    out += p.print("Contrast1=");
    out += p.print(contrast1);
    out += p.print(", Contrast2=");
    out += p.print(contrast2);
    out += p.print(", CRC=");
    out += p.print(_crc); 
    out += p.println( valid() ? " valid" : " invalid");
    return out;
   };
   
   EepromData(){
    _crc = 123456789;
   };
   
   EepromData(const char * weatherKey, const char * geoKey=nullptr, 
              uint8_t c1=50, uint8_t c2=50) {
    auto len = min( strlen(weatherKey), API_KEY_SIZE);
    strncpy(this->openWeatherApi, weatherKey, len);
    if ( geoKey ) {
      len = min( strlen(geoKey), API_KEY_SIZE);
      strncpy(this->geolocationApi, geoKey, len);
    }
    this->contrast1 = c1;
    this->contrast2 = c2;
    _crc = crc();
   };
   
   const char * getWeatherKey() const {
    if ( crc() != this->_crc ) return nullptr;
    return openWeatherApi;
   };
   
   const char * getGeoKey() const {
    if ( crc() != this->_crc ) return nullptr;
    return geolocationApi;
   };
   
   uint8_t getContrast1() const {
    if ( crc() != this->_crc ) return 50;
    return contrast1;
   };
   
   uint8_t getContrast2() const {
    if ( crc() != this->_crc ) return 50;
    return contrast2;
   };
   
   bool valid() const { return crc() == _crc; };

   bool copyWeatherKeyTo(char * dest) const {
    if ( dest && strncpy(dest, openWeatherApi, API_KEY_SIZE) ){
      dest[API_KEY_SIZE] ='\0';
      return true;
    }
    return false;
   };
   
   bool copyGeoKeyTo(char * dest) const {
    if ( dest && strncpy(dest, geolocationApi, API_KEY_SIZE) ){
      dest[API_KEY_SIZE] ='\0';
      return true;
    }
    return false;
   };
   
   bool setContrast(uint8_t c1, uint8_t c2) {
    contrast1 = c1;
    contrast2 = c2;
    _crc = crc();
    return true;
   }
};

bool loadEepromData(EepromData& data, const int eepromStart=0);
// Функция для сохранения всех данных в EEPROM
bool saveEepromData(const EepromData& eepromData, const int eepromStart=0) {
 if ( ! eepromData.valid() ) return false;
  EEPROM.begin(512 + (sizeof(EepromData)/512));
  const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(&eepromData);
  for (size_t i = 0; i < sizeof(EepromData); i++) {
      EEPROM.write(eepromStart + i, byteArray[i]);
  }
  EEPROM.commit();
  EEPROM.end();
  return true;
}

// Функция для сохранения только ключей API
bool saveApiKeys(const char* apiKey, const char* geoKey=nullptr, const int eepromStart=0) {
  EepromData currentData;
  if (!loadEepromData(currentData, eepromStart)) {
    currentData = EepromData("", "");
  }
  EepromData eepromData(apiKey, geoKey, currentData.getContrast1(), currentData.getContrast2());
  return saveEepromData(eepromData);
}

// Функция для сохранения только настроек контраста
bool saveContrastSettings(uint8_t c1, uint8_t c2, const int eepromStart=0) {
  EepromData currentData;
  if (!loadEepromData(currentData, eepromStart)) {
    currentData = EepromData("", "");
  }
  currentData.setContrast(c1, c2);
  return saveEepromData(currentData, eepromStart);
}

// Функция для загрузки всех данных из EEPROM
bool loadEepromData(EepromData& data, const int eepromStart) {
  EEPROM.begin(512 + (sizeof(EepromData)/512));
  uint8_t* byteArray = reinterpret_cast<uint8_t*>(&data);
  for (size_t i = 0; i < sizeof(EepromData); i++) {
      byteArray[i] = EEPROM.read(eepromStart + i);
  }
  bool valid = data.valid();
  EEPROM.end();
  return valid;
}

// Функция для загрузки только ключей API
bool loadApiKeys(char * weatherKey, char * geoKey, const int eepromStart=0) {
  EepromData data;
  bool valid = loadEepromData(data, eepromStart);
  Serial.println("EEPROM data loaded:"); data.printTo(Serial);
  if ( valid ) {
    valid =  data.copyWeatherKeyTo(weatherKey);
    valid = valid && data.copyGeoKeyTo(geoKey);
  } 
  return valid;
}

// Функция для загрузки только настроек контраста
bool loadContrastSettings(uint8_t &c1, uint8_t &c2, const int eepromStart=0) {
  EepromData data;
  bool valid = loadEepromData(data, eepromStart);
  if ( valid ) {
    c1 = data.getContrast1();
    c2 = data.getContrast2();
  } else {
    c1 = c2 = 50; // Значения по умолчанию
  }
  return valid;
}

// #pragma once

// #include <EEPROM.h>
// #include <Arduino.h>
// #include "crc32_tmpl.h"

// // Размер ключа API (32 символа)
// #define API_KEY_SIZE 32U
// // Размер контрольной суммы (CRC32)
// #define CRC_SIZE 4U

// struct EepromData {
//   private:
//     size_t printBytes(Print& p, const char * bytes, const size_t len){
//       for( size_t i=0; i<len; i++){
//         if ( bytes[i] == '\0' ) break;
//         p.print( bytes[i]);
//       }
//       return  p.println() + len;
//     };

//   public:
//    char openWeatherApi[API_KEY_SIZE] = {0};
//    char geolocationApi[API_KEY_SIZE] = {0};
//    uint32_t _crc;
//    size_t printTo(Print& p){
//     size_t out = p.print("Weather key=");
//     out += printBytes(p, openWeatherApi, API_KEY_SIZE);
//     out = p.print("Geo key=");
//     out += printBytes(p, geolocationApi, API_KEY_SIZE);
//     out += p.print("CRC=");
//     out += p.print(_crc); 
//     out += p.println( valid() ? " valid" : " invalid");
//     return out;
//    };
//    EepromData(){
//     _crc = 123456789;
//   };
//   EepromData(const char * weatherKey, const char * geoKey=nullptr) {
//     auto len = min( strlen(weatherKey), API_KEY_SIZE);
//     strncpy(this->openWeatherApi, weatherKey, len);
//     if ( geoKey ) {
//       len = min( strlen(geoKey), API_KEY_SIZE);
//       strncpy(this->geolocationApi, geoKey, len);
//     }
//    _crc = crc();
//   };
//   const char * getWeatherKey() const {
//     if ( crc() != this->_crc ) nullptr;
//     return openWeatherApi;
//   };
//   const char * getGeoKey() const {
//     if ( crc() != this->_crc ) nullptr;
//     return geolocationApi;
//   };
//   uint32_t crc() const { return CRC32T::calculate( (*this), offsetof(EepromData, _crc) ); };
//   bool valid() const { return crc() == _crc; };

//   bool copyWeatherKeyTo(char * dest) const {
//     if ( dest && strncpy(dest, openWeatherApi, API_KEY_SIZE) ){
//       dest[API_KEY_SIZE] ='\0';
//       return true;
//     } else
//     return false;
//   };
//   bool copyGeoKeyTo(char * dest) const {
//     if ( dest && strncpy(dest, geolocationApi, API_KEY_SIZE) ){
//       dest[API_KEY_SIZE] ='\0';
//       return true;
//     } else
//     return false;
//   };
  
// };

// // Функция для сохранения ключа API в EEPROM
// bool saveApiKeys(const EepromData& eepromKeys, const int eepromStart=0) {
//  if ( ! eepromKeys.valid() ) return false;
//   // Инициализация EEPROM
//   EEPROM.begin(512 + (sizeof(EepromData)/512));
//   // Записываем ключ и контрольную сумму в EEPROM
//     const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(&eepromKeys);
//     for (size_t i = 0; i < sizeof(EepromData); i++) {
//         EEPROM.write(eepromStart + i, byteArray[i]);
//     }

//   //EEPROM.write( reinterpret_cast<const uint8_t*>( &eepromKey ), sizeof(eepromKey ));

//   EEPROM.commit();
//   EEPROM.end();
//   return true;
// }

// // Функция для сохранения ключа API в EEPROM
// bool saveApiKey(const char* apiKey, const char* geoKey=nullptr, const int eepromStart=0) {
//   EepromData eepromKeys(apiKey, geoKey );
//   return saveApiKeys( eepromKeys);
// }


// // Функция для извлечения ключа API из EEPROM
// bool loadApiKeys(EepromData& keysData, const int eepromStart=0) {
//   // Инициализация EEPROM
//   EEPROM.begin(512 + (sizeof(EepromData)/512));
//   // Читаем ключ из EEPROM
//   uint8_t* byteArray = reinterpret_cast<uint8_t*>(&keysData);
//   for (size_t i = 0; i < sizeof(EepromData); i++) {
//       byteArray[i] = EEPROM.read(eepromStart + i);
//   }

//   //auto res = EEPROM.read( reinterpret_cast<const uint8_t*>( &eepromKey ), sizeof(eepromKey ));

//   bool valid = keysData.valid();
//   return valid;
// }

// bool loadApiKeys(char * weatherKey, char * geoKey, const int eepromStart=0) {
//   EepromData data;
//   // Инициализация EEPROM
//   bool valid = loadApiKeys( data, eepromStart );
//   Serial.println("EEPROM data loaded:"); data.printTo(Serial);
//   if ( valid ) {
//     valid =  data.copyWeatherKeyTo(weatherKey);
//     valid += data.copyGeoKeyTo(geoKey);
//   } 
//   // loadedKeys.copyWeatherKeyTo(weatherKey);
//   // loadedKeys.copyGeoKeyTo(geoKey);

//   return valid;
// }