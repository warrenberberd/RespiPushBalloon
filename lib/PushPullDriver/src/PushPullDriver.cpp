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

#include "PushPullDriver.h"

PushPullDriver ICACHE_RAM_ATTR *pointerToDriver;

// Select your Hardware Step Driver here
DRV8825 stepperDRV(MOTOR_STEPS, DIR_PIN, STEP_PIN);
//A4988 stepperDRV(MOTOR_STEPS, DIR_PIN, STEP_PIN);
//DRV8834 stepperDRV(MOTOR_STEPS, DIR_PIN, STEP_PIN);
//DRV8880 stepperDRV(MOTOR_STEPS, DIR_PIN, STEP_PIN);

// If other
//BasicStepperDriver stepperDRV(MOTOR_STEPS, DIR_PIN, STEP_PIN);

WiFiServer webServer(80);

// List of Interrupt callback
static ICACHE_RAM_ATTR void startInterruptCallback(){
    pointerToDriver->start();
}
static ICACHE_RAM_ATTR void stopInterruptCallback(){
    pointerToDriver->stop();
}
static ICACHE_RAM_ATTR void incInterruptCallback(){
    pointerToDriver->incrementSpeed(+5);
}
static ICACHE_RAM_ATTR void decInterruptCallback(){
    pointerToDriver->incrementSpeed(-5);
}

// Initialisation
void PushPullDriver::begin(){
    pointerToDriver=this;   // Setting pointer to interrupt callback management

    Serial.println("PushPullDriver::Setup...");

    Serial.println("PushPullDriver::SetPointer...");
    this->_stepperDRV=&stepperDRV;

    Serial.println("PushPullDriver::Begin SubDriver...");
    this->_stepperDRV->begin(RPM, MICROSTEPS); // configure microstepping
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);

    // Configure the acceleration profile
    //this->setAccel(4.0,7.0);  // NOW this is in main loop

    Serial.println("PushPullDriver::Enabling SubDriver...");
    this->_stepperDRV->enable(); // Lock the motor
    
    // Initialize SPIFFS
    SPIFFS.begin();

    /*// Read Style File
    File cssFile = SPIFFS.open("/styles.css", "r");
    if (!cssFile) {
        Serial.println("styles.css open failed");
    }

    this->cssFileContent=cssFile.readString();
    cssFile.close();

    File jsFile = SPIFFS.open("/custom.js", "r");
    if (!jsFile) {
        Serial.println("custom.js open failed");
    }

    this->jsFileContent=jsFile.readString();
    jsFile.close();*/

    Serial.println("PushPullDriver::begin Ended.");

}

// Pins configuration
void PushPullDriver::configurePins(int LED_PINT,int START_PINT,int STOP_PINT,int INC_PINT,int DEC_PINT){
    this->LED_PIN=LED_PINT;
    this->START_PIN=START_PINT;
    this->STOP_PIN=STOP_PINT;
    this->INC_PIN=INC_PINT;
    this->DEC_PIN=DEC_PINT;
    // Configure stopper pin to read HIGH unless grounded
    //pinMode(STOPPER_PIN, INPUT_PULLUP);

    if(LED_PIN>0) pinMode(LED_PIN,OUTPUT);

    // If connected, attach hardware interrupt to START switch
    if(START_PIN>0){
        pinMode(START_PIN,INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(START_PIN), startInterruptCallback, FALLING);
    }

    // If connected, attach hardware interrupt to STOP switch
    if(STOP_PIN>0){
        pinMode(STOP_PIN,INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(STOP_PIN), stopInterruptCallback, FALLING);
    }

    // If connected, attach hardware interrupt to INCREASE SPEED switch
    if(INC_PIN>0){
        pinMode(INC_PIN,INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(INC_PIN), incInterruptCallback, FALLING);
    }

    // If connected, attach hardware interrupt to DECREASE SPEED switch
    if(DEC_PIN>0){
        pinMode(DEC_PIN,INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(DEC_PIN), decInterruptCallback, FALLING);
    }
}

