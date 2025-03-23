

#include <Wire.h>
#include <Adafruit_HTU21DF.h> // Используем библиотеку Adafruit HTU21D
//#include <SparkFunHTU21D.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>


#define PRINT_COLOR BLACK
#define CONTRAST1 57
#define CONTRAST2 60
#include "utils/weather_async.h" // _utils.h"
//#include "utils/weather_icons.h" // Подключаем файл с иконками

// Настройки Wi-Fi
// const char* ssid = "Your_SSID";
// const char* password = "Your_PASSWORD";
#define MY_CREDENTIAL
#include <my.h>
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// Настройки OpenWeatherMap
// const String apiKey = OpenWeatherMap_API_Key;
// char apiKey[API_KEY_SIZE+1] = {0};
// const String city = "Moscow";
// const String country = "RU";
const char timeZone[] = "GMT";
tm* nowTm;

//const String units = "metric"; // Используйте "imperial" для Фаренгейта

#include "utils/wifi_utils.h"
#include "utils/geo_utils.h"
#include "utils/sensor_utils.h"
#include "utils/display_utils.h"
#include "utils/time_utils.h"
#include "utils/tz_utils.h"


#define hourToMs(hs) (1000L * 60 * 60 * hs  )
// Период обновления данных о погоде (по умолчанию 3 часа)
//unsigned long weatherUpdateInterval = hourToMs(1); // 1 час в миллисекундах
unsigned long weatherUpdateInterval = hourToMs( 1/20 ); // 3 минут для отладки
unsigned long lastWeatherUpdate = 0;

#include "utils/ticker.h"
#define SECUND *1000

struct SimpleTicker weatherTick(weatherUpdateInterval);
struct SimpleTicker htuSensorTick( 60 SECUND );


//HTU21D htu;

// Создаем объекты для экранов Nokia 5110
Adafruit_PCD8544 display2 = Adafruit_PCD8544(14, 13, 2, 12, 16); // CLK, DIN, DC, SCE, RST (экран 1)
//                                           D5, D7, D4,D8, D0      
Adafruit_PCD8544 display1 = Adafruit_PCD8544(14, 13, 0, 15, 16); // CLK, DIN, DC, SCE, RST (экран 2)
//                                            D5, D7,D3,D6, D0


// Переменные для хранения данных о погоде
String weatherDescription = "";
float weatherTemp = 0, weatherTempFeel = 0;
float weatherHumidity = 0;
//bool wifiSleep = false;


void setup() {
  
  // Инициализация Serial для отладки
  Serial.begin(115200);

  configTime( timeZone, NTP_SERVERS);

  
  // Инициализация экранов
  displays::init();

  // TestChars::setDelay(3000);
  // TestChars::run(display1, 2);


  Wire.begin(SDA, SCL);
  //Wire.setClock(400000); // Высокая скорость I2C
  I2C_Scan::printTo(Serial);
  
  HtuSensor::hasSensor = htu.begin(&Wire);

  if ( !HtuSensor::hasSensor ) {
    Serial.println("Couldn't find sensor!");
    
  } else {
    HtuSensor::updateData();
    HtuSensor::printData(display2, true);

    // float temp = htu.readTemperature();
    // float rel_hum = htu.readHumidity();
    Serial.print("Temp: "); Serial.print(HtuSensor::temperature); Serial.print("C");
    Serial.print("\t\t");
    Serial.print("Humidity: "); Serial.print(HtuSensor::humidity); Serial.println("\%");
  }
  //hasSensor = true;


  //testPrintDots(display1, 50);

  // Подключение к Wi-Fi
  connectToWiFi(&display1);


  auto error = GeoLocation::getLocation( myLocation, &display1 );
  switch( error ){
    case AsyncRequest::OK:
      //myLocation.printTo(Serial);
      //TimeZone::configTime( myLocation.timeZone, NTP_SERVERS);
      display1.clearDisplay();
      display1.display();
      break;
    default:
      Serial.print("Error get location: ");
      Serial.println( error );
  } 

  //HtuSensor::sensorData(display2, true, true );
  weatherTick.reset( -weatherUpdateInterval );
  //Serial.println("End begin"); delay(3000);
}

//bool weatherUpdated =false;
void loop() {
  
  if ( weatherTick.tick() ){
    //Weather::update(display1, true);
    if ( Weather::updateData() == AsyncRequest::OK ){
    
      //weatherUpdated = true;
      //Weather::update(display1);
      //TimeZone::configTime(myLocation.timeZone, NTP_SERVERS);
    } else {
      //weatherUpdated = false;

      // если даже нет местоположения, то это заставка wifi
      // рисуем точки
      if ( ! myLocation.valid() ) printDots(display1); 
      // повторить запрос через 5 секунд
      weatherTick.reset( Weather::wrongUpdateInterval( 5 SECUND ) );
    }
  }
  
  switch ( Weather::updateState ){
    case AsyncRequest::Unknown:
      break;
    case AsyncRequest::State::SuccessRespond:
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      Serial.println("WiFi disconnect and off");
      Weather::update(display1);
      Weather::updateState = AsyncRequest::State::Unknown;
      break;
    case AsyncRequest::State::RespondWaiting:
      Weather::update(display1, true);
      Weather::updateState = AsyncRequest::State::Unknown;
      break;
    default:
    //  case Weather::FailUpdate:
      Weather::update(display1, nowTm->tm_sec == 0);
      break;
  }

  
  if( TimeUtils::printTo( display2 ) ){
      //HtuSensor::tick(display2);
      if ( htuSensorTick.tick()) {
        HtuSensor::updateData();
      }
      HtuSensor::printData(display2);
  }
    
  // if ( WiFi.isConnected() && 
  //     TimeUtils::isSynced() && 
  //     weatherUpdated ){
      
  //     WiFi.disconnect();
  //     WiFi.mode(WIFI_OFF);
  //     Serial.println("WiFi disconnect and off");
  //   }
  
}