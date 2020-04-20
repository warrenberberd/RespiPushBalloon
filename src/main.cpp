/*
    This code is attended to drive a small and LowCost Respirator.
    It was release in the COVID-19 Crisis
    
    Copyright (C) 2020 Alban Ponche

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Arduino.h>

// This constant is define in PushPullDriver.h :
// #define DIR_PIN  D4      // Direction PIN
// #define STEP_PIN D3      // Step PIN
#include <PushPullDriver.h> 
//#include <HttpRequest.h>

// To have Wifi Access
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
//#include <DNSServer.h>
//#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// README:  Running instruction
//    * => start
//    0 => stop
//    + => Increase speed
//    - => Decrease speed

// The driver
PushPullDriver ppDriver;

// Wifi Manager
WiFiManager wifiManager;
#define DEFAULT_WIFI_SSID "RESPI-AP"
#define DEFAULT_WIFI_PASSWORD "49dcc193b7bf33625fe9a8a33187df36"

// Force first saving values
bool changedValues=true;

#define PIN_UNCONNECTED -1

// Hardware button mapping
// Button for +, -, start and stop
//#define LED_PIN   LED_BUILTIN
#define LED_PIN   PIN_UNCONNECTED
#define STOP_PIN  D5  // Switch to stop  cycle
#define START_PIN D6  // Switch to start cycle
#define INC_PIN   D1  // Switch to increase speed
#define DEC_PIN   D2  // Switch do decrease speed

// Configuration
void setup() {
  Serial.begin(115200);
  while (!Serial) ; // wait for Arduino Serial Monitor to open

  Serial.println("");
  delay(1000);

  Serial.println("main::Setup...");

  //first parameter is name of access point, second is the password
  //wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
  wifiManager.autoConnect(); // For auto-generated SSID

  ppDriver.setWifiManager(&wifiManager);

  // Uncomment to force recalibration
  //ppDriver.setDefaultsValues(true);

  // Configuration of PIN mode
  ppDriver.configurePins(LED_PIN,START_PIN,STOP_PIN,INC_PIN,DEC_PIN);

  ppDriver.setNbTurnDefault(13); // Default number of turn

  // Uncomment to force default settings values
  //ppDriver.setDefaultsValues();

  ppDriver.begin();

  // Loading saved values
  ppDriver.loadSavedValues();

  // For to start OFF (not breathing at poweron)
  ppDriver.stop();
  
  // To force RPM
  ppDriver.setRPM(600);           // Modify max SPID (Max: 600)
  ppDriver.setAccel(10000,15000); // acceleration/deceleration profile
  ppDriver.setPushPullSpeedRatio(1.2, 1.0);

  // To launch the calibration step, if necessary
  ppDriver.firstInstall();

  ppDriver.saveValues();

  Serial.println("main::End of setup.");
}

// Main Loop
void loop() {
  // Push and pull the baloon
  ppDriver.cycle();

  // To save changed values in EEPROM
  if(changedValues || ppDriver.isValuesChanged()){
    ppDriver.saveValues();
    changedValues=false;
  }

  changedValues=ppDriver.readInputs();

  delay(2); // Prevent crash in main loop
}
