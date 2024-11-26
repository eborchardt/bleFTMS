#define NIMBLE
#include <ArduinoJson.h>
#include <TimeLib.h>  // https://playground.arduino.cc/Code/Time/

#include "bleFTMS.h" 

#include <NimBLEDevice.h>

const char* deviceName = DEVICE_NAME;

bool bleClientConnected = false;
bool bleClientConnectedPrev = false;
bool bleClientOpen = true;

// note: Fitness Machine Feature is a mandatory characteristic (property_read)
#define FTMSService BLEUUID((uint16_t)0x1826)
#define RSCService BLEUUID((uint16_t)0x1814)

BLEServer* pServer = NULL;
BLECharacteristic* pTreadmill    = NULL;
BLECharacteristic* pFeature      = NULL;
BLECharacteristic* pControlPoint = NULL;
BLECharacteristic* pStatus       = NULL;
BLEAdvertisementData advert;
BLEAdvertisementData scan_response;
BLEAdvertising *pAdvertising;

// {0x2ACD,"Treadmill Data"},
BLECharacteristic TreadmillDataCharacteristics(BLEUUID((uint16_t)0x2ACD),
					       NIMBLE_PROPERTY::NOTIFY
					       );

BLECharacteristic FitnessMachineFeatureCharacteristic(BLEUUID((uint16_t)0x2ACC),
						      NIMBLE_PROPERTY::READ
						      );

BLEDescriptor TreadmillDescriptor(BLEUUID((uint16_t)0x2901)
				  , NIMBLE_PROPERTY::READ,1000
				  );

BLECharacteristic CadenceCharacteristic(BLEUUID((uint16_t)0x2A53), // UUID for Cadence
                                             NIMBLE_PROPERTY::NOTIFY | 
                                             NIMBLE_PROPERTY::READ,1000
                                             );
BLEDescriptor CadenceDescriptor(BLEUUID((uint16_t)0x2901),
                                     NIMBLE_PROPERTY::READ,1000);

// seems kind of a standard callback function
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleClientConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    bleClientConnected = false;
  }
};

void loopHandleBLE()
{
  // if changed to connected ...
  if (bleClientConnected && !bleClientConnectedPrev) {
    bleClientConnectedPrev = true;
    Serial.println("BT Client connected!\n");
  }
  else if (!bleClientConnected && !bleClientConnectedPrev) {
    if (bleClientOpen) {
      delay (1000);
      Serial.println("Waiting for BT Client...\n");
      bleClientOpen = false;
    }
  }
  else if (!bleClientConnected && bleClientConnectedPrev) {
    bleClientConnectedPrev = false;
    bleClientOpen = true;
    Serial.println("BT Client disconnected!\n");
  }
}

// If no name has been provided, use the default name
void initBLE(void) {
  initBLE(DEVICE_NAME);
}

void initBLE(const char* deviceName) {
  Serial.println("init BLE...");
  Serial.print("Starting BLE Service... ");
  Serial.println(deviceName);

  BLEDevice::init(deviceName);  // set server name (here: deviceName)

  // create BLE Server, set callback for connect/disconnect
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create FTMS service
  BLEService *pFTMSService = pServer->createService(FTMSService);
  pFTMSService->addCharacteristic(&TreadmillDataCharacteristics);
  TreadmillDescriptor.setValue("Treadmill Stats");
  TreadmillDataCharacteristics.addDescriptor(&TreadmillDescriptor);
  pFTMSService->addCharacteristic(&FitnessMachineFeatureCharacteristic);

  // Create RSCS service
  BLEService *pRSCSService = pServer->createService(RSCService);
  CadenceDescriptor.setValue("Cadence");
  CadenceCharacteristic.addDescriptor(&CadenceDescriptor);
  pRSCSService->addCharacteristic(&CadenceCharacteristic);



  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  /***************************************************
   NOTE: DO NOT create a 2902 descriptor.
   it will be created automatically if notifications
   or indications are enabled on a characteristic.
  ****************************************************/
  // Start services
  pFTMSService->start();
  pRSCSService->start();

  pAdvertising = pServer->getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->addServiceUUID(FTMSService);
  pAdvertising->addServiceUUID(RSCService);

  // pAdvertising->setMinPreferred(0x06);  // set value to 0x00 to not advertise this parameter

  pAdvertising->start();
  Serial.println("done\n");  
}

