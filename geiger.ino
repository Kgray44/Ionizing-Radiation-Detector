/*************************************************************
File Name: geiger.ino
Processor/Platform: Leonardo (CM-23U4 tested)
Development Environment: Arduino 1.8.19
Download latest code here:
https://github.com/Kgray44/Ionizing-Radiation-Detector
Geiger code meant to be used alongside the tutorial found here:
https://www.hackster.io/k-gray/ionizing-radiation-detector-a0a782

Copyright 2022 K Gray
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
Liscense found here:
https://opensource.org/licenses/MIT
 *************************************************************/


#include <TinyGPS++.h>
#include <SD.h>
#include <SPI.h>
#include <DFRobot_RGBLCD1602.h>
#include <SoftwareSerial.h>
#include <DFRobot_Geiger.h>
#include <EEPROM.h>

#define geigerPin 0//D0
#define vibration 11//D11
#define SDss A0
#define button A1

#define startuSvh 1
#define dangeruSvh 20

#define arrSize(X) sizeof(X) / sizeof(X[0])

int values[] =             {    1,         2,        5,          10,         40,      100,       250,          400,          1000,      10000   };
const char* equivalent[] = { "Normal", "Airport", "Dental", "Norm(1day)", "Flight", "X-Ray", "NPP(1year)", "Mammogram",  "Gov Limit", "CT Scan" };

const byte hazardChar[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b11001,
  0b10101,
  0b10011,
  0b01110,
  0b00000
};

const byte motor1Char[8] = {
  0b00110,
  0b00110,
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b01110
};

const byte motor2Char[8] = {
  0b01100,
  0b01100,
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b01110
};

const byte fullChar[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

long savegeigertime = 2000;//5 sec  //120000;//2 min

static const int RXPin = 9, TXPin = 1;//D1/D9
static const uint32_t GPSBaud = 9600;

int timer1 = 0;
int lastMillis = 0;
int timer3 = 0;
int mode = 1;
boolean vib = true;
int count = 0;

int CPM = 0;
int uSvh = 0;
int nSvh = 0;

File myFile;
char sfileName[] = "geiger.txt";
char wfileName[] = "warnings.txt";


DFRobot_Geiger geiger(geigerPin);
DFRobot_RGBLCD1602 lcd(/*lcdCols*/16,/*lcdRows*/2);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  pinMode(button, INPUT);
  pinMode(SDss, OUTPUT);
  pinMode(vibration, OUTPUT);
  digitalWrite(vibration, LOW);
  ss.begin(GPSBaud);

  geiger.start();
  
  lcd.init();
  lcd.customSymbol(1,motor1Char);
  lcd.customSymbol(2,motor2Char);
  lcd.customSymbol(3,hazardChar);
  lcd.customSymbol(4,fullChar);
  lcd.setCursor(4,0);
  lcd.print(F("Geiger"));
  lcd.setCursor(3,1);
  lcd.print(F("Counter"));
  lcd.setRGB(0,255,0);
  delay(3000);
  for (int i=0;i<15;i++){
    lcd.scrollDisplayRight();
    delay(100);
  }

  if (!SD.begin(SDss)) {
    lcd.setRGB(255, 0, 0);
    lcd.setCursor(1,0);
    lcd.print(F("SD Init Failed"));
    delay(5000);
  }
  
  lcd.clear();

}

