#ifndef BLE_FTMS
#define BLE_FTMS

// Define the device name with a default value
#ifndef DEVICE_NAME
#define DEVICE_NAME "My Fitness Device"
#endif

extern bool bleClientConnected;

String readHour(void);
String readMinute(void);
String readSecond(void);
String readSpeed(float kmph);
String readDist(float totalDistance);
String readIncline(float incline);
String readElevation(float elevationGain);
String readCadence(int cadence);

void initBLE(const char* deviceName);
void initBLE();

// void initBLE(void);
void loopHandleBLE(void); 
void updateBLEdata(float kmph, float incline, float elevationGain, float totalDistance, int cadence);
void resetStopWatch(void);

#endif