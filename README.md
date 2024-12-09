# bleFTMS

Based mostly on the net-control library of https://github.com/lefty01/ESP32_TTGO_FTMS. I removed all of the MQTT and WiFi stuff, since I only want to use the BLE FTMS features. I also added the Running Speed and Cadence Service (RSCS), so that I can also feed the running cadence to Zwift. I left all of the readParameter functions in place, as they might come in handy later. 

The bleFTMS library provides a simple and easy-to-use interface for interacting with Bluetooth Low Energy (BLE) Fitness Machine Service (FTMS) devices. This library allows you to read and update various fitness-related data, such as speed, distance, incline, elevation gain, and cadence.

### Features
* Read and update fitness data, including:
  * Speed (km/h)
  * Distance (km)
  * Incline (%)
  * Elevation gain (m)
  * Cadence (rpm)
* Handle BLE client connections and disconnections
* Serial debugging output

### Usage
To use bleFTMS to send the data from your treadmill, you only need a couple of commands:
##### Initialize the BLE services
    initBLE();

##### Update the current values
    updateBLEdata(currentSettings.kmph, 
                currentSettings.incline, 
                currentSettings.elevationGain, 
                currentSettings.totalDistance, 
                currentSettings.cadence);
