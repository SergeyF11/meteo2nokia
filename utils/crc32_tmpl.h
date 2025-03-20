#pragma once

#include <cstdint>
#include <cstddef>

/// @param MyStruct
///  struct MyStruct {
///    int32_t id;
///    float value;
///    char name[16];
///    uint32_t crc; // Поле для хранения CRC32, исключаемое из расчёта
///  };
/// @brief using: MyStruct data = {123, 45.67f, "TestStruct", 0};
// data.crc = calculateCRC32(data, offsetof(MyStruct, crc), sizeof(data.crc));

namespace CRC32T {

// Скрытая функция для вычисления CRC32
namespace {
    // Вспомогательная функция для вычисления CRC32 для байтового массива
    uint32_t _calculate(const uint8_t* byteArray, size_t length, size_t excludeOffset = 0) {
        uint32_t crc = 0xFFFFFFFF;
        size_t excludeSize = ( excludeOffset == 0 ) ? 0 : sizeof(uint32_t);
        for (size_t i = 0; i < length; i++) {
            if (i >= excludeOffset && i < excludeOffset + excludeSize) {
                continue; // Пропуск исключаемых байтов
            }
            uint8_t c = byteArray[i];
            for (uint32_t j = 0x80; j > 0; j >>= 1) {
                bool bit = crc & 0x80000000;
                if (c & j) {
                    bit = !bit;
                }
                crc <<= 1;
                if (bit) {
                    crc ^= 0x04C11DB7;
                }
            }
        }
        return crc;
    }
};   


/// @brief Основная шаблонная функция для вычисления CRC32
/// @param data MyStruct
/// @note struct MyStruct {
/// @note   int32_t id;
/// @note   float value;
/// @note   char name[16];
/// @note   uint32_t crc; 
/// @note };
/// @note using: 
/// @note data.crc = calculateCRC32(data, offsetof(MyStruct, crc), sizeof(data.crc));
template <typename T>
uint32_t calculate(const T& data, size_t excludeOffset = 0) {
    const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(&data);
    size_t length = sizeof(T);
    return _calculate(byteArray, length, excludeOffset);
}

};