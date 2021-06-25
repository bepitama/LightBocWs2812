//Created by Giuseppe Tamanini 25/06/2021 with license cc-by-sa 4.0

#include <TM1637.h>
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN 14 // pin LEDs WS2812
#define NUMPIXELS 83 // Number of LEDs WS2812

RTC_DS1307 rtc;

// Instantiation and pins configurations
// Pin 2 - > DIO
// Pin 0 - > CLK
TM1637 tm1637(0, 2);
boolean dp;
int hh;
int mm;
int ss;
int oldss;
String Sore;
char ore[5];
int buttonPin = 12;
unsigned long myTime;
unsigned long myTime1;
unsigned long previousMillis;
boolean valbutton;
boolean oldvalbutton;
boolean cambiato;
boolean modifica = false;
int sezioni[10][2] = {{1, 10},{11, 18},{19, 23},{24,25},{26,28},{29,36},{37,46},{47,58},{59,68},{69,82}};
String scene[4] = {{"alba"},{"giorno"},{"tramonto"},{"notte"}};
int scena;
int oldscena;
int R;
int G;
int B;
int sezione;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  WiFi.mode( WIFI_OFF );
  tm1637.init();
  tm1637.setBrightness(5);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2021, 6, 2, 0, 18, 0));
  }
  //rtc.adjust(DateTime(2021, 6, 9, 20, 14, 30));
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)  
  pixels.clear(); // Set all pixel colors to 'off'  previousMillis = millis();
  alba();
}

void loop() {
  DateTime now = rtc.now();
  hh = now.hour();
  mm = now.minute();
  ss = now.second();
  if (ss == oldss) {
    if (millis() - previousMillis < 500 && dp == false) {
      sprintf(ore, "%02d%02d", hh, mm);
      Sore = ore;
      tm1637.display(Sore); // Display the time
      dp = true;
    }
    if (millis() - previousMillis > 500 && dp == true) {
      sprintf(ore, "%02d.%02d", hh, mm);
      Sore = ore;
      tm1637.display(Sore);
      dp = false;
    }
  } else {
    oldss = ss;
    previousMillis = millis(); 
  }
  valbutton = digitalRead(buttonPin);
  if (valbutton == false && !oldvalbutton) {
    myTime = millis();
    oldvalbutton = true; 
  } else if (valbutton && oldvalbutton) {
    oldvalbutton = false;  // pone il vecchio stato del pulsante a falso
    cambiato = false; // lo stato dell'azione del pulsante1 è falsa 
  }
  if (valbutton == false && oldvalbutton && cambiato == false && millis() - myTime > 100) {
    oldvalbutton = false;
    scena = scena + 1;
    if (scena == 4) scena = 0;
    if (scena != oldscena) {
      oldscena = scena;
      switch (scena) {
        case 0:
          alba();
          break;
        case 1:
          giorno();
          break;
        case 2:
          tramonto();
          break;
        case 3:
          notte();
          break;
      }
    }
  }
  if (valbutton == false && oldvalbutton && cambiato == false && millis() - myTime > 3000) {
    oldvalbutton = false;
    cambiato = false;
    modifica = true;
    myTime1 = millis();
    modificaora(); // esegue la funzione modificadata
  }
}

void modificaora() {
  while (modifica) {
    sprintf(ore, "%02d.%02d", hh, mm);
    Sore = ore;
    tm1637.display(Sore); // Display the time
    valbutton = digitalRead(buttonPin);
    if (valbutton == false && !oldvalbutton) {
      myTime = millis();
      oldvalbutton = true;
      cambiato = true;
    } else if (valbutton && oldvalbutton) {
      oldvalbutton = false;  // pone il vecchio stato del pulsante a falso
      cambiato = false;
      myTime1 = millis();
    }
    if (valbutton == false && oldvalbutton && millis() - myTime > 100) {
      Serial.println(valbutton);
      myTime = millis();
      oldvalbutton = true;
      mm = mm + 1;
      if (mm == 60) {
        mm = 0;
        hh = hh + 1;
        if (hh == 24) hh = 0;
      }
    }
    delay(0);
    if (valbutton && oldvalbutton == false && cambiato == false && millis() - myTime1 > 3000) {
      modifica = false;
      rtc.adjust(DateTime(2021, 1, 1, hh, mm, ss));
    }
  }
}

void colora(int sezione, int R, int G, int B) {
  for (int i = sezioni[sezione][0] - 1; i < sezioni[sezione][1]; i++) {
    pixels.setPixelColor(i, pixels.Color(R, G, B));
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
}


void alba() {
  colora(0, 0, 0,0);
  colora(1, 0, 0, 0);
  colora(2, 64, 32, 16);
  colora(3, 0, 0, 0);
  colora(4, 0, 0, 0);
  colora(5, 0, 0, 0);
  colora(6, 0, 0, 0);
  colora(7, 127, 64, 32);
  colora(8, 127, 64, 32);
  colora(9, 170, 255, 0);
}

void giorno() {
  colora(0, 0, 255,0);
  colora(1, 128, 192, 0);
  colora(2, 0, 128, 128);
  colora(3, 192, 192, 128);
  colora(4, 0, 0, 0);
  colora(5, 64, 64, 0);
  colora(6, 0, 128, 128);
  colora(7, 0, 128, 127);
  colora(8, 0, 128, 128);
  colora(9, 170, 255, 0);
}

void tramonto() {
  colora(0, 0, 255,0);
  colora(1, 127, 192, 0);
  colora(2, 0, 255, 255);
  colora(3, 192, 192, 0);
  colora(4, 0, 0, 0);
  colora(5, 64, 64, 0);
  colora(6, 127, 0, 128);
  colora(7, 0, 0, 255);
  colora(8, 0, 0, 255);
  colora(9, 255, 0, 0);
}

void notte() {
  colora(0, 0, 0,128);
  colora(1, 0, 0, 0);
  colora(2, 0, 64, 64);
  colora(3, 0, 0, 0);
  colora(4, 64, 96, 0);
  colora(5, 0, 0, 0);
  colora(6, 0, 0, 0);
  colora(7, 0, 0, 128);
  colora(8, 0, 0, 128);
  colora(9, 128, 192, 0);
}
