/*
  Modified by tlitre 2018
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://premiereepass.local *edit* dns will catch all request and response with 302
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>

#define MAX_ATTEMPTS 120

const int LED_PIN = 5;

//set ssid and password for wifi network
const char *ssid = "PremierPass";
const char *password = "premierpass";

/* hostname for mDNS. Should work at least on windows. Try http:/premierpass.local */
const char *myHostname = "premierpass";

const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"3; url=http://192.168.4.1/index.htm\" /></head><body><p>redirecting...</p></body>";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

bool drillRunning = false;

unsigned long previousMillis = 0;
/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1); // note: update metaRefreshStr string if ip change!
IPAddress netMsk(255, 255, 255, 0);

// Web server
ESP8266WebServer server(80);

//Player Info on login
String playerName = "";
int numBalls = 0;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

//SPIFFS helper programs
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}

//Called when the user picks number of balls and times in the webapp.
//Communicates parameters via Serial to Arduino to start the drill.
void handleStart()
{
  //init with default values
  int touchTime = 6;
  playerName = server.arg("name");
  numBalls = server.arg("balls").toInt();
  touchTime = server.arg("touchT").toInt();
  previousMillis = millis();
  //while(Serial.read() != '#' && millis() - previousMillis < 5000){
  Serial.write('%');
  Serial.write(numBalls);
  Serial.write(touchTime);
  Serial.write('#');
    //yield();
  //}

    server.send(200, "text/plain", "Drill starting with your parameters: " + String(numBalls) + " " + String(touchTime));
    digitalWrite(LED_PIN, 1);

  //Serial.println(playerName);
  //Serial.println(balls);
  //Serial.println(touchTime);
  drillRunning = true;
}

//Called when the user submits their name.
//Pulls up their player file from SD.
void handleStats()
{
  //Serial.println("getting stats!");
  playerName = String(server.arg("name"));
  playerName.toUpperCase();
  //Serial.println("checking if " + playerName + " exists");
  SDFile userFile = SD.open(playerName + ".TXT");
  if(userFile.available()){
    //Serial.println("Player found");
    String playerData = "";
    while(userFile.available()){
      playerData += (char) userFile.read();
    }
    server.send(200, "text/plain", playerData);
  }else{
    //Serial.println("Player Not Found");
    server.send(201, "text/plain", playerName + " is not in the system");
  }

}

//Called onload to get leaderboard values.
void handleLeaderboard(){
  SDFile leaderBoard = SD.open("LEADERS.TXT", FILE_READ);
  String lead = "";
  while(leaderBoard.available()){
    lead += (char) leaderBoard.read();
  }
  server.send(200, "text/plain", lead);
}

//Called if a player gets a new personal high score.
void updateLeader(int score) {
  SDFile leaderBoard = SD.open("LEADERS.TXT", FILE_WRITE);
  char nameStart = (char) playerName[0];
  while(leaderBoard.available()){
    if(nameStart == (char) leaderBoard.read()){
      bool found = true;
      for(int i = 1; i < playerName.length() && found == true; i++){
        char next = (char) leaderBoard.read();
        found = (char) playerName[i] == next;
      }
      if(found){
        leaderBoard.read();
        if(score == 10){
          leaderBoard.print('1');
          leaderBoard.print('0');
        }else{
          leaderBoard.print('0');
          leaderBoard.print((char) score);
        }
      }
    }
  }
  leaderBoard.close();
}

//Called when the user does not exist in the system and opts to make a new profile.
//Creates a new player file for them with default values and puts them on leaderboard.
void handleNewUser() {
  playerName = String(server.arg("name"));
  playerName.toUpperCase();
  SDFile userFile = SD.open(playerName + ".txt", FILE_WRITE);
  userFile.print('00');
  userFile.print('00');
  userFile.close();
  SDFile lead = SD.open("LEADERS.TXT", FILE_WRITE);
  lead.println(playerName + ",00,");
  lead.close();
  server.send(200, "text/plain", "Profile created");
}

//Called when the drill is over.
//Writes results to player's file.
void writeResults(){
  //TODO make sure to check if the first number in the file is lower/higher if it's out of 10
  //if it is, then add it to the leaderboard page and recalculate
  //also check for last attempt which is in the second spot
  String hits = "";
  int successes = 0;
  bool newHigh = false;
  for(int i = 0; i < numBalls; i++){
    int curr = Serial.read();
    hits += String(curr);
    if(curr == 1) successes++;
  }

  SDFile userFile = SD.open(playerName + ".txt", FILE_WRITE);
  userFile.print(String(successes * 100 /numBalls));
  if(numBalls == 10){ //if it's a 10 ball attempt
    String currHigh = ""; //read current player high score from SD file
    currHigh += userFile.read();
    currHigh += userFile.read();
    char attempt[2];
    if(successes == 10){
      attempt[0] = '1';
      attempt[1] = '0';
    }else{
      attempt[0] = '0';
      attempt[1] = char(successes);
    }
    if(currHigh.toInt()<successes){ //check if attempt is higher
      newHigh = true;
      userFile.seek(0);
      userFile.print(attempt[0]);
      userFile.print(attempt[1]);
      userFile.flush();
    }
    userFile.seek(2);
    userFile.print(attempt[0]); //either way record this as last attempt
    userFile.print(attempt[1]);
  }
  userFile.close();
  if(newHigh) updateLeader(successes);
  newHigh = false;
  server.send(202, "text/plain", "Attempt logged");
}

void setup(void){
  //Start SPIFFS
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
    }
  }


  //WIFI INIT
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  // Setup MDNS responder
  if (!MDNS.begin(myHostname)) {
  } else {
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }

  //SERVER INIT
  //list directory
  server.on("/listFiles", HTTP_GET, handleFileList);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(302, "text/html", metaRefreshStr);
  });

  //Tell the server what to listen for to handle each feature.
  server.on("/start", HTTP_GET, handleStart);
  server.on("/stats", HTTP_GET, handleStats);
  server.on("/create", HTTP_GET, handleNewUser);
  server.on("/leader", HTTP_GET, handleLeaderboard);

  server.begin();

  //SD CARD INIT
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    // don't do anything more:
    return;
  }
}

void loop(void){
  //If Arduino is running drill then wait for results
  if(drillRunning){
    char buf = Serial.read();
    if(buf == '%'){
      drillRunning = false;
      writeResults();
    }
  }
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}

