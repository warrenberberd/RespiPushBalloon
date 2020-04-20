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

#include "HttpRequest.h"

/* bool HttpRequest::parseFromString(String req){
    this->clear();

    this->fullRequest=req;

    if(!this->parseRawProperties()) return false;
    if(!this->parseProperties()) return false;

    return true;
} */

// Clear old value
void HttpRequest::clear(){
    this->method="";
    this->URL="";
    this->Accept="";
    this->Accept_Encoding="";
    this->Accept_Language="";
    this->Cache_Control="";
    this->Connection="";
    this->fullRequest="";
    this->User_Agent="";
    this->Referer="";
}

bool HttpRequest::isError(){
    if(this->URL.isEmpty()) return true;

    return false;
}

/* bool HttpRequest::parseRawProperties(){
    // this->fullRequest

    uint length = this->fullRequest.length();



    return true;
}

bool HttpRequest::parseProperties(){
    
    return true;
}
*/

// Parse argument from First Line header
bool HttpRequest::parseFirstLine(String line){
    // GET / HTTP/1.1
    char separator=' ';

    int oldIdx=-1;
    uint newIdx=0;

    // Method
    newIdx=line.indexOf(separator,oldIdx+1);
    this->method=line.substring(oldIdx+1,newIdx);
    oldIdx=newIdx;

    // Path
    newIdx=line.indexOf(separator,oldIdx+1);
    this->URL=line.substring(oldIdx+1,newIdx);
    oldIdx=newIdx;

    // Http version
    //newIdx=line.indexOf(separator,oldIdx+1);
    this->HTTP_VERSION=line.substring(oldIdx+1);
    //oldIdx=newIdx;

    return true;
}

// Parse one line of Query
bool HttpRequest::parseOneLine(String line){
    char separator=':';

    line.trim();

    // The first line doesnt contains separator
    if(line.startsWith("GET "))  return this->parseFirstLine(line);
    if(line.startsWith("POST ")) return this->parseFirstLine(line);
    if(line.startsWith("HEAD ")) return this->parseFirstLine(line);

    uint firstPartIdx=line.indexOf(separator,0);
    String param=line.substring(0,firstPartIdx);

    if(param.isEmpty()) return false;

    /*
        String Host="";         // 192.168.1.13
        String User_Agent;      // Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:74.0) Gecko/20100101 Firefox/74.0
        String Accept;             // text/html,application/xhtml+xml,application/xml;q=0.9,image/webp, ;q=0.8
        String Accept_Language;    // fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3
        String Accept_Encoding;    // gzip, deflate
        String Connection = "";    // keep-alive
        String Cache_Control = ""; // max-age=0
    */

    String value=line.substring(firstPartIdx+2);    // Start at +2 beause separator is followed by space

    if(param.equals("Host")){
        this->Host=value;
    }else if(param.equals("User-Agent")){
        this->User_Agent=value;
    }else if(param.equals("Accept")){
        this->Accept=value;
    }else if(param.equals("Accept-Language")){
        this->Accept_Language=value;
    }else if(param.equals("Accept-Encoding")){
        this->Accept_Encoding=value;
    }else if(param.equals("Connection")){
        this->Connection=value;
    }else if(param.equals("Cache-Control")){
        this->Cache_Control=value;
    }else if(param.equals("Referer")){
        this->Referer=value;
    }else if(param.equals("Upgrade-Insecure-Requests")){
        return false;
    }else if(param.equals("Save-Data")){
        return false;
    }else if(param.equals("Pragma")){
        return false;
    }else{
        Serial.println("Unknown HTTP parameter : " + param);
        Serial.println("Value : " + value);
        return false;
    }

    //Serial.println(param + " parsed.");

    return true;
}

// Decode input WebQuery
String HttpRequest::readWebRequest(WiFiClient* _client){
    this->clear();  // We clear old values

    String query;
    if(!_client->connected()){
        Serial.println(" nobody is Connected");
        return "";    // There is nobody connected
    }

    uint MAX_ITERATION_TIMEOUT=500;  // 500*10ms = 5sec
    for(uint i=0;i<=MAX_ITERATION_TIMEOUT;++i){
        delay(10);

        // If data is ready, leave the wiating loop
        if(_client->available()>0) break;
    }

    //delay(10);

    //Serial.println("There is a client connected");
    if(_client->available()<1){
        Serial.println("No data to read");
        return "";
    }
    //Serial.println("We have data to read");

    String line="";
    char separator='\r';

    // While there is something to read
    while(_client->available()>0){
        char rc=(char)_client->read(); // Concat the char with the query

        // If we reach endOfLine, we trigger the parsing of the line (more effective than spliting lines)
        if(rc!=separator){
            line+=rc;
        }else{
            this->parseOneLine(line);   // Parsing of line
            line="";    // reset the line buffer
        }

        query+=rc;
    }

    query.trim(); //triming the string
    this->fullRequest=query;

    // Response with ack
    //Serial.println("Query : " + query);

    return query;
}

// Get the requested URL
String HttpRequest::getUrlPath(){
    return this->URL;
}

// Print the Query For Debugging
void HttpRequest::printDebug(){
    Serial.println("Query : " + this->method + " " + this->URL + " " + this->HTTP_VERSION);
    Serial.println("Host: " + this->Host);
    Serial.println("User-Agent: " + this->User_Agent);
    Serial.println("Accept: " + this->Accept);
    Serial.println("Accept_Encoding: " + this->Accept_Encoding);
    Serial.println("Accept_Language: " + this->Accept_Language);
    Serial.println("Cache_Control: " + this->Cache_Control);
}