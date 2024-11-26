#ifndef BLE_FTMS
#define _BLE_FTMS_H_

extern bool bleClientConnected;

String readHour(void);
String readMinute(void);
String readSecond(void);
String readSpeed(float kmph);
String readDist(float totalDistance);
String readIncline(float incline);
String readElevation(float elevationGain);
String readCadence(int cadence);

void initBLE(void);

void initBLE(void);
void loopHandleBLE(void); 
void updateBLEdata(float kmph, float incline, float elevationGain, float totalDistance, int cadence);
void resetStopWatch(void);

#endif