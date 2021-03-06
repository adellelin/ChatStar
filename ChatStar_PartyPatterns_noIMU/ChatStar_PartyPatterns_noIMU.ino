// used on jacket that interfaces with buttons
#include "FastLED.h"

#include <elapsedMillis.h>

#include "RainbowSparkle.h"
#include "WhiteTrace.h"
#include "BookendTrace.h"
#include "Twinkler.h"
#include "BookendFlip.h"

// How many leds in your strip?
#define NUM_LEDS 18

#define ASHLEY_LEFT  1
#define ASHLEY_RIGHT 2
#define JAMEY_LEFT   11
#define JAMEY_RIGHT  12


#define ADELLE 1 // use this when other jacket is HR, otherwise 0
#define MATT 0 // we used MATT 0 for white jacket

#define WHOSE_HARDWARE MATT // little black jacet with one long strip split to two connectors

#define ADC_TEENSY_3_1 // change this depending on which board

#ifdef ADC_TEENSY_3_1 // sets up pin conditions for the 3.2 boards
#define DATA_PIN_RIGHT 23
#define CLOCK_PIN_RIGHT 22
#define DATA_PIN_LEFT 18
#define CLOCK_PIN_LEFT 19
#else
#define DATA_PIN_RIGHT 8
#define CLOCK_PIN_RIGHT 7
#define DATA_PIN_LEFT 4
#define CLOCK_PIN_LEFT 3
#endif

#define MY_PATTERN_ID 3

Pattern* patterns[] = {
  new BookendFlip(), // pattern individually triggered
  new WhiteTrace(), // pattern upon receive
  new RainbowSparkle(),
  new BookendTrace(),
  new WhiteTrace(),
  new Twinkler()
};

int ledHue[] = { 0, 20, 255, 229, 160, 120};
// 120 was cyan
// 229 pink
// 22 orange
// 200 lilac

elapsedMillis ellapseTimeMs[(sizeof(patterns)) / 4];

float crazyDuration = 2000;
float regularDuration = 3000;
float fastDuration = 2000;
float durationMs = crazyDuration;

float repeatDurationMs = 10000;
float repeatCrazyMs = 3000;
boolean sentAlready = false; // send message boolean

elapsedMillis sendEllapseMs;
float sendMs = 2000;


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete



CRGB led[(sizeof(patterns)) / 4][NUM_LEDS];

// Define the array of leds
CRGB leds_right[NUM_LEDS];
CRGB leds_left[NUM_LEDS];

char soundCommand[] = {
  'q', 'a', 'z',  // mushroom 0 is when all off
  'w', 's', 'x',  // mushroom 1 is when only 4 on
  'e', 'd', 'c',  // mushroom 2 is when only 3 on
  'r', 'f', 'v'   // mushroom 3 is when 3 and 4 on
};

void setup() {

  // setup serial DEBUG and XBEE
  Serial.begin(9600);
  Serial1.begin(9600);


  // setup pins for ASHLEY vs JAMEY
  pinMode(10, INPUT_PULLUP);
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
  pinMode(12, INPUT_PULLUP);

  Serial.println("resetting");
  LEDS.addLeds<WS2801, DATA_PIN_LEFT, CLOCK_PIN_LEFT, RGB>(leds_left, NUM_LEDS);
  LEDS.addLeds<WS2801, DATA_PIN_RIGHT, CLOCK_PIN_RIGHT, RGB>(leds_right, NUM_LEDS);
  LEDS.setBrightness(255);
  LEDS.setDither( 0 );
  FastLED.setDither( 0 );

        
  Serial.print("uploaded.. ");
  Serial.println(__FILE__);
        
  Serial.print("reseting jacket.. ");
  Serial.println(__DATE__);


  Serial.print("jacket switch is");
  if (isCrazy()) {
    Serial.println(" CRAZY");
  }
  if (isRegular()) {
    Serial.println(" REGULAR");
  }
        

  for (int i = 0; i < (sizeof(patterns)) / 4; i++) {
    for (int j = 0; j < NUM_LEDS; j ++) {
      led[i][j] = CHSV(255 / NUM_LEDS * j, 255 / (sizeof(patterns)) / 4 * i, 255);
    }
  }

  inputString.reserve(200);
}

