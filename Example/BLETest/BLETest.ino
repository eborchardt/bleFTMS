#include <bleFTMS.h>

typedef struct {
  float kmph;
  float incline;
  float elevationGain;
  float totalDistance;
  int cadence;
} CurrentSettings;

CurrentSettings currentSettings = {
  .kmph = 5,
  .incline = 0.0,
  .elevationGain = 0.0,
  .totalDistance = 0.0,
  .cadence = 180
};

void setup() {
  Serial.begin(9600);
  initBLE();
  updateBLEdata(currentSettings.kmph, 
                currentSettings.incline, 
                currentSettings.elevationGain, 
                currentSettings.totalDistance, 
                currentSettings.cadence);
}

void loop() {
  loopHandleBLE();

  Serial.print("Enter the new speed in X.XX km/h: ");
  while (!Serial.available()) {
  // Wait indefinitely for user input
    updateBLEdata(currentSettings.kmph, 
                  currentSettings.incline, 
                  currentSettings.elevationGain, 
                  currentSettings.totalDistance, 
                  currentSettings.cadence);
    delay(1000);
  }

  String userInputStr = Serial.readStringUntil('\n'); // Wait for Enter key
  currentSettings.kmph = userInputStr.toFloat();
  updateBLEdata(currentSettings.kmph, 
                currentSettings.incline, 
                currentSettings.elevationGain, 
                currentSettings.totalDistance, 
                currentSettings.cadence);

  Serial.print("speed has been updated to: ");
  Serial.println(readSpeed(currentSettings.kmph));
}
