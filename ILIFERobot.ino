#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include "Structs.h"
#include <ArduinoJson.h>
#include <time.h>
#include "htmlindex.h"


const int sleepTime = 50; //ms
const int publishStatusTimer = 60000; //ms

const char* WiFi_SSID = "<ssid>"; // LAN
const char* WiFi_PW = "<pass>>";
const char* AP_SSID = "ilife_upgrade"; // AP and UDP clients
const char* AP_PW = "123456";

const char* devicename = "ilife-vacuum";
const char* mqtt_server = "<broker-ip>>";
const char* mqtt_client = devicename;
const char* mqtt_user = "<mqtt_username>";
const char* mqtt_pass = "<mqtt_pass>";
const char* willTopic = "ilife-vacuum/LWT";
const char* inTopic = "ilife-vacuum/command";
const char* stateTopic = "ilife-vacuum/state";
const char* statusTopic = "ilife-vacuum/status";
const char* fanTopic = "ilife-vacuum/fan_speed";
const char* outTopic_debug = "ilife-vacuum/debug";

const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

#define IRPin       4 //pin D2, pin that is used for sending the IR signals
#define statusPin1  14 // pin D5, input pin for robot status (from led signal)
#define statusPin2  5 // pin D1, input pin for robot status (from led signal)
#define statusPin3  12 // pin D6, input pin for robot status (from led signal)
#define dockPin     16 // pin D0, input pin for dock contact
#define batteryPin  A0

const IRbutton rStart = {"start", {8850,4500, 500,600, 500,600, 500,600, 500,600, 500,600, 500,600, 500,1700, 550,600, 500,1700, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,600, 500,600, 500,1750, 500,600, 500,600, 500,600, 500,1700, 500,600, 500,1750, 500,1700, 500,600, 500,1750, 500,1700, 500,1700, 500,600, 500,1750, 500}}; // NEC 2AA22DD
const IRbutton rUp =    {"up", {8850,4500, 500,600, 500,600, 500,600, 500,600, 500,600, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1700, 550,550, 550,1700, 500,1700, 550,600, 500,1700, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500}};  // NEC 2AA55AA
const IRbutton rRight = {"right", {8900,4450, 500,600, 500,600, 500,600, 500,600, 500,600, 550,550, 550,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,600, 500,1700, 550,550, 550,600, 500,600, 500,1700, 500,600, 500,600, 500,1750, 500,600, 500,1700, 500,1700, 500,1750, 500,600, 500,1700, 500,1750, 500}};  // NEC 2AA44BB
const IRbutton rDown =  {"down", {8900,4450, 500,600, 550,550, 550,600, 500,600, 500,600, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1700, 550,600, 500,1700, 500,600, 500,600, 500,1750, 500,1700, 500,600, 500,600, 500,1700, 500,1750, 500,600, 500,1750, 500,600, 500,600, 500,1700, 500,1700, 500,600, 500,600, 500,1750, 500}};  // NEC 2AA6699
const IRbutton rLeft =  {"left", {8900,4500, 500,600, 500,600, 500,600, 500,600, 500,600, 500,600, 500,1700, 550,550, 550,1700, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,600, 550,550, 500,1750, 500,1700, 500,600, 500,600, 500,1750, 500,1700, 550,1700, 500,1700, 550,550, 500,600, 500,1750, 500,1700, 500,600, 500,600, 500}};  // NEC 2AA33CC
const IRbutton rSpot =  {"spot", {8900,4450, 550,550, 550,600, 500,600, 500,600, 500,600, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,600, 500,1750, 500,1700, 500,1700, 550,600, 500,1700, 500,1700, 500,1750, 500,1700, 500,600, 550,550, 500,600, 550,1700, 500,600, 500,600, 500,600, 500}};  // NEC 2AA7788
const IRbutton rHome =  {"home", {8900,4450, 500,600, 500,600, 500,600, 500,600, 500,600, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,600, 550,550, 550,1700, 500,600, 500,600, 500,600, 500,600, 500,1750, 500,1700, 500,1700, 550,550, 550,1700, 500,1700, 500,1750, 500}};  // NEC 2AA8877
const IRbutton rEdge =  {"edge", {8900,4500, 500,600, 500,600, 500,600, 500,600, 500,600, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 500,600, 500,1750, 500,600, 500,1700, 550,550, 500,1750, 500,600, 500,600, 500,1700, 500,1700, 500,600, 550,600, 500,1700, 550,550, 500,1750, 500,1700, 500,600, 500,600, 500,1750, 500,1700, 500,600, 500}};  // NEC 2AA9966
const IRbutton rReboot =  {"reboot", {}};  // NEC 2AA9966

