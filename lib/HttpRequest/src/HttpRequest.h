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

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <Arduino.h>
#include <WiFiClient.h>

class HttpRequest{
    private:
        bool parseRawProperties();
        bool parseProperties();

        void clear();

    public:
        uint max_age=0;

        String method="";       // GET/POST
        String URL="";          // /path
        String HTTP_VERSION=""; // 1.1


        // Raw Properies
        String query="";        // GET / HTTP/1.1
        String Host="";         // 192.168.1.13
        String User_Agent;      // Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:74.0) Gecko/20100101 Firefox/74.0
        String Accept;             // text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
        String Accept_Language;    // fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3
        String Accept_Encoding;    // gzip, deflate
        String Connection = "";    // keep-alive
        String Cache_Control = ""; // max-age=0
        String Referer;             
        
        // Original Query
        String fullRequest;

        bool parseFromString(String req);
        String readWebRequest(WiFiClient* _client);     // Read from WiFiClient
        bool parseOneLine(String line);                 // Parsing One line
        bool parseFirstLine(String line);

        bool isError();
        void printDebug();

        String getUrlPath();
};

#endif