void loop() {
  // available() -- Get the number of bytes (characters) available
  // for reading from the serial port.
  // This is data that's already arrived and stored in the
  // serial receive buffer (which holds 64 bytes).

  while (Serial1.available()) {

    char inChar = (char)Serial1.read();
    Serial.println(inChar);
    //Serial.println(sizeof(soundCommand));
    //Serial.println(sizeof(patterns)/4);
    if (inChar == soundCommand[0]) {
      ellapseTimeMs[0] = 0;
      durationMs = fastDuration;
    } else if (inChar == soundCommand[1]) {
      ellapseTimeMs[1] = 0;
      durationMs = regularDuration;
    } else if (inChar == soundCommand[2]) {
      ellapseTimeMs[2] = 0;
      durationMs = crazyDuration;
    } else if (inChar == soundCommand[3]) {
      ellapseTimeMs[3] = 0;
      durationMs = fastDuration;
    } else if (inChar == soundCommand[4]) {
      ellapseTimeMs[4] = 0;
      durationMs = crazyDuration;
    } else if (inChar == soundCommand[5]) {
      ellapseTimeMs[5] = 0;
      durationMs = crazyDuration;
    }
  }


  /*
    // autosend every 2s
    if (ellapseTimeMs[0] > sendMs) {
    Serial1.print('a');
    Serial.print('a');
    sendEllapseMs = 0;
    }
  */

  if (isRegular()) {
    choosePattern();
  }  else {
    choosePattern();
  }


  // send message to other jacket after pattern duration
  if (false == sentAlready) {
    //if (ellapseTimeMs[MY_PATTERN_ID] > durationMs / 4) { // sending halfway through
    if (ellapseTimeMs[MY_PATTERN_ID] > 0) { // sending halfway through
      Serial.print("sending::");
      Serial.print(" my-pattern-id:");
      Serial.print(MY_PATTERN_ID);
      Serial.print(" cmd:");
      Serial.println(soundCommand[MY_PATTERN_ID]);
      Serial1.print(soundCommand[MY_PATTERN_ID]);
      sentAlready = true;

    }
  }

  for (int i = 0; i < (sizeof(patterns)) / 4; i++) {
    for (int j = 0; j < NUM_LEDS; j ++) {

      if (ellapseTimeMs[i] > durationMs ) {
        led[i][j] = 0; // turn all LED to black once time hits
      } else {
        float position = j / (float)NUM_LEDS;
        float remaining = 1.0 - ellapseTimeMs[i] / durationMs;
        led[0][j] = patterns[i]->paintLed (position, remaining, led[0][j], ledHue[i]);
        //Serial.println(ledHue[i]);
      }

    }
  }

  show();
  delay(10);

}


void show() {

  switch (WHOSE_HARDWARE) {
    case ADELLE :
      for (int i = 0; i < 20; i ++) {
        leds_left[i] = led[0][i];
        leds_right[i] = led[0][i];
      }
      for (int i = 0; i < 20; i ++) {
        leds_left[i + 20] = led[0][20 - i];
        leds_right[i + 20] = led[0][20 - i];
      }
      break;
    case MATT:
      for (int i = 0; i < NUM_LEDS; i ++) {
        leds_right[i] = led[0][i];
        leds_left[i] = led[0][i];
      }
      break;

  }


  FastLED.show();

  return;
}

void choosePattern() {

  float repeatMs = repeatDurationMs;

  if (isCrazy() ) {
    repeatMs = repeatCrazyMs;
  }

  // autofire over repeat duration
  if (ellapseTimeMs[MY_PATTERN_ID] > repeatMs) {
    ellapseTimeMs[MY_PATTERN_ID] = 0;
    durationMs = regularDuration;
    sentAlready = false;
  }

  return;
}


void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds_right[i].nscale8(200);
    leds_left[i].nscale8(200);
  }
}

boolean isRegular() {
  //return !isCrazy();
  digitalRead(10);
}

boolean isCrazy() {
  //return false;
  digitalRead(12);
}




