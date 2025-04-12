
#include <Wire.h>
#include <Adafruit_HTU21DF.h> // Используем библиотеку Adafruit HTU21D
//#include <SparkFunHTU21D.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544_multi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#define PRINT_COLOR BLACK
#define CONTRAST1 55
#define CONTRAST2 60


//#include "utils/weather_icons.h" // Подключаем файл с иконками

tm* nowTm;

//const String units = "metric"; // Используйте "imperial" для Фаренгейта
#define SECONDS *1000
#define MINUTES *60 SECONDS
#define HOURS *60 MINUTES
#include "utils/weather_async.h" 
#include "utils/wifi_utils.h"
//#include "utils/geo_utils.h"
#include "utils/geo_async.h"
#include "utils/sensor_utils.h"
#include "utils/display_utils.h"
#include "utils/time_utils.h"
//#include "utils/tz_utils.h"
#include "utils/button_utils.h"

AsyncHttpsClient httpsClient;
// Глобальные переменные для хранения настроек в eeprom и в программе
EepromData eepromSets;

#define  hourToMs(hs)  (1000UL * 60 * 60 * hs  )
// Период обновления данных о погоде (по умолчанию 3 часа)
//unsigned long weatherUpdateInterval = hourToMs(1); // 1 час в миллисекундах
constexpr unsigned long weatherUpdateInterval = 30 MINUTES; //hourToMs( 1/6 ); // = 10 минут для отладки
#define UPDATE_NOW -weatherUpdateInterval

unsigned long lastWeatherUpdate = 0;

#include "utils/ticker.h"


struct RefresherTicker weatherTick(weatherUpdateInterval);
struct SimpleTicker htuSensorTick( 60 SECONDS );



// Создаем объекты для экранов Nokia 5110
//auto builtinLed = LED_BUILTIN; 

//Adafruit_PCD8544 display2 = Adafruit_PCD8544(14, 13, D4, 12, 16); // CLK, DIN, DC, SCE, RST (экран 1)
Adafruit_PCD8544 display2 = Adafruit_PCD8544(14, 13, 0, 12, 16); // CLK, DIN, DC, SCE, RST (экран 1)
//                                           D5, D7, D4,D8, D0      
Adafruit_PCD8544 display1 = Adafruit_PCD8544(14, 13, 0, 15, 16); // CLK, DIN, DC, SCE, RST (экран 2)
//                                            D5, D7,D3,D6, D0


// Переменные для хранения данных о погоде
String weatherDescription = "";
float weatherTemp = 0, weatherTempFeel = 0;
float weatherHumidity = 0;

//bool wifiSleep = false;
//bool tripleReset;

void setup() {

  httpsClient.setInsecureMode(true);
  httpsClient.setTimeout(15000);

  // Инициализация Serial для отладки
  Serial.begin(115200);

  // load settings from EEPROM 0
  isSettingsValid = eepromSets.load();
  configTime( 0,0, NTP_SERVERS);
  
  // Инициализация экранов
  displays::init();
    if ( tripleReset.isTriggered() ){
    static unsigned long startMs = millis();
    while(!Serial){ 
      delay(10);
      if ( millis()-startMs > 3000UL ) return;
    }
    Serial.println("\n\nTriple reset detected!!!\n\n");   
  }

  // TestChars::setDelay(1000);
  // TestChars::run(display1, 2);
  // BigSign::test(display1);
  // pointStop(10000,"Test done\n");



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
  // EepromData loadedData;
  // hasValidApiKey = loadEepromData( loadedData  );  //loadApiKeys( openWeatherApiKeyStr, geolocationApiKeyStr );
  
  // if ( hasValidApiKey ) {
  //   loadedData.copyWeatherKeyTo(openWeatherApiKeyStr);
  //   loadedData.copyGeoKeyTo(geolocationApiKeyStr);
  // } else {
  //   openWeatherApiKeyStr[0] = '\0';
  //   geolocationApiKeyStr[0] = '\0';
  // }
  // displayContrast1 = loadedData.getContrast1();
  // displayContrast2 = loadedData.getContrast2();

  displays::setContrast(eepromSets.getContrast1(), eepromSets.getContrast2(), &Serial);
  CaptivePortal::init(eepromSets);


  //GeoLocationAsync::test();

  
  // Подключение к Wi-Fi
  connectToWiFi(&display1);

  
  bool validLocation = false;
  while( ! validLocation ){
    auto error = GeoLocationAsync::waitLocationReceived( GeoLocationAsync::myLocation, &display1 );
    switch( error ){
      case RequestGeoAsync::OK:
        validLocation = true;
        display1.clearDisplay();
        display1.display();
        if ( aproximateLocationAsync ) display1.print('~');
        display1.print(GeoLocationAsync::myLocation.city);
        break;
      default:
        Serial.print("Error get location: ");
        isSettingsValid = false;
        connectToWiFi(&display1);
    } 
  }

  // while(1){
  //   TimeUtils::printTo(display1, display2, &FreeMonoBold24pt7b, 1, &FreeMonoBold12pt7b );
  //   delay(1000);
  // }

  weatherTick.reset( -weatherUpdateInterval );
  httpsClient.setTimeout(3000);
}


void loop() {
  // вариант DS
  Weather::handleTick();  
  Weather::updateDataDS(display1);
  // if ( weatherTick.tick() ){
  //   if ( Reconnect::connect() ) {
  //     //Weather::waitConnection = true;
  //     Weather::updateState = AsyncRequest::WaitWiFiConnection;
  //   } else {
  //     weatherTick.reset( Weather::wrongUpdateInterval( 5 SECONDS ) );
  //   }
  // }
  //Weather::updateDataMy(display1);
  

  
  if( TimeUtils::printTo( display2 ) ){

      if ( htuSensorTick.tick()) {
        HtuSensor::updateData();
      }
      HtuSensor::printData(display2);
  }
    
  
}