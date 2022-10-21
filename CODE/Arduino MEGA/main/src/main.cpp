#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <PlotControl.h>

#define PWM 2
unsigned long tLoopStart;
unsigned int count = 1;
unsigned int LEDcount = 1;
int LEDhcount = 1;
bool LEDSTAT = true;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Moisture>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int AirValue = 615;
const int WaterValue = 280;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;
int sollsoilmoisturepercent = 80;
int pumptime;
int pumpfactor = 180;
int moisturecount;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Plotcontrol/Regelung>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PlotControl pc;
pltVal_t tIst, tSoll, pwm;
pltVal_t pWert, pAnteil;
pltVal_t iWert, iAnteil;
float rAbweichung;
uint8_t pwmInt = 0;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DHT11>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float tIstoutside;
float tIstinside;
float hIstoutside;
float hIstinside;

#define DHT1PIN 22 
#define DHT2PIN 24 

#define DHTTYPE DHT11

DHT dht(DHT1PIN, DHTTYPE);
DHT dht2(DHT2PIN, DHTTYPE);

//----------------setup-------------------
void setup() {
    
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DHT11>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  dht.begin();
  dht2.begin();

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Plotcontrol/Regelung>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  Serial.begin(115200);
  Serial3.begin(115200);
  Serial3.println("TEMPIN(°C)  TEMPOUT(°C)  HUMIN(%)  HUMOUT(%)  PWMFAN  MOIST(%)  TIME(msec)  PUMPTIME LEDSTAT");
  delay(50);
  
  pc.addVar(&tIst, "ist", 't');

  pc.addVar(&tSoll, "soll", 's');
  tSoll.val = 28; //---------Solltemperatur

  pc.addVar(&pwm, "pwm", 'w');
  pwm.skale = 5.0; 
  pc.setLimits(&pwm.valR, 0.0, 255.0); 

  pc.addVar(&pWert, "pWert", 'p');
  pWert.val = 0.05;
  pWert.skale = 2000.0;
  pc.addVar(&pAnteil, "pAnt", 'P');
  pAnteil.skale = pwm.skale;
  pc.addVar(&iWert, "iWert", 'i');
  iWert.val = 0.05;
  iWert.skale = 2000.0;
  pc.addVar(&iAnteil, "iAnt", 'I');
  iAnteil.val = 0.1;
  iAnteil.skale = pwm.skale;
  delay(50);
  pc.serPrintIds();
  delay(50);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Moisture>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  pinMode(48, OUTPUT);
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>LEDs>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  pinMode(13, OUTPUT);

}

//----------------regler Temp-------------------
void regler(){

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DHT11>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  tIstoutside = dht.readTemperature();
  tIstinside = dht2.readTemperature();
  tIst.val = tIstinside;
  hIstoutside = dht.readHumidity();
  hIstinside = dht2.readHumidity();

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Plotcontrol/Regelung>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  rAbweichung = tIst.val - tSoll.val;
  pAnteil.val = - pc.getParVal(&pWert) * rAbweichung;
  iAnteil.val -=pc.getParVal(&iWert) * rAbweichung;
  if (pc.parOff(&iWert)) iAnteil.val = 0.0;
  pwm.val = pAnteil.val - iAnteil.val;
  pc.valClip(&pwm);
  pwmInt = round(pwm.val);
  analogWrite(PWM, pwmInt);
}

//----------------regler Moisture-------------------
void moisture(){

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Moisture>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  soilMoistureValue = analogRead(A0);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilmoisturepercent >= 100) soilmoisturepercent = 100;
  else if(soilmoisturepercent <= 0) soilmoisturepercent = 0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Steuerung>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  if(tLoopStart >= 1800000*moisturecount){
    pumptime = (sollsoilmoisturepercent - soilmoisturepercent)* pumpfactor;
    if(pumptime < 0) pumptime = 0;
    digitalWrite(48, HIGH);
    delay(pumptime);
    digitalWrite(48, LOW);
    moisturecount++;
  }

}

//----------------LEDs-------------------
void LEDs(){
  if (tLoopStart >= 3600000*LEDcount){
    LEDcount++;
    LEDhcount++;
  }
  if (LEDhcount >= 25){
    LEDhcount = 1;
    Serial3.print("    ");
    Serial3.print("another day");
  }
  if (LEDhcount >= 19){
    digitalWrite(13, LOW);
    LEDSTAT = false;
  }
  else {
    digitalWrite(13, HIGH);
    LEDSTAT = true;

  }
}
//----------------loop-------------------
void loop() {
  tLoopStart = millis();
  Serial.print(tLoopStart);
  regler();
  pc.serPrintVals();
  pc.checkKeys();
  moisture();
  LEDs();

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Steuerung>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  if (tLoopStart >= 90000*count){
    count++;
    Serial3.print(tIstinside);
    Serial3.print("    ");
    Serial3.print(tIstoutside);
    Serial3.print("    ");
    Serial3.print(hIstinside);
    Serial3.print("    ");
    Serial3.print(hIstoutside);
    Serial3.print("    ");
    Serial3.print(pwmInt);
    Serial3.print("    ");
    Serial3.print(soilmoisturepercent);
    Serial3.print("    ");
    Serial3.print(tLoopStart);
    Serial3.print("    ");
    Serial3.print(pumptime);
    Serial3.print("    ");
    Serial3.print(LEDSTAT);
    Serial3.print("    ");
    Serial3.print('\r');
    delay(100);
  }
}