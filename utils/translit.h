#pragma once
#define TRANSLIT_DEBUG

namespace Translit {
    bool check(const String& input){
        if( input.isEmpty()) return false;

        int charPtr = 0;
        do {
            char c = input.charAt(charPtr); 
            // кирилица в 1251, не надо транслита
            if ( (c >= 0xC0 && c <= 0xFF) || (c >= 0x80 && c <= 0x9F) ) return false;
            if ( isAlpha(c)) return true;
            charPtr++;
        } while( charPtr < input.length());
        return false;
    };

    // Вспомогательные функции для работы с регистром
bool isUpperCase(char c) {
  return (c >= 'A' && c <= 'Z');
}

char toUpperCase(char c) {
  if (c >= 'a' && c <= 'z') {
    return c - 32;
  }
  return c;
}

char toLowerCase(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c + 32;
  }
  return c;
}

    // Функция преобразования транслита в кириллицу
String toCyrillic(const String& input) {

  String result = "";
  
  for (unsigned int i = 0; i < input.length(); i++) {
    char current = input[i];
    char next = (i + 1 < input.length()) ? input[i + 1] : '\0';
    bool isUpper = isUpperCase(current);
    char currentLower = toLowerCase(current);
    
    // Проверяем двухбуквенные комбинации
    if (i + 1 < input.length()) {
      char nextLower = toLowerCase(next);
      
      if (currentLower == 's' && nextLower == 'h') { 
        result += isUpper ? "Ш" : "ш"; i++; continue; 
      }
      else if (currentLower == 'c' && nextLower == 'h') { 
        result += isUpper ? "Ч" : "ч"; i++; continue; 
      }
      else if (currentLower == 'z' && nextLower == 'h') { 
        result += isUpper ? "Ж" : "ж"; i++; continue; 
      }
      else if (currentLower == 'y' && nextLower == 'o') { 
        result += isUpper ? "Ё" : "ё"; i++; continue; 
      }
      else if (currentLower == 'y' && nextLower == 'u') { 
        result += isUpper ? "Ю" : "ю"; i++; continue; 
      }
      else if (currentLower == 'y' && nextLower == 'a') { 
        result += isUpper ? "Я" : "я"; i++; continue; 
      }
      else if (currentLower == 'k' && nextLower == 'h') { 
        result += isUpper ? "Х" : "х"; i++; continue; 
      }
      else if (currentLower == 't' && nextLower == 's') { 
        result += isUpper ? "Ц" : "ц"; i++; continue; 
      }
    }
    
    // Однобуквенные соответствия
    switch (currentLower) {
      case 'a': result += isUpper ? "А" : "а"; break;
      case 'b': result += isUpper ? "Б" : "б"; break;
      case 'v': result += isUpper ? "В" : "в"; break;
      case 'g': result += isUpper ? "Г" : "г"; break;
      case 'd': result += isUpper ? "Д" : "д"; break;
      case 'e': result += isUpper ? "Е" : "е"; break;
      case 'z': result += isUpper ? "З" : "з"; break;
      case 'i': result += isUpper ? "И" : "и"; break;
      case 'y': result += isUpper ? "Ы" : "ы"; break;
      case 'k': result += isUpper ? "К" : "к"; break;
      case 'l': result += isUpper ? "Л" : "л"; break;
      case 'm': result += isUpper ? "М" : "м"; break;
      case 'n': result += isUpper ? "Н" : "н"; break;
      case 'o': result += isUpper ? "О" : "о"; break;
      case 'p': result += isUpper ? "П" : "п"; break;
      case 'r': result += isUpper ? "Р" : "р"; break;
      case 's': result += isUpper ? "С" : "с"; break;
      case 't': result += isUpper ? "Т" : "т"; break;
      case 'u': result += isUpper ? "У" : "у"; break;
      case 'f': result += isUpper ? "Ф" : "ф"; break;
      case 'h': result += isUpper ? "Х" : "х"; break;
      case 'c': result += isUpper ? "Ц" : "ц"; break;
      case 'j': result += isUpper ? "Дж" : "дж"; break;
      case 'w': result += isUpper ? "В" : "в"; break;
      case 'q': result += isUpper ? "К" : "к"; break;
      default: result += current;
    }
  }
  
  #ifdef TRANSLIT_DEBUG
  Serial.printf("Translit: %s => %s\n", input.c_str(), result.c_str());
  #endif
  return result;
}


}