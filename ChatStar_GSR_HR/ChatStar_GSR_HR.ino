// used on jacket that interfaces with buttons
#include "FastLED.h"

#include <elapsedMillis.h>

#include "RainbowSparkle.h"
#include "WhiteTrace.h"
#include "BookendTrace.h"
#include "Twinkler.h"
#include "AllOn.h"
#include "BookendFlip.h"

// How many leds in your strip?
#define NUM_LEDS 38

#define ASHLEY_LEFT  1
#define ASHLEY_RIGHT 2
#define JAMEY_LEFT   11
#define JAMEY_RIGHT  12

#define ADELLE 0
#define MATT 5

#define ADC_TEENSY_3_5 // change this depending on which board

#ifdef ADC_TEENSY_3_1 // sets up pin conditions for the 3.2 boards
#define DATA_PIN_RIGHT 23  // LED : YELLOW, connector : GREEN
#define CLOCK_PIN_RIGHT 22 // LED : GREEN,  connector : BLACK
#define DATA_PIN_LEFT 18   // LED : YELLOW, connector : GREEN
#define CLOCK_PIN_LEFT 19  // LED : GREEN,  connector : BLACK
#else
#define DATA_PIN_RIGHT 8
#define CLOCK_PIN_RIGHT 7
#define DATA_PIN_LEFT 4
#define CLOCK_PIN_LEFT 3
#endif

/// GSR
const int LED = 13;
//const int GSR=A2;
const int GSR = 16;
int threshold = 0;
int sensorValue;
int temp;
float hue;

// Polar HR
const int HR_RX = 9; // teensy 3.5 sRX3
long oldSample, sample;
long beatSum = 0;
long beatAverage = 0;
long beatSumArray = 0;
int beatSumArrayCount = 0;

elapsedMillis ellapseHRTimeMs;
unsigned long timeCounter = 0;
unsigned long HRdurationMs = 60000; // for bpm calc


Pattern* patterns[] = {
  new BookendFlip(), // pattern individually triggered
  new WhiteTrace(), // pattern upon receive
  new RainbowSparkle(),
  new BookendTrace(),
  new WhiteTrace(),
  new Twinkler()
};

int ledHue[] = {0, 200, 255, 120, 160, 120};

elapsedMillis ellapseTimeMs[(sizeof(patterns)) / 4];

float crazyDuration = 2000;
float regularDuration = 2000;
float gsrDuration = 3000;
float fastDuration = 1500;
float durationMs = crazyDuration;
float hrDuration = 1000;

float repeatDurationMs = 6000;
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

  // setup pins for NORMAL vs CRAZY
  pinMode(10, INPUT_PULLUP);
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
  pinMode(12, INPUT_PULLUP);

  Serial.println("resetting");
  LEDS.addLeds<WS2801, DATA_PIN_LEFT, CLOCK_PIN_LEFT, RGB>(leds_left, NUM_LEDS);
  LEDS.addLeds<WS2801, DATA_PIN_RIGHT, CLOCK_PIN_RIGHT, RGB>(leds_right, NUM_LEDS);
  LEDS.setBrightness(84);

  for (int i = 0; i < (sizeof(patterns)) / 4; i++) {
    for (int j = 0; j < NUM_LEDS; j ++) {
      led[i][j] = CHSV(255 / NUM_LEDS * j, 255 / (sizeof(patterns)) / 4 * i, 255);
    }
  }

  inputString.reserve(200);

  GSRSetup();

  HRSetup();

}

void GSRSetup() {


  // pinMode(LED, OUTPUT);
  // digitalWrite(LED, LOW);
  // delay(1000);

  calibrate();
}

void HRSetup() {
  delay(1000);
  Serial.println("begin");
  pinMode (HR_RX, INPUT);  //Signal pin to input

  Serial.println("Waiting for heart beat...");

  // Wait until a heart beat is detected
  /*
    while (!digitalRead(HR_RX)) {};
    Serial.println ("Heart beat detected!");
  */
}

void calibrate () {

  long sum = 0;

  // calibrating
  for (int i = 0; i < 500; i++) {
    sensorValue = analogRead(GSR);
    sum += sensorValue;
    delay(10);
  }
  threshold = sum / 500;
  Serial.print("threshold =");
  Serial.println(threshold);

  return;
}