// End of use
void PushPullDriver::end(void){
    Serial.println("PushPullDriver::end");

    // Unlock motor
    this->_stepperDRV->disable();
}

// To know if values have changed
bool PushPullDriver::isValuesChanged(){
    bool out=this->changedValues;

    if(out){
        Serial.println("Values have changed !");
        this->changedValues=false; // Flip/Flop
    }

    return out;
}

// First install procedure
void PushPullDriver::firstInstall(){
    // if ever configure, we leave
    if(this->maxStepValue>0) return;

    Serial.println("This is the first start of this respirator :");

    // Waiting for console
    delay(1000);

    // Setting default value
    this->setDefaultsValues();

    // Amplitude calibration
    this->amplitudeCalibration();
}

// max courses calibration
void PushPullDriver::amplitudeCalibration(){
    Serial.println("");
    Serial.println("Use + and - keys, for setting the max amplitude");
    Serial.println("<Press Enter, when finish>");

    uint follow=0;      // amplitude set (0 by default)
    uint step=20;       // We increase/decrease with increment of 20 steps

    // Clear buffer
    Serial.flush();

    int nbChar=0;
    char rc;    // The return character

    this->disableLED();

    while(true){
        delay(20);  // 20ms pause
        nbChar=Serial.available();
        if(nbChar<1) continue; // If nothing to read, we leave
        
        //Serial.print("DEBUG : char available : ");
        //Serial.println(nbChar);

        rc=Serial.read();       // Reading the caracter
        if(rc=='\0') continue;  // If null char, bypass
        if((int)rc==255) continue;  // If error char, bypass

        //Serial.flush(); // Clear buffer

        //Serial.print("DEBUG : pressed char : ");
        //Serial.println((int)rc);

        // Increase the width
        if(rc=='+'){
            this->enableLED();
            // Step forward
            this->_stepperDRV->move(step);
            follow+=step;

            this->disableLED();
        }else if(rc=='-'){
            if(follow<0) continue; // if less than 0, ignore

            this->enableLED();

            // Reduce the amplitude
            this->_stepperDRV->move(-step);
            follow-=step;

            this->disableLED();
        }

        // Show current value
        Serial.print("Value : ");
        Serial.println(follow);

        // If 'Enter', exit while
        if(rc=='\n') break;
    }

    Serial.println("End of calibration.");
    this->maxStepValue=follow;      // maxStepValue configuring
    if(!this->saveValues()){         // Saving in EEPROM
        delay(5000);        // If error, we wait
        ESP.restart();      // And reboot ESP
    }

    Serial.println("Return to 0 position...");
    this->_stepperDRV->move(-follow);   // Pull the balloon, to reset the loop
}

// ON the LED
void PushPullDriver::enableLED(){
    if(LED_PIN<0) return;

    digitalWrite(LED_PIN,LOW);
}

// Off the LED
void PushPullDriver::disableLED(){
    if(LED_PIN<0) return;

    digitalWrite(LED_PIN,HIGH);
}

// Push the balloon
void PushPullDriver::push(){
    Serial.print("PUSH with pause ");
    Serial.println(this->pushPause);
    this->enableLED();

    uint oldRPM=this->rpm;
    this->_stepperDRV->setRPM(oldRPM/this->pushSpeedRatio); // For setting different speed, for PUSH

    // Positive for straight forward
    this->_stepperDRV->move(this->nextNbStep);

    this->_stepperDRV->setRPM(oldRPM);  // Reset the default speed

    // Pause after push
    delay(this->pushPause);
}

