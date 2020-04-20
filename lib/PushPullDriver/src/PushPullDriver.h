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

#ifndef PUSHPULLDRIVER_H
#define PUSHPULLDRIVER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <WiFiManager.h>

// For reading file from SPIFFS
#include <FS.h>

// For parsing HTTP Query
#include "HttpRequest.h"

// this pin should connect to Ground when want to stop the motor
//#define STOPPER_PIN 4

#define RPM         600
// Microstepping mode. If you hardwired it to save pins, set to the same value here.
//#define MICROSTEPS 16
#define MICROSTEPS  1

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200

#define DIR_PIN     D4      // Direction PIN
#define STEP_PIN    D3      // Step PIN

#define SLEEP_PIN -1        // optional (just delete SLEEP from everywhere if not used)
#define MODE0 -1
#define MODE1 -1
#define MODE2 -1

// For the DRV8834
// #include "DRV8834.h"

// For the A4988
//#include "A4988.h"

// For the DRV8825
#include "DRV8825.h"

// For the DRV8880
// #include "DRV8880.h"

// Generic driver, if necessary
#include "BasicStepperDriver.h" // generic


// EEPROM
#define RESPI_VALUES_ADRESS 0x0     // Start Adress in EEPROM for storing Values

// EEPROM Data structure
struct {
    uint speedPct = 0;          // Selected Speed Percent (0=dont move)
    uint amplitudePct;          // Compression percent

    // Not configure, by default, to force calibration
    uint maxStepValue=0;          // Absolute max number of steps for full compression

    uint pullPause;          // Wait after pull (in ms)
    uint pushPause;          // Wait after push (in ms)

    uint rpm=RPM;                // Speed RPM
} savedDataStruct;

class PushPullDriver{
    private:
        BasicStepperDriver*   _stepperDRV;
        WiFiManager*          _wifiManager;
        WiFiServer*           _webServer;

        //String cssFileContent;
        //String jsFileContent;

        //DRV8825*                _stepperDRV;

        int LED_PIN=-1;
        int START_PIN=-1;
        int STOP_PIN=-1;
        int INC_PIN=-1;
        int DEC_PIN=-1;

        bool changedValues=false;               // Flag to know if values have changed

        uint amplitudePct;                      // Compression percent
        uint speedPct;                          // Speed percent
        uint maxStepValue;                      // Absolute max step value for complete press (from calibration)

        uint rpm;                               // Rotation speed
        uint nbTurnForPress=13;                 // Number of turn for complet press (The default value)

        uint pullPause;                         // Wait time after pull
        uint pushPause;                         // Wait time after push

        float pushSpeedRatio=1.2;
        float pullSpeedRatio=1.0;

        uint nextNbStep;                        // Number of the for the current (or next) cycle

        void calcNextStepValue();               // Compute the nb of steps for the next cycle

        void enableLED();                       // Turn the LED ON
        void disableLED();                      // Turn the LED OFF

    public:
        PushPullDriver(){}                      // Constructor
        ~PushPullDriver(){end();}               // Destructor

        void begin();                           // Init
        void configurePins(int LED_PINT,int START_PINT,int STOP_PINT,int INC_PINT,int DEC_PINT);                   // Pin configuration
        void end();                             // Stopping the driver

        bool isValuesChanged();                 // To know if values have changed (and need to be saved)

        void firstInstall();                    // First install procedure
        void amplitudeCalibration();            // Amplitude calibration procedure

        void setDefaultsValues(bool razStep=false);   // To force calibration value, and/or recalibration

        void cycle();                           // Do a push/pull cycle

        void push();                            // Push the balloon
        void pull();                            // Pull the balloon

        bool loadSavedValues();                 // Loading values from EEPROM
        bool saveValues();                      // Backup values in EEPROM

        bool incrementSpeed(int inc);           // Increase (or decrease) speed
        bool setSpeed(int val);                 // To set the speed to a specific value

        bool readInputs();                      // Reading from all sources
        bool readFromSerial();                  // Reading from Serial console
        bool readFromWifi();                    // Reading from WiFi

        void setRPM(int rpm);                   // To modify the speed RPM
        void setAccel(uint accel,uint decel);   // To modify acceleration parameter

        void setNbTurnDefault(uint nb);         // Define the nbTurnForPress
        void setPushPullSpeedRatio(float pushRatio, float pullRatio);

        bool stop();                            // Stop the cycle
        bool start();                           // Start the cycle

        void setWifiManager(WiFiManager* _wifi); // For configuring pointer to WiFiManager instance

        String getWebResponse(String req,bool isProcessingDone);      // get page for the request URL
        bool sendRequestHeader(WiFiClient* _client,uint http_code,String contentType,String cacheControl); // Envoie du header
        bool processWebQuery(String req);       // Processing of Web Request

        String getWebPageForEnable(String req,bool isProcessingDone); 
        String getWebPageForDisable(String req,bool isProcessingDone); 
        String getWebPageForIncrease(String req,bool isProcessingDone);
        String getWebPageForDecrease(String req,bool isProcessingDone);
        String getWebPageForSetRPM(String req,bool isProcessingDone);
        String getWebPageForSetSpeed(String req,bool isProcessingDone);
        String getWebPageForStatus(String req,bool isProcessingDone);
        //String getStylesPage(String req,bool isProcessingDone);
        //String getJsPage(String req,bool isProcessingDone);
        String getRawPage(String req);
        
        String getWebStatusPage(String req,bool isProcessingDone);
        String getHTMLOneLineStatus(String name,String value, String withInput="");
};

#endif /* PUSHPULLDRIVER_H */