void loop() {
  // available() -- Get the number of bytes (characters) available
  // for reading from the serial port.
  // This is data that's already arrived and stored in the
  // serial receive buffer (which holds 64 bytes).

  while (Serial1.available()) {

    char inChar = (char)Serial1.read();
    Serial.println(inChar);
    Serial.println(sizeof(soundCommand));
    //Serial.println(sizeof(patterns)/4);
    if (inChar == soundCommand[0]) {
      ellapseTimeMs[0] = 0;
      durationMs = regularDuration;
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
    } else if (inChar == 'C') {
      calibrate();
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

  ///GSR
  GSRCalc();


  HRCalc();


  //choosePattern(ADELLE);


  // send message to other jacket after pattern duration
  if (false == sentAlready) {
    // if (ellapseTimeMs[ADELLE] > durationMs / 4) { // sending halfway through
    if (ellapseTimeMs[ADELLE] > 0) { // instantaneous
      Serial.print("e:");
      Serial.println(ellapseTimeMs[ADELLE]);
      //Serial1.print(soundCommand[ADELLE]);
      Serial1.print(soundCommand[6]);
      sentAlready = true;

    }
  }

  //determining LED patterns
  for (int i = 0; i < (sizeof(patterns)) / 4; i++) {
    for (int j = 0; j < NUM_LEDS; j ++) {

      if (ellapseTimeMs[i] > durationMs ) {
        led[i][j] = 0; // turn all LED to black once time hits

        digitalWrite(LED, LOW); // turn indicator LED off

      } else {
        float position = j / (float)NUM_LEDS;
        float remaining = 1.0 - ellapseTimeMs[i] / durationMs;
        //led[0][j] = patterns[i]->paintLed (position, remaining, led[0][j], ledHue[i]);
        led[0][j] = patterns[i]->paintLed (position, remaining, led[0][j], hue); //GSR

      }

    }
  }

  show(ADELLE);
  // delay(16);

}

void GSRCalc() {
  sensorValue = analogRead(GSR);
  //Serial.print("sensorValue=");
  //Serial.println(sensorValue);
  temp = threshold - sensorValue;
  /*
    Serial.print(sensorValue );
    Serial.print(" : ");
    Serial.print(temp );
    Serial.print(" with thresh: ");
    Serial.print(threshold );
    Serial.print(" -> color val: ");
    Serial.println((abs(temp)) / (float)threshold * 255);
  */

  hue = ((abs(temp)) / (float)threshold) * 255 * 2;
  //  Serial.println(hue);
}

void HRCalc() {
  sample = digitalRead(HR_RX);  //Store signal output
  //Serial.println(sample);
  //Serial.println(ellapseHRTimeMs);
  if (sample && (oldSample != sample)) {

    Serial.println("Beat");
    ellapseTimeMs[ADELLE] = 0;
    durationMs = hrDuration;
    sentAlready = false;

    beatSum += sample;
    Serial.println(beatSum);

  }
  if (ellapseHRTimeMs > HRdurationMs) {
    Serial.print("bpm: ");
    Serial.println(beatSum);
    beatSumArray += beatSum;
    beatSumArrayCount += 1;
    int averageBpm = beatSumArray / beatSumArrayCount;
    Serial.print("Sum bpm: ");
    Serial.println(beatSumArray);
    Serial.print("Sum Minutes: ");
    Serial.println(beatSumArrayCount);


    Serial.print("Average bpm: ");
    Serial.println(averageBpm);
    beatSum = 0;
    ellapseHRTimeMs = 0;

  }
  oldSample = sample;           //Store last signal received
}

void show(char whoseJacket) {

  switch (whoseJacket) {
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

void choosePattern(char whoseJacket) {
  switch (whoseJacket) {
    case ADELLE:
      // autofire over repeat duration
      if (ellapseTimeMs[ADELLE] > repeatDurationMs) {
        ellapseTimeMs[ADELLE] = 0;
        durationMs = regularDuration;
        sentAlready = false;
      }
      break;

    case MATT:
      if (ellapseTimeMs[MATT] > repeatDurationMs) {
        ellapseTimeMs[MATT] = 0;
        durationMs = regularDuration;
        sentAlready = false;

      }
      break;
  }
}


void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds_right[i].nscale8(200);
    leds_left[i].nscale8(200);
  }
}

boolean isRegular() {
  return true;
  //return digitalRead(10);
}

boolean isCrazy() {
  return false;
  //return digitalRead(12);
}