// Pull the baloon
void PushPullDriver::pull(){
    Serial.print("PULL with pause ");
    Serial.println(this->pullPause);
    this->disableLED();

    uint oldRPM=this->rpm;
    this->_stepperDRV->setRPM(oldRPM/this->pullSpeedRatio); // Setting different speed value, for pull

    // négative, for backward
    this->_stepperDRV->move(-(this->nextNbStep));

    this->_stepperDRV->setRPM(oldRPM);  // Reset the old speed

    // Pause after pull
    delay(this->pushPause);
}

// compute number of step for next cycle
void PushPullDriver::calcNextStepValue(){
    uint nextStep=this->maxStepValue * this->amplitudePct/100;

    if(nextStep!=this->nextNbStep) this->changedValues=true;    //  If value changed, we set the flag (causing save value in main loop)

    this->nextNbStep=nextStep;
}

// On fait un cycle de push/pull
void PushPullDriver::cycle(){
    // compute number of step for next cycle
    this->calcNextStepValue();

    // If amplitude=0%, dont move
    if(this->amplitudePct<1) return;    

    ulong start=millis();

    // Press the balloon
    PushPullDriver::push();

    // Pull the balloon
    PushPullDriver::pull();

    ulong stop=millis();
    ulong delay=stop-start;

    Serial.print("    Cycle duration : ");
    Serial.print(delay/1000.0,3);
    Serial.println(" sec");
}

// To force default calibration values (instead of EEPROM one)
void PushPullDriver::setDefaultsValues(bool razStep /* false by default */){
    Serial.println("PushPullDriver::setDefaultsValues");
    this->speedPct=0;
    this->amplitudePct=0;
    this->maxStepValue=this->nbTurnForPress*MOTOR_STEPS; // par defaut : 20 rotations par compression
    if(razStep){
        Serial.println("  Forcing recalibration...");
        this->maxStepValue=0; // Cela va forcer la recalibration
    }
    this->pullPause=5;
    this->pushPause=0;

    this->saveValues(); // On sauvegarde en EEPROM
}

// Loading values from EEPROM
bool PushPullDriver::loadSavedValues(){
    Serial.println("Reading saved values...");

    // On positionne le point de lecture de la mémoire
    EEPROM.begin(sizeof(savedDataStruct));

    // Lecture des valeurs sauvegardées
    EEPROM.get(RESPI_VALUES_ADRESS,savedDataStruct);
    //flashmem_read(savedDataStruct,RESPI_VALUES_ADRESS,sizeof(savedDataStruct));
    EEPROM.end();

    // Controle de l'intégrité des donnée (on ne maitrise pas ce qu'il y a pu avoir en flash...)
    if(savedDataStruct.speedPct>100 || savedDataStruct.amplitudePct>100) return false;

    // On prépare la structure
    this->amplitudePct=savedDataStruct.amplitudePct;
    this->speedPct=savedDataStruct.speedPct;
    this->maxStepValue=savedDataStruct.maxStepValue;
    this->pullPause=savedDataStruct.pullPause;
    this->pushPause=savedDataStruct.pushPause;
    this->rpm=savedDataStruct.rpm;
    
    Serial.print("\nmaxStepValue : ");
    Serial.println(this->maxStepValue);

    Serial.print("\nSpeed Pct : ");
    Serial.println(this->speedPct);

    Serial.print("\nSpeed Pct : ");
    Serial.println(this->speedPct);

    return true;
}

// Saving in EEPROM
bool PushPullDriver::saveValues(){
    Serial.println("Saving values...");

    // On prépare la structure
    savedDataStruct.amplitudePct=this->amplitudePct;
    savedDataStruct.speedPct=this->speedPct;
    savedDataStruct.maxStepValue=this->maxStepValue;
    savedDataStruct.pullPause=this->pullPause;
    savedDataStruct.pushPause=this->pushPause;
    savedDataStruct.rpm=this->rpm;

    // On positionne le point de lecture de la mémoire
    EEPROM.begin(sizeof(savedDataStruct));

    // Ecriture des valeurs sauvegardées
    EEPROM.put(RESPI_VALUES_ADRESS,savedDataStruct);

    // On ecrit ca en EEPROM
    EEPROM.commit();

    EEPROM.end();
    EEPROM.begin(sizeof(savedDataStruct));


    // On modifie la structur pour comparere la lecture
    savedDataStruct.maxStepValue=0;

    // On relie, pour verifier
    EEPROM.get(RESPI_VALUES_ADRESS,savedDataStruct);

    // Si la valeur en mémoire, ne correspond pas a ce qu'on voulais stocker
    if(savedDataStruct.maxStepValue!=this->maxStepValue){
        Serial.println("ERREUR : La sauvegarde des valeurs a echouee !!!!");
        return false;
    }

    EEPROM.end();

    Serial.println("Values successfully saved.");
    return true;
}

