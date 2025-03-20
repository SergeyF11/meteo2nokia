#pragma once

#include <EEPROM.h>
#include <Arduino.h>
#include "crc32_tmpl.h"

// Размер ключа API (32 символа)
#define API_KEY_SIZE 32
// Размер контрольной суммы (CRC32)
#define CRC_SIZE 4

struct EEEPROM_keyCrc {
   char key[API_KEY_SIZE] = {0};
   uint32_t _crc;
  EEEPROM_keyCrc(){
    _crc = 123456789;
  };
  EEEPROM_keyCrc(const char * apiKey) {
    strncpy(this->key, apiKey, API_KEY_SIZE);
   _crc = crc();
  };
  const char * get() const {
    if ( crc() != this->_crc ) nullptr;
    return key;
  }
  uint32_t crc() const { return CRC32T::calculate(key); };
  bool valid() const { return crc() == _crc; };
  bool copyTo(char * dest) const {
    if ( dest && strncpy(dest, key, API_KEY_SIZE) ){
      return true;
    } else
    return false;
  };
};

// Функция для сохранения ключа API в EEPROM
bool saveApiKey(const EEEPROM_keyCrc& eepromKey, const int eepromStart=0) {
 if ( ! eepromKey.valid() ) return false;
  // Инициализация EEPROM
  EEPROM.begin(512 + (sizeof(EEEPROM_keyCrc)/512));
  // Записываем ключ и контрольную сумму в EEPROM
    const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(&eepromKey);
    for (size_t i = 0; i < sizeof(EEEPROM_keyCrc); i++) {
        EEPROM.write(eepromStart + i, byteArray[i]);
    }

  //EEPROM.write( reinterpret_cast<const uint8_t*>( &eepromKey ), sizeof(eepromKey ));

  EEPROM.commit();
  EEPROM.end();
  return true;
}

// Функция для сохранения ключа API в EEPROM
bool saveApiKey(const char* apiKey, const int eepromStart=0) {
  EEEPROM_keyCrc eepromKey(apiKey);
  return saveApiKey( eepromKey);

}


// Функция для извлечения ключа API из EEPROM
bool loadApiKey(EEEPROM_keyCrc& loadedKey, const int eepromStart=0) {
  // Инициализация EEPROM
  EEPROM.begin(512 + (sizeof(EEEPROM_keyCrc)/512));
  // Читаем ключ из EEPROM
  uint8_t* byteArray = reinterpret_cast<uint8_t*>(&loadedKey);
  for (size_t i = 0; i < sizeof(EEEPROM_keyCrc); i++) {
      byteArray[i] = EEPROM.read(eepromStart + i);
  }

  //auto res = EEPROM.read( reinterpret_cast<const uint8_t*>( &eepromKey ), sizeof(eepromKey ));

  bool valid = loadedKey.valid();
  return valid;
}

bool loadApiKey(char * key, const int eepromStart=0) {
  EEEPROM_keyCrc loadedKey;
  // Инициализация EEPROM
  bool valid = loadApiKey( loadedKey );
  if ( valid ) {
    valid =loadedKey.copyTo(key);
  }
  return valid;
}