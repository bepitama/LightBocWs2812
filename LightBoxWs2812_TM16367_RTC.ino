//Created by Giuseppe Tamanini 30/06/2021 with license cc-by-sa 4.0

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
int hh; // hour
int mm;  // minute
int ss;  // second
int oldss; // old second
String Sore;
char ore[5];
int buttonPin = 12; // button pin
unsigned long myTime; // button reading variable
unsigned long myTime1;
unsigned long previousMillis;
boolean valbutton;
boolean oldvalbutton;
boolean cambiato;
boolean modifica = false; // true if the program is in time edit mode
int sezioni[10][2] = {{1, 10},{11, 18},{19, 23},{24,25},{26,28},{29,36},{37,46},{47,58},{59,68},{69,82}}; // 83 LED divided in blocks
byte scene[4][10][3] = {
                        {{0,   0,  0},{  0,   0, 0},{64,  32,  16},{  0,   0,   0},{ 0,  0, 0},{ 0,  0, 0},{  0,   0,   0},{127,  64,  32},{127,  64,  32},{170, 255, 0}},
                        {{0, 255,  0},{128, 192, 0},{ 0, 128, 128},{192, 192, 128},{ 0,  0, 0},{64, 64, 0},{  0, 128, 128},{  0, 128, 127},{  0, 128, 128},{170, 255, 0}},
                        {{0, 255,  0},{127, 192, 0},{ 0, 255, 255},{192, 192,   0},{ 0,  0, 0},{64, 64, 0},{127,   0, 128},{  0,   0, 255},{  0,   0, 255},{255,   0, 0}},
                        {{0,   0,128},{  0,   0, 0},{ 0,  64,  64},{  0,   0,   0},{64, 96, 0},{ 0,  0, 0},{  0,   0,   0},{  0,   0, 128},{  0,   0, 128},{128, 192, 0}}
                      }; // defined colors sections in scenes
int R; // rosso
int G; // verde
int B; // blu
int sezione; // section number
int scena; // scene number
int oldscena = 99; // previous scene
int scenadissolve; // scene fade
int oldscenadissolve = 99; // previous scene fade
int n; // scene step number while fade
int fadeTime = 50; // milliseconds of step 
boolean dissolve; // true when fade
boolean fixed; // true when fixed scene
int sceneTime = 5000; // millisecond fixed scene
unsigned long stepdissolveTime; // used to measure the duration of the fade
unsigned long fixedTime; // it is used to measure the duration of the fixed scene
float stepvalue[10][3]; // matrix of color variation steps during fading

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
  for (int i = 0; i < 10; i++) {
    colora(i, scene[scena][i][0] + stepvalue[i][0] * n, scene[scena][i][1] + stepvalue[i][1] * n, scene[scena][i][2] + stepvalue[i][2] * n);
  }
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
    oldvalbutton = false;  // sets the old button state to false
    cambiato = false; // the state of the button action is false 
  }
  if (valbutton == false && oldvalbutton && cambiato == false && millis() - myTime > 100) {
    oldvalbutton = false;
    scena = scena + 1;
    Serial.println(scena);
    if (scena == 5) scena = 0;
    if (scena != oldscena) {
      oldscena = scena;
      if (scena < 4) {
        for (int i = 0; i < 10; i++) {
          colora(i, scene[scena][i][0] + stepvalue[i][0] * n, scene[scena][i][1] + stepvalue[i][1] * n, scene[scena][i][2] + stepvalue[i][2] * n);
        }
      }
    }
  }
  if (scena == 4) fade();
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
      oldvalbutton = false;  // sets the old button state to false
      cambiato = false; // the state of the button action is false 
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

void fade() {
  if (scenadissolve != oldscenadissolve && dissolve == false) {
    oldscenadissolve = scenadissolve;
    dissolve = true;
    for (int i = 0; i < 10; i++) { // i è il numero della sezione
      for (int k = 0; k < 3; k++) { // k è il numero del colore 0 - R, 1 - G, 2 - B
        if (scenadissolve < 3) {
          stepvalue[i][k] = ((float)scene[scenadissolve + 1][i][k] - (float)scene[scenadissolve][i][k]) / 100.00; // fa la differenza fra il valore del canale i della scena successiva (j + 1) e quella corrente (j) / 100
        } else {
          stepvalue[i][k] = ((float)scene[0][i][k] - (float)scene[scenadissolve][i][k]) / 100.00;
        }
      }
    }
    stepdissolveTime = millis();
  }
  if (dissolve) {    
    if (n < 100) {
      if (millis() - stepdissolveTime > fadeTime) {     
        for (int i = 0; i < 10; i++) {
          colora(i, scene[scenadissolve][i][0] + stepvalue[i][0] * n, scene[scenadissolve][i][1] + stepvalue[i][1] * n, scene[scenadissolve][i][2] + stepvalue[i][2] * n);
        }
        n = n + 1;
        stepdissolveTime = millis();
      }
    } else {
      n = 0;
      dissolve = false;
      fixed = true;
      fixedTime = millis();
    }
  }
  if (fixed && millis() - fixedTime > sceneTime) {
    fixed = false;
    scenadissolve = scenadissolve + 1;
    if (scenadissolve == 4) scenadissolve = 0;
  }
}