// Increase (or decrease) speed
bool PushPullDriver::incrementSpeed(int inc){
    this->speedPct+=inc;
    if(this->speedPct<0) this->speedPct=0;
    if(this->speedPct>200) this->speedPct=200;

    if(this->speedPct>0){
        Serial.print("PushPullDriver::incrementSpeed ");
        Serial.println(this->speedPct);
    }

    // On calcul la vitesse de rotation
    this->rpm=RPM*this->speedPct/100;
    this->_stepperDRV->setRPM(this->rpm);

    // Si l'amplitude est désactivée, on l'active
    //if(this->speedPct>0 && this->amplitudePct<1) this->amplitudePct=100;

    return true;
}

// Set the speed to a specific value
bool PushPullDriver::setSpeed(int val){
    this->speedPct=val;
    if(this->speedPct<0) this->speedPct=0;
    if(this->speedPct>200) this->speedPct=200;

    if(this->speedPct>0){
        Serial.print("PushPullDriver::setSpeed ");
        Serial.println(this->speedPct);
    }

    // On calcul la vitesse de rotation
    this->rpm=RPM*this->speedPct/100;
    this->_stepperDRV->setRPM(this->rpm);

    // Si l'amplitude est désactivée, on l'active
    //if(this->speedPct>0 && this->amplitudePct<1) this->amplitudePct=100;

    return true;
}

// Reading inputs, searching for modify to apply
bool PushPullDriver::readInputs(){
    if(this->readFromSerial()) return true; // Read from Serial
    if(this->readFromWifi()) return true;   // Read from wifi

    return false;
}

// Reading inputs from Wifi WebServer
bool PushPullDriver::readFromWifi(){
    //Serial.println("PushPullDriver::readFromWifi()");
    
    WiFiClient webClient = this->_webServer->available();   // Listen for incoming clients
    if(!webClient.connected()) return false;

    //Serial.println("The is a WebClient connected !");

    // Parsing Query
    HttpRequest reqOBJ;
    //reqOBJ.parseFromString(req);

    // To know what page is called
    reqOBJ.readWebRequest(&webClient);

    // For debugging HttpRequest Parsing
    //reqOBJ.printDebug();

    // If query is empty 
    if(reqOBJ.isError()){
        //Serial.println("Web Request is error");
        this->sendRequestHeader(&webClient,500,"","");
        webClient.stop();
        return false;
    }

    // What is the URL query
    String req=reqOBJ.getUrlPath();

    // Processing of the query
    bool isProcessingDone=this->processWebQuery(req);

    // Get the reponse page
    String resp = this->getWebResponse(req,isProcessingDone);

    String contentType="text/html";
    String cacheControl="no-cache";

    int locator=0;

    // For custom content-type
    if(resp.startsWith("Content-Type: ")){
        locator=resp.indexOf("\n");
        contentType=resp.substring(0,locator);
        contentType.replace("Content-Type: ","");
        contentType.trim();

        //Serial.println("  Content-Type for " + req + " : " + contentType);

        resp=resp.substring(locator+1);
    }

    // For custome Cache-Control
    if(resp.startsWith("Cache-Control: ")){
        locator=resp.indexOf("\n");
        cacheControl=resp.substring(0,locator);
        cacheControl.replace("Cache-Control: ","");
        cacheControl.trim();

        //Serial.println("  Cache-Control for " + req + " : " + cacheControl);

        resp=resp.substring(locator+1);
    }


    // Send OK header
    this->sendRequestHeader(&webClient,200,contentType,cacheControl);

    webClient.print(resp);  // We send the response

    webClient.stop(); // closing connection
    return isProcessingDone;
}