void updateBLEdata(float kmph, float incline, float elevationGain, float totalDistance, int cadence)
{
  uint16_t inst_incline = incline;
  uint16_t inst_grade;
  uint16_t inst_elevation_gain = 0;
  uint8_t treadmillData[34] = {};
  uint16_t flags = 0x0018;  // b'000000011000 
  //                             119876543210
  //                             20
  // get speed and incline ble ready
  uint16_t inst_speed = kmph * 100;    // kilometer per hour with a resolution of 0.01 
  inst_incline = incline * 10;         // percent with a resolution of 0.1
  inst_grade = atan2(incline / 100.0, 1.0) * (180.0 / PI) * 10; // Grade (°) = arctan(Incline (%) / 100) * (180 / π)
  inst_elevation_gain = elevationGain * 10;

  // now data is filled starting at bit0 and appended for the
  // fields we 'enable' via the flags above ...
  //treadmillData[0,1] -> flags
  treadmillData[0] = (uint8_t)(flags & 0xFF);
  treadmillData[1] = (uint8_t)(flags >> 8);

  // speed
  treadmillData[2] = (uint8_t)(inst_speed & 0xFF);
  treadmillData[3] = (uint8_t)(inst_speed >> 8);

  // incline & degree
  treadmillData[4] = (uint8_t)(inst_incline & 0xFF);
  treadmillData[5] = (uint8_t)(inst_incline >> 8);
  treadmillData[6] = (uint8_t)(inst_grade & 0xFF);
  treadmillData[7] = (uint8_t)(inst_grade >> 8);

  // Positive Elevation Gain 16 Meters with a resolution of 0.1
  treadmillData[8] = (uint8_t)(inst_elevation_gain & 0xFF);
  treadmillData[9] = (uint8_t)(inst_elevation_gain >> 8);

   // flags do enable negative elevation as well but this will be always 0
  TreadmillDataCharacteristics.setValue(treadmillData, 34);
  TreadmillDataCharacteristics.notify();

  // Prepare the RSCS data for BLE
  uint8_t cadenceValue = (uint8_t)cadence;
  uint8_t cadenceData[5] = {};
  uint16_t cadenceFlags = 0x0004;
  uint16_t cadenceSpeed = (inst_speed / 3.6) * 2.56;    // This is in m/s

  cadenceData[0] = cadenceFlags;

  // cadence sensor speed
  cadenceData[1] = (uint8_t)(cadenceSpeed & 0xFF);
  cadenceData[2] = (uint8_t)(cadenceSpeed >> 8);

  // cadence
  cadenceData[3] = (uint8_t)(cadenceValue & 0xFF);
  cadenceData[4] = (uint8_t)(cadenceValue >> 8);

  // Speed: X.XX m/s, Cadence: XXX RPM
  CadenceCharacteristic.setValue(cadenceData, 5);
  CadenceCharacteristic.notify();
}


void resetStopWatch(void)
{
  setTime(0,0,0,0,0,0);
}


String readSpeed(float kmph)
{
  return String(kmph);
}

String readDist(float totalDistance)
{
  return String(totalDistance / 1000);
}

String readIncline(float incline)
{
  return String(incline);
}

String readElevation(float elevationGain)
{
  return String(elevationGain);
}

String readHour() {
  return String(hour());
}

String readMinute() {
  int m = minute();
  String mStr(m);

  if (m < 10)
    mStr = "0" + mStr;

  return mStr;
}

String readSecond() {
  int s = second();
  String sStr(s);

  if (s < 10)
    sStr = "0" + sStr;

  return sStr;
}

String readCadence(int cadence) {
  return String(cadence);
}