const IRbutton buttonCmds[] = {rStart, rUp, rRight, rDown, rLeft, rSpot, rHome, rEdge, rReboot};

const IRbutton_long rLocate = {"locate", {8900,4450, 500,600, 500,600, 500,600, 500,600, 500,600, 500,600, 500,1750, 500,600, 500,600, 500,1700, 550,600, 500,1700, 500,600, 550,1650, 500,600, 500,1750, 500,600, 500,600, 500,1700, 550,1700, 500,600, 500,600, 500,600, 500,600, 500,1750, 500,1700, 500,600, 500,600, 500,1700, 550,1700, 500,1700, 500,1750, 500,1700, 550,550, 550,550, 550,550, 550,550, 550,550, 550,550, 550,600, 500,600, 500,600, 500,1700, 500,1750, 500,600, 500,1700, 500,1700, 550,1700, 500}};  // NEC 25530CF

WiFiClient espClient;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
PubSubClient mqtt(espClient);
// WiFiUDP UDP;

Status robotStatus = S_BOOTING;
bool fanPower = false; //0=normal, 1=max
unsigned long lastStatusUpdate = 0;
unsigned long lastStatusPinUpdate = 0;
unsigned int stuckCount = 0;

PinTime led1;
PinTime led2;
PinTime led3;
Battery bat;

time_t boot_time; //stores boot time

byte activeSockets, retryCounter, retries = 20;
boolean WiFiUp = false; // Wifi flag
unsigned long connectedMillis;
// const unsigned int localPort = 8888;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  delay(10);
  
  pinMode(IRPin, OUTPUT);
  digitalWrite(IRPin, HIGH);
  pinMode(statusPin1, INPUT);
  pinMode(statusPin2, INPUT);
  pinMode(statusPin3, INPUT);
  pinMode(dockPin, INPUT);
  
  setupWifi();

  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);
  
  setupHTTP();

  initPinTimer(led1, statusPin1, 20);
  initPinTimer(led2, statusPin2, 20);
  initPinTimer(led3, statusPin3, 20);

  //get boot datetime
  setupTime();
  boot_time = getCurrentTime();

}
 
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  
  if (WiFiUp && !activeSockets && WiFi.status() != WL_CONNECTED && retryCounter < retries && millis() - connectedMillis >= 60000UL * sq(retryCounter)) reconnectWifi();
  
  server.handleClient();
  mqtt.loop();
  checkLedStatus();
  delay(sleepTime);
}