// Reading speed modification from Serial
bool PushPullDriver::readFromSerial(){
    if(Serial.available()<1) return false; // If nothing to read, exit

    int stepPct=5;

    char rc=Serial.read();
    if(rc=='\0') return false;      // If 'null char' caractere, bypass
    if((int)rc==255) return false;  // Error caractere

    //Serial.flush(); // Purge the buffer

    // Si c'est l'allumage :
    //if(this->amplitudePct<1 && this->speedPct<1) stepPct=100; // On démarre a 100% (plus facile de réduire les cycle)

    if(rc=='+') return this->incrementSpeed(stepPct);
    if(rc=='-') return this->incrementSpeed(-stepPct);
    if(rc=='0') return this->stop();
    if(rc=='*') return this->start();

    return false;
}

// Modify RPM SPEED
void PushPullDriver::setRPM(int rpmT){
    this->rpm=rpmT;
    this->_stepperDRV->setRPM(this->rpm);
}

// Modify the acceleration profile
void PushPullDriver::setAccel(uint accel,uint decel){
    //int accelT=1000 * accel; // Accélération en Full Step/s^2
    //int decelT=1000 * decel; // Décélération en Full Step/s^2
    this->_stepperDRV->setSpeedProfile(this->_stepperDRV->LINEAR_SPEED,accel,decel);
}

// Set the number of full turn, by default, for full translation (to call BEFORE the setDefaultsValues)
void PushPullDriver::setNbTurnDefault(uint nb){
    this->nbTurnForPress=nb;
}

// Speed Ration between Main RPM with push, and Main RPM with pull
void PushPullDriver::setPushPullSpeedRatio(float pushRatio, float pullRatio){
    this->pushSpeedRatio=pushRatio;
    this->pullSpeedRatio=pullRatio;
}

// For stopping
bool PushPullDriver::stop(){
    Serial.println("PushPullDriver::stop");
    this->amplitudePct=0;

    return true;
}

// For relaunch, when stopped
bool PushPullDriver::start(){
    Serial.println("PushPullDriver::start");

    this->amplitudePct=100;

    // If SpeedPct is 0, we set to 100
    if(this->speedPct<1) this->incrementSpeed(100);

    return true;
}

// For setting internal pointer to WiFiManager instance
void PushPullDriver::setWifiManager(WiFiManager* _wifi){
    this->_wifiManager=_wifi;

    // Set web server port number to 80
    this->_webServer=&webServer;


    // Starting listen on port 80
    this->_webServer->begin();

}

// Compute the page to response
String PushPullDriver::getWebResponse(String req,bool isProcessingDone){
    if(req.equals("/enable")) return this->getWebPageForEnable(req,isProcessingDone);
    if(req.equals("/disable")) return this->getWebPageForDisable(req,isProcessingDone);
    if(req.startsWith("/increase/")) return this->getWebPageForIncrease(req,isProcessingDone);
    if(req.startsWith("/decrease/")) return this->getWebPageForDecrease(req,isProcessingDone);
    if(req.startsWith("/setRPM/")) return this->getWebPageForSetRPM(req,isProcessingDone);
    if(req.startsWith("/setSpeed/")) return this->getWebPageForSetSpeed(req,isProcessingDone);
    if(req.startsWith("/status")) return this->getWebPageForStatus(req,isProcessingDone);
    //if(req.equals("/styles.css")) return this->getStylesPage(req,isProcessingDone);
    //if(req.equals("/custom.js")) return this->getJsPage(req,isProcessingDone);

    // Main Page
    if(req.equals("/")){
        // Status Page
        String out="";


        String status="Disabled";
        if(this->amplitudePct>0) status="Enabled";

        String header=this->getRawPage("/header.html");
        header.replace("__RESPIRATOR_STATUS__",status);
        out+=header;

        out+=this->getWebStatusPage(req,isProcessingDone) + "\n";

        out+=this->getRawPage("/footer.html");

        return out;
    }

    // For static pages
    // If not custom page, read from SPIFFS
    return this->getRawPage(req);
}

