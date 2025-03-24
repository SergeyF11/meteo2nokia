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
    size_t printBytes(Print& p, const char * bytes, const size_t len){
      for( size_t i=0; i<len; i++){
        if ( bytes[i] == '\0' ) break;
        p.print( bytes[i]);
      }
      return  p.println() + len;
    };

  public:
   char openWeatherApi[API_KEY_SIZE] = {0};
   char geolocationApi[API_KEY_SIZE] = {0};
   uint32_t _crc;
   size_t printTo(Print& p){
    size_t out = p.print("Weather key=");
    out += printBytes(p, openWeatherApi, API_KEY_SIZE);
    out = p.print("Geo key=");
    out += printBytes(p, geolocationApi, API_KEY_SIZE);
    out += p.print("CRC=");
    out += p.print(_crc); 
    out += p.println( valid() ? " valid" : " invalid");
    return out;
   };
   EepromData(){
    _crc = 123456789;
  };
  EepromData(const char * weatherKey, const char * geoKey=nullptr) {
    auto len = min( strlen(weatherKey), API_KEY_SIZE);
    strncpy(this->openWeatherApi, weatherKey, len);
    if ( geoKey ) {
      len = min( strlen(geoKey), API_KEY_SIZE);
      strncpy(this->geolocationApi, geoKey, len);
    }
   _crc = crc();
  };
  const char * getWeatherKey() const {
    if ( crc() != this->_crc ) nullptr;
    return openWeatherApi;
  };
  const char * getGeoKey() const {
    if ( crc() != this->_crc ) nullptr;
    return geolocationApi;
  };
  uint32_t crc() const { return CRC32T::calculate( (*this), offsetof(EepromData, _crc) ); };
  bool valid() const { return crc() == _crc; };

  bool copyWeatherKeyTo(char * dest) const {
    if ( dest && strncpy(dest, openWeatherApi, API_KEY_SIZE) ){
      dest[API_KEY_SIZE] ='\0';
      return true;
    } else
    return false;
  };
  bool copyGeoKeyTo(char * dest) const {
    if ( dest && strncpy(dest, geolocationApi, API_KEY_SIZE) ){
      dest[API_KEY_SIZE] ='\0';
      return true;
    } else
    return false;
  };
  
};

// Функция для сохранения ключа API в EEPROM
bool saveApiKeys(const EepromData& eepromKeys, const int eepromStart=0) {
 if ( ! eepromKeys.valid() ) return false;
  // Инициализация EEPROM
  EEPROM.begin(512 + (sizeof(EepromData)/512));
  // Записываем ключ и контрольную сумму в EEPROM
    const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(&eepromKeys);
    for (size_t i = 0; i < sizeof(EepromData); i++) {
        EEPROM.write(eepromStart + i, byteArray[i]);
    }

  //EEPROM.write( reinterpret_cast<const uint8_t*>( &eepromKey ), sizeof(eepromKey ));

  EEPROM.commit();
  EEPROM.end();
  return true;
}

// Функция для сохранения ключа API в EEPROM
bool saveApiKey(const char* apiKey, const char* geoKey=nullptr, const int eepromStart=0) {
  EepromData eepromKeys(apiKey, geoKey );
  return saveApiKeys( eepromKeys);
}


// Функция для извлечения ключа API из EEPROM
bool loadApiKeys(EepromData& keysData, const int eepromStart=0) {
  // Инициализация EEPROM
  EEPROM.begin(512 + (sizeof(EepromData)/512));
  // Читаем ключ из EEPROM
  uint8_t* byteArray = reinterpret_cast<uint8_t*>(&keysData);
  for (size_t i = 0; i < sizeof(EepromData); i++) {
      byteArray[i] = EEPROM.read(eepromStart + i);
  }

  //auto res = EEPROM.read( reinterpret_cast<const uint8_t*>( &eepromKey ), sizeof(eepromKey ));

  bool valid = keysData.valid();
  return valid;
}

bool loadApiKeys(char * weatherKey, char * geoKey, const int eepromStart=0) {
  EepromData data;
  // Инициализация EEPROM
  bool valid = loadApiKeys( data, eepromStart );
  data.printTo(Serial);
  if ( valid ) {
    valid =  data.copyWeatherKeyTo(weatherKey);
    valid += data.copyGeoKeyTo(geoKey);
  } 
  // loadedKeys.copyWeatherKeyTo(weatherKey);
  // loadedKeys.copyGeoKeyTo(geoKey);

  return valid;
}