void loop() {  
  //geiger.pause();
  CPM = geiger.getCPM();
  uSvh = geiger.getuSvh();
  nSvh = geiger.getnSvh();
  Serial.print("CPM: ");
  Serial.println(CPM);
  //geiger.start();

  while (ss.available() > 0){
    gps.encode(ss.read());
  }
  
  if (digitalRead(button) == HIGH){
    /*do {
      //redo:
      timer1++;
      delay(1);
    } while (digitalRead(button) == LOW);*/
    redo:
    timer1++;
    delay(1);
    if (digitalRead(button) == HIGH){goto redo;}
    
    lcd.clear();
    
    switch (timer1) {
    case 0 ... 1499 :
      mode = 1;
      lcd.setRGB(0, 255, 0);
      lcd.setCursor(0,0);
      lcd.print(F("Standard Mode"));
      delay(3000);
      break;
    case 1500 ... 2499 :
      mode = 2;
      lcd.setRGB(0, 255, 255);
      lcd.setCursor(3,0);
      lcd.print(F("Bar Mode"));
      delay(3000);
      break;
    case 2500 ... 3499 :
      mode = 3;
      lcd.setRGB(0, 0, 255);
      lcd.setCursor(0,0);
      lcd.print(F("Equivalents Mode"));
      delay(3000);
      break;
    case 3500 ... 4499 :
      vib = !vib;
      lcd.setRGB(255, 0, 255);
      lcd.setCursor(3,0);
      lcd.print(F("Vibration"));
      lcd.setCursor(4,1);
      lcd.print(vib > 0 ? "On" : "Off");
      for (int i=0;i<10;i++){
        lcd.setCursor(14,1);
        lcd.write(1);
        delay(300);
        lcd.setCursor(14,1);
        lcd.write(2);
        delay(300);
      }
      break;
    default :
      lcd.setRGB(255, 150, 100);
      lcd.setCursor(3,0);
      lcd.print(F("Incorrect"));
      lcd.setCursor(1,1);
      lcd.print(F("1,2,3,or4 sec"));
      delay(3000);
      break;
    }
    timer1=0;
    if (mode == 1 || mode == 2 || mode == 3){
      EEPROM.write(0, mode);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sucess!");
      delay(800);
    }
    lcd.clear();
  }

  lcdDisplayData();
  
  if (uSvh > startuSvh && uSvh < (dangeruSvh - 1)){
    //lcd.setRGB(255, 172, 28);
    lcd.setCursor(14,1);
    lcd.write(3);
    if (vib){
      //analogWrite(vibration, 128);
      digitalWrite(vibration, HIGH);
      delay(300);
      digitalWrite(vibration, LOW);
    }
    else {
      delay(300);
    }
    lcd.clear();
    saveToFile(2);
  }
  else if (uSvh >= dangeruSvh){
    //lcd.setRGB(255, 0, 0);
    lcd.setCursor(14,1);
    lcd.write(3);
    if (vib){
      //analogWrite(vibration, 255);
      digitalWrite(vibration, HIGH);
      delay(700);
      digitalWrite(vibration, LOW);
    }
    else {
      delay(700);
    }
    saveToFile(1);
  }
  
  if ((millis() - lastMillis) >= savegeigertime && uSvh > 0){
    count++;
    saveToFile(3);
    lastMillis = millis();
  }
  /*timer3++;
  if (timer3 >= 800){
    lcd.clear();
    lcdDisplayData(uSvh);
    timer3 = 0;
  }*/
  delay(3000);
}

int nearestEqual(int x, bool sorted = true) {
  int idx = 0; // by default near first element
  int distance = abs(values[idx] - x);
  for (int i = 1; i < arrSize(values); i++) {
    int d = abs(values[i] - x);
    if (d < distance) {
      idx = i;
      distance = d;
    }
    else if (sorted) return idx;
  }
  return idx;
}

void lcdDisplayData(){
  if (mode == 1){
    lcd.setCursor(0,0);
    lcd.print(F("Geiger Val(CPM):"));
    lcd.setCursor(4,1);
    lcd.print(CPM);
  }
  else if (mode == 2){
    lcd.setCursor(0,0);
    lcd.print(F("Geiger:"));
    lcd.setCursor(9,0);
    lcd.print(CPM);
    lcd.setCursor(0,1);
    lcdBar(CPM);
  }
  else if (mode == 3){
    lcd.setCursor(0,0);
    lcd.print(F("Geiger:"));
    lcd.setCursor(9,0);
    lcd.print(CPM);
    lcd.setCursor(0,1);
    if (CPM == 0){
      lcd.print("Normal");
    }
    else {
      lcd.print(equivalent[nearestEqual(CPM)]);
      delay(500);
    }
  }
  lcdbacklight(CPM);
}