// Get the Ajax Response Page for /enable
String PushPullDriver::getWebPageForEnable(String req,bool isProcessingDone){
    if(isProcessingDone) return "PushPull was Enabled";

    return "Unable to Enabling the PushPull !!";
}

// Get the Ajax Response Page for /disable
String PushPullDriver::getWebPageForDisable(String req,bool isProcessingDone){
    if(isProcessingDone) return "PushPull was Disabled";

    return "Unable to Disabling the PushPull !!";
}

// Get the Ajax Response Page for /increase/val
String PushPullDriver::getWebPageForIncrease(String req,bool isProcessingDone){
    if(isProcessingDone) return "The speed was increase";

    return "Unable to Increase the speed !!";
}

// Get the Ajax Response Page for /decrease/val
String PushPullDriver::getWebPageForDecrease(String req,bool isProcessingDone){
    if(isProcessingDone) return "The speed was lowered";

    return "Unable to Decrease the speed !!";
}

// Get the Ajax Response Page for /setRPM
String PushPullDriver::getWebPageForSetRPM(String req,bool isProcessingDone){
    if(isProcessingDone) return "The RPM was modified";

    return "Unable to modify the RPM !!";
}

// Get the Ajax Response Page for /setSpeed
String PushPullDriver::getWebPageForSetSpeed(String req,bool isProcessingDone){
    if(isProcessingDone) return "The Speed Percent was modified";

    return "Unable to modify the Speed Percent !!";
}

// Get the Ajax Response Page for /status
String PushPullDriver::getWebPageForStatus(String req,bool isProcessingDone){
    String out="";

    out+="Content-Type: application/json\n";
    out+="{";

    // amplitudePct
    out+="\"amplitudePct\": ";
    out+=this->amplitudePct;
    out+=",\n";

    // speedPct
    out+="\"speedPct\": ";
    out+=this->speedPct;
    out+=",\n";

    // RPM
    out+="\"RPM\": ";
    out+=this->rpm;
    out+=",\n";

    // pushSpeedRatio
    out+="\"pushSpeedRatio\": ";
    out+=this->pushSpeedRatio;
    out+=",\n";

    // pullSpeedRatio
    out+="\"pullSpeedRatio\": ";
    out+=this->pullSpeedRatio;
    out+=",\n";

    // maxStepValue
    out+="\"maxStepValue\": ";
    out+=this->maxStepValue;
    out+=",\n";

    // pullPause
    out+="\"pullPause\": ";
    out+=this->pullPause;
    out+=",\n";

    // pushPause
    out+="\"pushPause\": ";
    out+=this->pushPause;
    out+=",\n";

    // nextNbStep
    out+="\"nextNbStep\": ";
    out+=this->nextNbStep;
    out+="\n";

    out+="}\n";

    return out;
}

// Get the styles.css page
/*String PushPullDriver::getStylesPage(String req,bool isProcessingDone){
    return this->cssFileContent;
}*/

// Get the custom.js page
/*String PushPullDriver::getJsPage(String req,bool isProcessingDone){
    return this->jsFileContent;
}*/

