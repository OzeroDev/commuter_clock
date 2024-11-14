#include <Arduino.h>

#include <EEPROM.h>



#include "time.h"
#include "esp_sntp.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";



#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

BLEServer *pServer = NULL;


// UUIDs for bluetooth le:
// https://www.uuidgenerator.net/
#define CHARACTERISTIC_UUID_SSID "WiFi SSID"
#define CHARACTERISTIC_UUID_PASSWORD "WiFi Password"
#define CHARACTERISTIC_UUID_START_COORDINATE "Starting Coordinates in the following format: 'Latitude,Longtitude'"
#define CHARACTERISTIC_UUID_END_COORDINATE "Ending Coordinates in the following format: 'Latitude,Longtitude'"
#define CHARACTERISTIC_UUID_MON "Monday Arrival in the following 24-hour format: 'xxxx', for example 8:00 am would be '0800'"
#define CHARACTERISTIC_UUID_TUE "Tuesday Arrival in the following 24-hour format: 'xxxx', for example 8:00 am would be '0800'"
#define CHARACTERISTIC_UUID_WEN "Wednesday Arrival in the following 24-hour format: 'xxxx', for example 8:00 am would be '0800'"
#define CHARACTERISTIC_UUID_THU "Thursday Arrival in the following 24-hour format: 'xxxx', for example 8:00 am would be '0800'"
#define CHARACTERISTIC_UUID_FRI "Friday Arrival in the following 24-hour format: 'xxxx', for example 8:00 am would be '0800'"
#define CHARACTERISTIC_UUID_API_KEY "API key for distancematrix (can obtain one from https://distancematrix.ai/)"


// max char[] lengths
#define SSID_LENGTH 20
#define PASSWORD_LENGTH 20
#define COORDINATE_LENGTH 20
#define TIME_LENGTH 4
#define TIMEZONE_IO_LENGTH 5
#define API_KEY_LENGTH 64

char ssid[SSID_LENGTH] = "0";
char password[PASSWORD_LENGTH] = "0";
char startCoords[COORDINATE_LENGTH] = "0";
char endCoords[COORDINATE_LENGTH] = "0";
char monTime[TIME_LENGTH] = "0";
char tueTime[TIME_LENGTH] = "0";
char wedTime[TIME_LENGTH] = "0";
char thuTime[TIME_LENGTH] = "0";
char friTime[TIME_LENGTH] = "0";
char api[API_KEY_LENGTH] = "0";

// offsets
#define SSID_OFFSET 0
#define PASSWORD_OFFSET 21
#define START_COORDINATE_OFFSET 42
#define END_COORDINATE_OFFSET 63
#define MON_TIME_OFFSET 84
#define TUE_TIME_OFFSET 89
#define WEN_TIME_OFFSET 94
#define THU_TIME_OFFSET 99
#define FRI_TIME_OFFSET 104
#define TIMEZONE_IO_OFFSET 109
#define API_KEY_OFFSET 115

#define FLASH_SIZE 180
// to check if a param has been set, do 

void writeStringToEEPROM(int addrOffset, const char* strToWrite) {
  int len = strlen(strToWrite);
  Serial.println(len);
  for (int i = 0; i < len; i++) {
    EEPROM.put(addrOffset + i, strToWrite[i]);
  }
  EEPROM.put(addrOffset + len, '\0');
  EEPROM.commit();
}

void readStringFromEEPROM(int addrOffset, int readSize, char* strToRead) {
  for (int i = 0; i < readSize; i++) {
    char in;
    if ((in = EEPROM.read(addrOffset + i)) != '\0'){
      strToRead[i] = in;
    } else {
      strToRead[i] = '\0';
      break;
    }
  }
  strToRead[readSize] = '\0';
}



bool checkIfSetupCompleted(){
  char res;
  readStringFromEEPROM(FLASH_SIZE, 1, &res);
  Serial.println("rh");
  Serial.println(res);
  Serial.println("here");
  if (res == '1'){
    return true;
  } else {
    return false;
  }
}