void lcdBar(int re){
  for (int i=0;i<(nearestEqual(re)+1);i++){
    lcd.setCursor(i+1,1);
    lcd.write(4);
  }
}

void lcdbacklight(int re){
  int theNearestEqual = nearestEqual(re);
  switch (theNearestEqual) {
  case 1 :
    lcd.setRGB(0,255,0);
    break;
  case 2 :
    lcd.setRGB(0,255,0);
    break;
  case 3 :
    lcd.setRGB(150,255,0);
    break;
  case 4 :
    lcd.setRGB(255,255,0);
    break;
  case 5 :
    lcd.setRGB(255,180,0);
    break;
  case 6 :
    lcd.setRGB(255,100,0);
    break;
  case 7 :
    lcd.setRGB(255,0,0);
    break;
  case 8 :
    lcd.setRGB(255,0,150);
    break;
  case 9 :
    lcd.setRGB(175,0,255);
    break;
  default :
    break;
  }
  if (re == 0){
    lcd.setRGB(0,255,0);
  }
}

void saveToFile(int warningType){
  if (warningType == 3){
    myFile = SD.open(sfileName, FILE_WRITE);
    if (myFile){
      myFile.print(F("Time: "));
      myFile.print(gps.time.hour());
      myFile.print(F(":"));
      myFile.print(gps.time.minute());
      myFile.print(F(":"));
      myFile.println(gps.time.second());
      myFile.print(F("Date: "));
      myFile.print(gps.date.month());
      myFile.print(F("/"));
      myFile.print(gps.date.day());
      myFile.print(F("/"));
      myFile.println(gps.date.year());
      myFile.print(F("Count: "));
      myFile.println(count);
      myFile.print(F("Millis: "));
      myFile.println(millis());
      myFile.print(F("Latitude: "));
      myFile.println(gps.location.lat(),6);
      myFile.print(F("Longitude: "));
      myFile.println(gps.location.lng(),6);
      myFile.print(F("Satellites: "));
      myFile.println(gps.satellites.value());
      myFile.print(F("Altitude: "));
      myFile.println(gps.altitude.feet());
      myFile.print(F("Course: "));
      myFile.println(gps.course.deg());
      myFile.print(F("Speed: "));
      myFile.println(gps.speed.mph(),2);
      myFile.print(F("Geiger (CPM): "));
      myFile.println(CPM);
      myFile.print(F("Geiger (uSvh): "));
      myFile.println(uSvh);
      myFile.print(F("Geiger (nSvh): "));
      myFile.println(nSvh);
      myFile.println(F("----------------------------------------------------------"));
      myFile.println("");
      myFile.close();
    }
  }
  else {
    myFile = SD.open(wfileName, FILE_WRITE);
    if (myFile){
      myFile.println(F("---------Warning---------"));
      if (warningType == 1){
        myFile.println(F("-------Type=Severe-------"));
      }
      else {
        myFile.println(F("------Type=Moderate------"));
      }
      myFile.print(F("Time: "));
      myFile.print(gps.time.hour());
      myFile.print(F(":"));
      myFile.print(gps.time.minute());
      myFile.print(F(":"));
      myFile.println(gps.time.second());
      myFile.print(F("Date: "));
      myFile.print(gps.date.month());
      myFile.print(F("/"));
      myFile.print(gps.date.day());
      myFile.print(F("/"));
      myFile.println(gps.date.year());
      myFile.print(F("Millis: "));
      myFile.println(millis());
      myFile.print(F("Geiger (CPM): "));
      myFile.println(CPM);
      myFile.print(F("Geiger (uSvh): "));
      myFile.println(uSvh);
      myFile.print(F("Geiger (nSvh): "));
      myFile.println(nSvh);
      myFile.println(F("----------------------------------------------------------"));
      myFile.println("");
      myFile.close(); 
    }
  }
}