// Reading raw static file from SPIFFS
String PushPullDriver::getRawPage(String req){
    String out="";

    //Serial.println("Retrieve raw page : '" + req  + "'");

    if(req.endsWith(".css")) out+="Content-Type: text/css\n";
    if(req.endsWith(".js"))  out+="Content-Type: text/javascript\n";

    if(!out.isEmpty()) out+="Cache-Control: max-age=600\n";

    File file = SPIFFS.open(req, "r");
    if (!file) {
        Serial.println(req + " open failed");

        out+="File not found\n";

        return out;
    }

    out+=file.readString();
    file.close();

    return out;
}


bool PushPullDriver::sendRequestHeader(WiFiClient* _client,const uint http_code,String contentType,String cacheControl){
    String statusMSG="OK";
    if(http_code>=300) statusMSG="Redirect";
    if(http_code>=400) statusMSG="User Error";
    if(http_code>=500) statusMSG="Server Internal Error";

    String httpFirstLine="HTTP/1.1 ";
    httpFirstLine+=http_code;
    httpFirstLine+=" ";
    httpFirstLine+=statusMSG;

      // header
    _client->println(httpFirstLine);
    if(!contentType.isEmpty()) _client->println("Content-type: " + contentType);
    if(!cacheControl.isEmpty()) _client->println("Cache-Control: " + cacheControl);
    _client->println("Connection: close");
    _client->println();

    return true;
}

// Processing of Web Request
bool PushPullDriver::processWebQuery(String req){
    if(req.equals("/enable")){
        return this->start();
    }
    if(req.equals("/disable")){
        return this->stop();
    }

    int inc=0;
    if(req.startsWith("/increase/")){
        String value=req.substring(10);
        inc=value.toInt();

        return this->incrementSpeed(inc);
    }
    if(req.startsWith("/decrease/")){
        String value=req.substring(10);
        inc=value.toInt();

        return this->incrementSpeed(-inc);
    }
    if(req.startsWith("/setSpeed/")){
        String value=req.substring(10);
        inc=value.toInt();

        return this->setSpeed(inc);
    }
    if(req.startsWith("/setRPM/")){
        String value=req.substring(8);
        inc=value.toInt();

        this->setRPM(inc);
        return true;
    }

    return false;
}

// HTML Page for the status
String PushPullDriver::getWebStatusPage(String req,bool isProcessingDone){
    String out="";

    out+="<table>\n";

    out+=this->getHTMLOneLineStatus("Amplitude Percent",String(this->amplitudePct),"SLIDER");
    out+=this->getHTMLOneLineStatus("Speed Percent",String(this->speedPct),"INCDEC");
    out+=this->getHTMLOneLineStatus("RPM",String(this->rpm));
    out+=this->getHTMLOneLineStatus("Push Speed Ratio",String(this->pushSpeedRatio));
    out+=this->getHTMLOneLineStatus("Pull Speed Ratio",String(this->pullSpeedRatio));

    out+="</table>\n";

    return out;
}

String PushPullDriver::getHTMLOneLineStatus(String name, String value, String withInput){
    String out;

    String smallName="valueOf_" + name;
    smallName.replace(" ","");


    out+="<tr>\n";
    out+="<td bold=1>" + name + ":</td>\n";
    out+="<td id='" + smallName + "'>" + value + "</td>\n";

    if(!withInput.isEmpty()){
        if(withInput.equals("SLIDER")){
            out+="<td>";

            String checked="";
            if(value.equals("100") || value.equals("true")) checked="checked";

            String subHTML=this->getRawPage("/slider.html");
            subHTML.replace("__CHECKED__",checked);
            subHTML.replace("__NAME__",name);
            out+=subHTML;

            out+="</td>\n";
        }else if(withInput.equals("INCDEC")){
            out+="<td>";

            String checked="";
            if(value.equals("100") || value.equals("true")) checked="checked";

            String subHTML=this->getRawPage("/IncDecButtons.html");
            subHTML.replace("__VALUE__",value);
            subHTML.replace("__NAME__",name);
            out+=subHTML;

            out+="</td>\n";
        }
    }

    out+="</tr>\n";
    return out;
}