String loadAllConfigs(){
  char res;

  readStringFromEEPROM(SSID_OFFSET+SSID_LENGTH, 1, &res);
  if (res != '1'){
    return "WiFi SSID";
  } else {
    char returnedChar[SSID_LENGTH];
    readStringFromEEPROM(SSID_OFFSET, SSID_LENGTH, returnedChar);
    strncpy(ssid, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(PASSWORD_OFFSET+PASSWORD_LENGTH, 1, &res);
  if (res != '1'){
    return "WiFi Password";
  } else {
    char returnedChar[PASSWORD_LENGTH];
    readStringFromEEPROM(PASSWORD_OFFSET, PASSWORD_LENGTH, returnedChar);
    strncpy(password, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(START_COORDINATE_OFFSET+COORDINATE_LENGTH, 1, &res);
  if (res != '1'){
    return "Starting Coordinates";
  } else {
    char returnedChar[COORDINATE_LENGTH];
    readStringFromEEPROM(START_COORDINATE_OFFSET, COORDINATE_LENGTH, returnedChar);
    strncpy(startCoords, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(END_COORDINATE_OFFSET+COORDINATE_LENGTH, 1, &res);
  if (res != '1'){
    return "Ending Coordinates";
  } else {
    char returnedChar[COORDINATE_LENGTH];
    readStringFromEEPROM(END_COORDINATE_OFFSET, COORDINATE_LENGTH, returnedChar);
    strncpy(endCoords, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(MON_TIME_OFFSET+TIME_LENGTH, 1, &res);
  if (res != '1'){
    return "Monday Arrive Time";
  } else {
    char returnedChar[TIME_LENGTH];
    readStringFromEEPROM(MON_TIME_OFFSET, TIME_LENGTH, returnedChar);
    strncpy(monTime, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(TUE_TIME_OFFSET+TIME_LENGTH, 1, &res);
  if (res != '1'){
    return "Tuesday Arrive Time";
  } else {
    char returnedChar[TIME_LENGTH];
    readStringFromEEPROM(TUE_TIME_OFFSET, TIME_LENGTH, returnedChar);
    strncpy(tueTime, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(WEN_TIME_OFFSET+TIME_LENGTH, 1, &res);
  if (res != '1'){
    return "Wednesday Arrive Time";
  } else {
    char returnedChar[TIME_LENGTH];
    readStringFromEEPROM(WEN_TIME_OFFSET, TIME_LENGTH, returnedChar);
    strncpy(wedTime, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(THU_TIME_OFFSET+TIME_LENGTH, 1, &res);
  if (res != '1'){
    return "Thursday Arrive Time";
  } else {
    char returnedChar[TIME_LENGTH];
    readStringFromEEPROM(THU_TIME_OFFSET, TIME_LENGTH, returnedChar);
    strncpy(thuTime, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(FRI_TIME_OFFSET+TIME_LENGTH, 1, &res);
  if (res != '1'){
    return "Friday Arrive Time";
  } else {
    char returnedChar[TIME_LENGTH];
    readStringFromEEPROM(FRI_TIME_OFFSET, TIME_LENGTH, returnedChar);
    strncpy(friTime, returnedChar, strlen(returnedChar));
  }

  readStringFromEEPROM(API_KEY_OFFSET+API_KEY_LENGTH, 1, &res);
  if (res != '1'){
    return "API Key";
  } else {
    char returnedChar[API_KEY_LENGTH];
    readStringFromEEPROM(API_KEY_OFFSET, TIME_LENGTH, returnedChar);
    strncpy(api, returnedChar, strlen(returnedChar));
  }

  writeStringToEEPROM(FLASH_SIZE, new char('1'));

  return "done";
}

bool setupCompleted;

class MyCallbacks: public BLECharacteristicCallbacks {
  public: 
    int mem_address;
    char* val_ptr;
    int set_address;
    MyCallbacks (int memaddress, char* ptr, int setAddress){
      mem_address = memaddress;
      val_ptr = ptr;
      set_address = setAddress;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      writeStringToEEPROM(mem_address, value.c_str());
      writeStringToEEPROM(set_address, new char('1'));
      
      strncpy(val_ptr, value.c_str(), strlen(value.c_str()));

      setupCompleted = checkIfSetupCompleted();
      Serial.println("was setupcomplete:");
      Serial.println(setupCompleted);

      if (!setupCompleted) {
        String message = loadAllConfigs();

        tft.fillScreen(TFT_WHITE);

        tft.setTextSize(2);
        tft.setTextColor(TFT_BLACK);

        tft.setCursor(20,90);
        tft.println("Setup this device");
        tft.setCursor(20,110);
        tft.println("via Bluetooth");

        tft.setTextSize(1);
        tft.setCursor(20,140);
        tft.println("Device Name: Commuter Clock");

        tft.setCursor(20,160);
        tft.println("Please set the following:");
        tft.setCursor(20,170);
        tft.println(message);

        if(message.equals("done")){
          setupCompleted = true;
        }
      }

    }
};


bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class BLEServiceCreator {
  public:
    
    BLEService *pService;
    BLEServiceCreator (BLEUUID s_uuid) {
        //BLEUUID c_uuid, BLEUUID d_uuid,
      pService = pServer->createService(s_uuid);
    }
    void addCharacteristic(BLEUUID s_uuid,  char* description, int mem_address, char* ptr, int set_address){

      BLECharacteristic *pCharacteristic;
      BLECharacteristic *pDescription;

      pCharacteristic = pService->createCharacteristic(
                          s_uuid,
                          BLECharacteristic::PROPERTY_WRITE
                        );

      pDescription = pService->createCharacteristic(
                          s_uuid,
                          BLECharacteristic::PROPERTY_READ 
                        );
      pDescription->setValue((uint8_t *)description, size_t(strlen(description)));

      pCharacteristic->setCallbacks(new MyCallbacks(mem_address, ptr, set_address));
    }
    void startService(){
      pService->start();

    }
};

bool wifiSetup = false;
// setup1
void setup() {
  Serial.begin(115200);
  EEPROM.begin(FLASH_SIZE+1);


  tft.init();
  tft.fillScreen(TFT_WHITE);

  loadAllConfigs();
  setupCompleted = checkIfSetupCompleted();
  Serial.println("was setupcomplete:");
  Serial.println(setupCompleted);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(timeinfo.tm_mday);
  Serial.println(timeinfo.tm_wday);
  Serial.println(timeinfo.tm_hour);
  Serial.println(timeinfo.tm_min);
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

int getCurrentHour(){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.println(timeinfo.tm_hour);
  return (timeinfo.tm_hour);
}

int getCurrentMin(){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.println(timeinfo.tm_min);
  return (timeinfo.tm_min);
}

void updateScreen(){
  tft.fillScreen(TFT_WHITE);
  int hour;
  int min;
  int nhour;
  int nmin;
  //get new time info
  struct tm timeinfo;
  switch (timeinfo.tm_wday)
  {
  case 1:
     hour = (monTime[0] - '0') * 10 + (monTime[1] - '0');
     min = (monTime[2] - '0') * 10 + (monTime[3] - '0');

     nhour = (tueTime[0] - '0') * 10 + (tueTime[1] - '0');
     nmin = (tueTime[2] - '0') * 10 + (tueTime[3] - '0');
    if (getCurrentHour()<=hour && getCurrentMin()<min){
      Serial.print(hour);
      Serial.print(min);
    } else {
      Serial.print(nhour);
      Serial.print(nmin);
    }

    

    break;
  case 2:
     hour = (tueTime[0] - '0') * 10 + (tueTime[1] - '0');
     min = (tueTime[2] - '0') * 10 + (tueTime[3] - '0');

     nhour = (wedTime[0] - '0') * 10 + (wedTime[1] - '0');
     nmin = (wedTime[2] - '0') * 10 + (wedTime[3] - '0');
    if (getCurrentHour()<=hour && getCurrentMin()<min){
      Serial.print(hour);
      Serial.print(min);
    } else {
      Serial.print(nhour);
      Serial.print(nmin);
    }
    /* code */
    break;
  case 3:
     hour = (wedTime[0] - '0') * 10 + (wedTime[1] - '0');
     min = (wedTime[2] - '0') * 10 + (wedTime[3] - '0');

     nhour = (thuTime[0] - '0') * 10 + (thuTime[1] - '0');
     nmin = (thuTime[2] - '0') * 10 + (thuTime[3] - '0');
    if (getCurrentHour()<=hour && getCurrentMin()<min){
      Serial.print(hour);
      Serial.print(min);
    } else {
      Serial.print(nhour);
      Serial.print(nmin);
    }
      //https://api.distancematrix.ai/maps/api/distancematrix/json?origins=45.220029,-93.647192&destinations=44.971440,-93.236081&arrival_time=1727702100&mode=driving&units=imperial&key=DhNDSg7FDGvreVSBvNvVHT8ZLSDjzsK4FYGbUQKP7PIoV5b9v6m3CkKqxynVj0nd
  
    /* code */
    break;
  case 4:
     hour = (thuTime[0] - '0') * 10 + (thuTime[1] - '0');
     min = (thuTime[2] - '0') * 10 + (thuTime[3] - '0');

     nhour = (friTime[0] - '0') * 10 + (friTime[1] - '0');
     nmin = (friTime[2] - '0') * 10 + (friTime[3] - '0');
    if (getCurrentHour()<=hour && getCurrentMin()<min){
      Serial.print(hour);
      Serial.print(min);
    } else {
      Serial.print(nhour);
      Serial.print(nmin);
    }
    /* code */
    break;
  case 5:
     hour = (friTime[0] - '0') * 10 + (friTime[1] - '0');
     min = (friTime[2] - '0') * 10 + (friTime[3] - '0');

     nhour = (monTime[0] - '0') * 10 + (monTime[1] - '0');
     nmin = (monTime[2] - '0') * 10 + (monTime[3] - '0');
    if (getCurrentHour()<=hour && getCurrentMin()<min){
      Serial.print(hour);
      Serial.print(min);
    } else {
      Serial.print(nhour);
      Serial.print(nmin);
    }
    /* code */
    break;
  
  default:
     hour = (monTime[0] - '0') * 10 + (monTime[1] - '0');
     min = (monTime[2] - '0') * 10 + (monTime[3] - '0');

     nhour = (tueTime[0] - '0') * 10 + (tueTime[1] - '0');
     nmin = (tueTime[2] - '0') * 10 + (tueTime[3] - '0');
    Serial.print(hour);
    Serial.print(min);

    break;
  }


  HTTPClient client;
  client.begin("https://api.distancematrix.ai/maps/api/distancematrix/json?origins="+String(startCoords)+"&destinations="+String(endCoords)+"&mode=driving&departure_time=now&units=imperial?key="+String(key));

  int httpCode = client.GET();

  if (httpCode > 0){
    String payload = client.getString();
    payload.replace("\n", "");
    payload.trim();

    char json[500];
    payload.toCharArray(json,  500);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);

    const char* duration = doc["rows"][0]["elements"][0]["duration"]["text"];
    const char* durationhist = doc["rows"][0]["elements"][0]["duration_in_traffic"]["text"];
    Serial.println(String(duration));
    Serial.println(String(durationhist));
  

    switch (err.code()) {
      case DeserializationError::Ok:
        Serial.print(F("Deserialization succeeded"));
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F("Invalid input!"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Not enough memory"));
        break;
      default:
        Serial.print(F("Deserialization failed"));
        break;
    }
  }

}

int wifiFailedCount = 0;

// loop1
void loop() {


  if (!setupCompleted){
    //setup not completed, start bluetooth

        
    // Create the BLE Device
    BLEDevice::init("Commuter Clock");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEServiceCreator c1 = BLEServiceCreator(BLEUUID("46a914b6-427a-4065-b538-d45cf7856901"));
    c1.addCharacteristic(BLEUUID("01c66ecc-dcf7-4343-95eb-b4f83f4775a4"), CHARACTERISTIC_UUID_SSID, SSID_OFFSET, ssid, SSID_OFFSET+SSID_LENGTH);
    c1.addCharacteristic(BLEUUID("f8cd9aa8-b682-4f3d-868f-c5e9bf2019fb"), CHARACTERISTIC_UUID_PASSWORD, PASSWORD_OFFSET, password, PASSWORD_OFFSET+PASSWORD_LENGTH);
    c1.startService();

    BLEServiceCreator c2 = BLEServiceCreator(BLEUUID("2a6db749-544d-4e20-865a-7952e6916cd5"));
    c2.addCharacteristic(BLEUUID("65a9743f-d233-4d5c-a672-5c7b52dd2d63"), CHARACTERISTIC_UUID_START_COORDINATE, START_COORDINATE_OFFSET, startCoords, START_COORDINATE_OFFSET+COORDINATE_LENGTH);
    c2.addCharacteristic(BLEUUID("cb9fbb6a-b19e-484a-8fde-4745e27f544f"), CHARACTERISTIC_UUID_END_COORDINATE, END_COORDINATE_OFFSET, endCoords, END_COORDINATE_OFFSET+COORDINATE_LENGTH);
    c2.startService();

    BLEServiceCreator c3 = BLEServiceCreator(BLEUUID("67624b74-95bf-47ae-8878-9c8cd35d4e34"));
    c3.addCharacteristic(BLEUUID("9730f332-d2a0-420a-93de-fd300be3259c"), CHARACTERISTIC_UUID_MON, MON_TIME_OFFSET, monTime, MON_TIME_OFFSET+TIME_LENGTH);
    c3.addCharacteristic(BLEUUID("b7b2e8df-3417-4936-ae76-54387b7c8fc9"), CHARACTERISTIC_UUID_TUE, TUE_TIME_OFFSET, tueTime, TUE_TIME_OFFSET+TIME_LENGTH);
    c3.addCharacteristic(BLEUUID("ace410db-cc42-4624-a3ad-10f6f9516a0b"), CHARACTERISTIC_UUID_WEN, WEN_TIME_OFFSET, wedTime, WEN_TIME_OFFSET+TIME_LENGTH);
    c3.startService();


    BLEServiceCreator c4 = BLEServiceCreator(BLEUUID("7bf6c76c-b88e-4588-8c7c-af3925bd3753"));
    c4.addCharacteristic(BLEUUID("6e71f1a1-9147-401c-a55c-d730013e4bd1"), CHARACTERISTIC_UUID_THU, THU_TIME_OFFSET, thuTime, THU_TIME_OFFSET+TIME_LENGTH);
    c4.addCharacteristic(BLEUUID("fe2503d9-5809-4d1c-88c8-54997c154ebe"), CHARACTERISTIC_UUID_FRI, FRI_TIME_OFFSET, friTime, FRI_TIME_OFFSET+TIME_LENGTH);
    c4.startService();

    BLEServiceCreator c5 = BLEServiceCreator(BLEUUID("6bbf26c8-0398-41c2-a3bb-8282950c2c33"));
    c5.addCharacteristic(BLEUUID("b1d0e060-4d4e-4a74-a1c9-eb1c362245a8"), CHARACTERISTIC_UUID_API_KEY, API_KEY_OFFSET, api, API_KEY_OFFSET+API_KEY_LENGTH);
    c5.startService();

    // Start advertising
    pServer->getAdvertising()->start();


    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);

    tft.setCursor(20,90);
    tft.println("Setup this device");
    tft.setCursor(20,110);
    tft.println("via Bluetooth");

    tft.setTextSize(1);
    tft.setCursor(20,140);
    tft.println("Device Name: Commuter Clock");


    tft.setCursor(20,160);
    tft.println("Please set the following:");
    tft.setCursor(20,170);

    String message = loadAllConfigs();
    tft.println(message);


    while (!setupCompleted) {
      // disconnecting
      if (!deviceConnected && oldDeviceConnected) {
          delay(500); // give the bluetooth stack the chance to get things ready
          pServer->startAdvertising(); // restart advertising
          Serial.println("start advertising");
          oldDeviceConnected = deviceConnected;
      }
      // connecting
      if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
      }
      delay(400);
    }
    BLEDevice::deinit(true);

  } else {
    //setup was completed
    wifiFailedCount = 0;

    Serial.println(ssid);
    Serial.println(password);
    if (!wifiSetup){
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED /*&& wifiFailedCount < 20*/) {
        delay(500);
        Serial.print(".");
        wifiFailedCount = wifiFailedCount+1;
      }
      /*
      if (wifiFailedCount >= 20){
        writeStringToEEPROM(SSID_OFFSET+SSID_LENGTH, new char('0'));
        writeStringToEEPROM(PASSWORD_OFFSET+PASSWORD_LENGTH, new char('0'));
        writeStringToEEPROM(FLASH_SIZE, new char('0'));
        setupCompleted = false;
        return;
      }
      */
      Serial.println(" CONNECTED");
      const char *time_zone = "CST6CDT,M3.2.0,M11.1.0";
      configTzTime(time_zone, ntpServer1, ntpServer2);
      wifiSetup = true;
    }
    printLocalTime();

    tft.fillScreen(TFT_WHITE);

    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    updateScreen();
  }

  if (!wifiSetup){
    return;
  } else {
    delay(1000*60*5);
  }
}