/* 
 *  Functions 
*/
void setupWifi() {
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(WiFi_SSID);
  
  WiFi.setAutoConnect(false);
  WiFi.hostname(devicename);
  Serial.printf("Scanning for %s\r\n", WiFi_SSID); // if WiFi/LAN is available
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == WiFi_SSID) {
      WiFiUp = true;
      WiFi.mode(WIFI_AP_STA); // LAN and AP and UDP clients
      //WiFi.config(ip, gateway, subnet); // LAN fixed IP
      WiFi.begin(WiFi_SSID, WiFi_PW); // connect to LAN with credentials
      Serial.printf("Found %s, trying to connect ", WiFi_SSID);
      break;
    }
    delay(10000);
  }
  connectWiFi();
  
}
void connectWiFi() {
  if (WiFiUp) {
    byte w8 = 0;
    while (WiFi.status() != WL_CONNECTED && w8++ < 15) {
      delay(5000); // try for 5 seconds
      Serial.print(">");
    }
    Serial.printf("\r\n");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n\tConnected to %s IP address %s strength %d%%, RSSI %d \r\n", WiFi_SSID, WiFi.localIP().toString().c_str(), WifiGetRssiAsQuality(WiFi.RSSI()), WiFi.RSSI() );
    WiFi.setAutoReconnect(true);
    retryCounter = 0; // reset counter when connected
  } else {
    delay(10000);
   WiFi.mode(WIFI_AP); // drop station mode if LAN/WiFi is down
   WiFi.softAP(AP_SSID, AP_PW);
   Serial.printf("\tLAN Connection failed\r\n\tTry %s AP with IP address %s\r\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  }
//  if (MDNS.begin(mDNSname)) Serial.printf("mDNS responder started\r\n\tName: %s.local\r\n", mDNSname);
//  else Serial.println("*** Error setting up mDNS responder\r\n");
//  if (UDP.begin(localPort)) Serial.printf("Broadcasting UDP on %s AP with IP address %s port %d\r\n", AP_SSID, WiFi.softAPIP().toString().c_str(), localPort);
//  else Serial.println("*** Error setting up UDP\r\n");
}

void reconnectWifi() {
  connectedMillis = millis(); // update
  retryCounter ++; // update connection retries
  WiFi.mode(WIFI_AP_STA); // LAN and AP and UDP clients
  WiFi.begin(); // connect to LAN
  Serial.printf("Trying to reconnect to %s, attempt %d \n", WiFi_SSID, retryCounter);
  connectWiFi();
}

int WifiGetRssiAsQuality(int rssi){
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return quality;
}

int findValidRobotCmd(const char* cmd) {
  for(int c = 0; c < sizeof(buttonCmds)/sizeof(buttonCmds[0]); c++) {
    if(strcmp(cmd, buttonCmds[c].name) == 0) {
      return c;
      break;
    }
  }
  return -1;
}

String printpulses(IRbutton irbutton, boolean printSerial)
{
  // print it in an 'array' format
  int arraySize = sizeof(irbutton.signal) / 4;
  String out = "int ";
  out += irbutton.name;
  out += "[] = {\n";
  out += "// ON, OFF\n";
  int i;
  for (i = 0; i < arraySize/2; i++)
  {
    out += irbutton.signal[i * 2];
    out += ", ";
    out += irbutton.signal[i * 2 + 1];
    out += ",\n";
  }
  if(arraySize % 2 != 0) {
    out += irbutton.signal[i * 2];
    out += "\n";
  }
  out += "};";
  
  if(printSerial)
    Serial.println(out);
    
  return out;
}


void SendIRCode(IRbutton irbutton)
{
  int arraySize = sizeof(irbutton.signal) / 4; //sizeof(irbutton.signal) / sizeof(unsigned int)
  int i;
  //printpulses(irbutton, true);
  
  noInterrupts();  // this turns off any background interrupts
  for (i = 0; i < arraySize/2; i++) {
    digitalWrite(IRPin, LOW);  // this takes about 3 microseconds to happen
    delayMicroseconds(irbutton.signal[i * 2] - 3);
    digitalWrite(IRPin, HIGH);   // this also takes about 3 microseconds
    delayMicroseconds(irbutton.signal[i * 2 + 1] - 3);
  }
  if(arraySize % 2 != 0) {
    digitalWrite(IRPin, LOW);
    delayMicroseconds(irbutton.signal[i * 2] - 3);
    digitalWrite(IRPin, HIGH);
  }
  interrupts();
  
  //mqtt feedback
  char command[20];
  sprintf(command, "IR %s", irbutton.name);
  mqtt.publish(outTopic_debug, command);
}

void SendIRCode_long(IRbutton_long irbutton)
{
  int arraySize = sizeof(irbutton.signal) / 4; //sizeof(irbutton.signal) / sizeof(unsigned int)
  int i;
  //printpulses(irbutton, true);
  
  noInterrupts();  // this turns off any background interrupts
  for (i = 0; i < arraySize/2; i++) {
    digitalWrite(IRPin, LOW);  // this takes about 3 microseconds to happen
    delayMicroseconds(irbutton.signal[i * 2] - 3);
    digitalWrite(IRPin, HIGH);   // this also takes about 3 microseconds
    delayMicroseconds(irbutton.signal[i * 2 + 1] - 3);
  }
  if(arraySize % 2 != 0) {
    digitalWrite(IRPin, LOW);
    delayMicroseconds(irbutton.signal[i * 2] - 3);
    digitalWrite(IRPin, HIGH);
  }
  interrupts();
  
  //mqtt feedback
  char command[20];
  sprintf(command, "IR %s", irbutton.name);
  mqtt.publish(outTopic_debug, command);
}


void checkLedStatus() {
  unsigned long now = millis();
  calcPinTime(led1, 2000);
  calcPinTime(led2, 2000);
  calcPinTime(led3, 2000);

  
  //give some time after startup before calculating state
  if(now > 20000) { 
//    if(led1.statusChanged || led2.statusChanged || led3.statusChanged) {
//      if(calculateStatus()) { //if pin status has changed, publish
//        publishStatus();
//        publishDebugStatus();
//      }
//    }
    
    if(now - lastStatusPinUpdate > 200) {
      lastStatusPinUpdate = now;
      
      if(calculateStatus()) { //if pin status has changed, publish
        publishState();
        publishStatus();
        lastStatusUpdate = now;
      }
 //     publishDebugStatus();
    }
  }

  
  if(now - lastStatusUpdate > publishStatusTimer) {
    lastStatusUpdate = now;
    calculateStatus();
    publishState();
    publishStatus();
    publishFanStatus();
    //publishDebugStatus();
  }
}

boolean calculateStatus() {
  Status newStatus;
  
  if((led1.ratio > 0.4 and led1.ratio < 0.55) and (led2.ratio > 0.4 and led2.ratio < 0.55) and (led3.ratio > 0.4 and led3.ratio < 0.55)) {
    stuckCount++;
  }
  //led is low when it is on
  if(isDocked())
    newStatus = S_DOCKED;
  else if(led1.isHigh and led2.isHigh and led3.isHigh)
    newStatus = S_SLEEP;
  else if(led1.isHigh and led2.isLow and led3.isBlinking || led1.isBlinking and led2.isLow and led3.isHigh || led1.isHigh and led2.isLow and led3.isHigh )
    newStatus = S_BUSY;
  else if(led1.isHigh and led2.isBlinking and led3.isHigh)
    newStatus = S_GOING_HOME;
   else if(stuckCount > 15)
    newStatus = S_STUCK;
  else
    newStatus = S_IDLE;

  if(robotStatus != newStatus) {
    robotStatus = newStatus;
    
    if(newStatus != S_STUCK)
      stuckCount = 0;
      
    return true;
  }
  else
    return false;
}

void publishState() {
  const char* stateName;
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& jst = jsonBuffer.createObject();
  
  switch(robotStatus) {
    case S_BOOTING: stateName = "booted"; break;
    case S_SLEEP: stateName = "sleep"; break;
    case S_IDLE: stateName = "idle"; break;
    case S_BUSY: stateName = "cleaning"; break;
    case S_STUCK: stateName = "error"; break;
    case S_DOCKED: stateName = "docked"; break;
    case S_GOING_HOME: stateName = "returning"; break;
  }
 
  jst["state"] = stateName;
  jst["battery_level"] = (int)round(calcBattery(true));
  
  char msg[100];
  jst.printTo(msg);
  mqtt.publish(stateTopic, msg, true);
}

void publishStatus() {
  const char* statusName;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& jst = jsonBuffer.createObject();
  
  switch(robotStatus) {
    case S_BOOTING: statusName = "booted"; break;
    case S_SLEEP: statusName = "sleep"; break;
    case S_IDLE: statusName = "idle"; break;
    case S_BUSY: statusName = "busy"; break;
    case S_STUCK: statusName = "stuck"; break;
    case S_DOCKED: statusName = "docked"; break;
    case S_GOING_HOME: statusName = "goinghome"; break;
  }
 
  jst["Status"] = statusName;
  jst["Charging"] = (boolean)isCharging() == true;
  jst["Docked"] = (boolean)isDocked() == true;
  jst["Fan_Speed"] = fanPower;
  jst["Battery_Level"] = (int)round(calcBattery(true));
  jst["Battery_Voltage"] = (float)round(100*calcBattery(false))/100;
  jst["Uptime"] = millis();
  jst["Boottime"] = boot_time;
  jst["Signal"] = WifiGetRssiAsQuality(WiFi.RSSI());
  jst["RSSI"] = WiFi.RSSI();
  
  char msg[200];
  jst.printTo(msg);
  mqtt.publish(statusTopic, msg);
}

void publishFanStatus() {
  char msg[10];
  if (fanPower == true) {
  snprintf (msg, 10, "max");
  mqtt.publish(fanTopic, msg);
  }
  else {
  snprintf (msg, 10, "normal");
  mqtt.publish(fanTopic, msg);
  }
}

//void publishDebugStatus() {
//  char msg[150];
//  char r1_temp[6];
//  char r2_temp[6];
//  char r3_temp[6];
//  dtostrf(led1.ratio, 4, 2, r1_temp);
//  dtostrf(led2.ratio, 4, 2, r2_temp);
//  dtostrf(led3.ratio, 4, 2, r3_temp);
//  snprintf (msg, 150, "Status now=%1d, pin1: %s, pin2: %s, pin3: %s - %1d %1d %1d - %1d %1d %1d, A0 %d", millis(),
//    r1_temp,
//    r2_temp,
//    r3_temp,
//    led1.lastValue, led2.lastValue, led3.lastValue,
//    digitalRead(statusPin1), digitalRead(statusPin2), digitalRead(statusPin3), analogRead(A0));
//  mqtt.publish(outTopic_debug, msg);
//}


boolean doAction(const char* action) {
  if(strcmp(action, "reboot") == 0) {
    Serial.println("Restarting ESP...");
    ESP.restart();
    return true;
  }   
  else if(strcmp(action, "start_pause") == 0) {
    SendIRCode(rStart);
    return true;
  }
  else if(strcmp(action, "start") == 0) {
    Serial.println("start");
    if(robotStatus == S_IDLE || robotStatus == S_GOING_HOME || robotStatus == S_DOCKED ) {
      SendIRCode(rStart);
    }
    else if(robotStatus == S_SLEEP || robotStatus == S_DOCKED and !isCharging() ) {
      SendIRCode(rStart); //wake up first
      delay(1000); //wait a bit
      SendIRCode(rStart); //now it will start
    }
    return true;
  }
  else if(strcmp(action, "stop") == 0) {
    if(robotStatus == S_BUSY || robotStatus == S_GOING_HOME )
      SendIRCode(rStart);
    return true;
  }
  else if(strcmp(action, "down") == 0) {
    //set fan power (0 = normal,1 = max)
    if(robotStatus == S_BUSY)      
    SendIRCode(rDown);
    fanPower = !fanPower;
    publishFanStatus();
    publishState();
    return true;
  }
  else if(strcmp(action, "home") == 0) {
    if(robotStatus == S_BUSY || robotStatus == S_IDLE )
      SendIRCode(rHome);
    else if(robotStatus == S_SLEEP and !isDocked()) {
      SendIRCode(rStart); //wake up first
      delay(1000); //wait a bit
      SendIRCode(rHome);
    }
    return true;
  }
  else if(strcmp(action, "locate") == 0) {
    SendIRCode(rStart); //wake up first
    delay(1000); //wait a bit
      for (int i = 0; i < 10; i++) {// Loop to beep n times
         SendIRCode_long(rLocate);
         delay(500);
         }
      if(robotStatus == S_BUSY)
      SendIRCode(rStart); //wake up first
      else {
           delay(500);
        }
    return true;
  } 
  else {
    int buttonCmd = findValidRobotCmd(action);
    if(buttonCmd != -1) {
      SendIRCode(buttonCmds[buttonCmd]);
      return true;
    }
  }
  return false;
};

float calcBattery(boolean returnPercentage) {
  float maxVolt = 12.41; //450 / 11
  float minVolt = 10.50;
  
  int value = analogRead(A0);
  float voltage = (float)value * (12.22/933); // 11 * 3.3 / 1024;
  
  bat.voltageBuffer[bat.valn] = voltage; //add reading to buffer
  bat.valn++;
  if(bat.valn == bat.bufferSize)
    bat.valn = 0;

  //calculate average
  float sum = 0;
  size_t bufferSize = 0;
  for(size_t i = 0; i < bat.bufferSize; i++) {
    sum += bat.voltageBuffer[i];
    if(bat.voltageBuffer[i] != 0)
      bufferSize++;
  }
  float meanVoltage = sum/bufferSize;

  //calculate percentage
  float percentage = (meanVoltage-minVolt)/(maxVolt-minVolt)*100;
  if(meanVoltage < minVolt)
    percentage = 0;
  if(meanVoltage > maxVolt)
    percentage = 100;
    
  if(returnPercentage)
    return percentage;
  else
    return meanVoltage;
}

boolean isDocked() {
  return (digitalRead(dockPin) == HIGH);  
}
boolean isCharging() {
  if(isDocked() and led1.isLow and (led2.isBlinking or led3.isBlinking))
  return true;
}