// used on jacket that interfaces with buttons
#include "FastLED.h"

#include <elapsedMillis.h>


// IMU Includes
#include <NXPMotionSense.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
SoftwareSerial xBeeSerial(20, 21); // RX, TX

#include "RainbowSparkle.h"
#include "WhiteTrace.h"
#include "BookendTrace.h"
#include "Twinkler.h"
#include "BookendFlip.h"

// How many leds in your strip?
#define NUM_LEDS 40

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
#define DATA_PIN_LEFT 11
#define CLOCK_PIN_LEFT 13
#else
#define DATA_PIN_RIGHT 8
#define CLOCK_PIN_RIGHT 7
#define DATA_PIN_LEFT 4
#define CLOCK_PIN_LEFT 3
#endif

#define MY_PATTERN_ID 1


// todo:
// add per pattern duration
// add per pattern cmd leter
// add per pattern default colors
Pattern* patterns[] = {
  new BookendFlip(), // pattern individually triggered
  new WhiteTrace(), // pattern upon receive
  new RainbowSparkle(),
  new BookendTrace(),
  new WhiteTrace(),
  new Twinkler()
};

int inboundHue = 229;

int ledHue[] = { 0, 20, 255, 229, 160, 120};
// 120 was cyan
// 229 pink
// 22 orange
// 200 lilac

elapsedMillis ellapseTimeMs[(sizeof(patterns)) / 4];

float regularDuration = 10000;
float fastDuration = 3000;
float durationMs = regularDuration;

float repeatDurationMs = 10000;
boolean sentAlready = false; // send message boolean

// for message sending timer
float sendQuaternionDur = 100;
elapsedMillis sendQuaternionMs;
char quaternionValue;

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

//IMU GLobal Fields
NXPMotionSense imu;
NXPSensorFusion filter;

float accel1;
float accel2;
float velocity = 0;
float preVelocity = 0;
float distanceCur = 0;
float distancePrev = 0;
float distance = 0;

float roll, pitch, heading;

float hue;
float COMPLEXITY = 15.0;
float complexity_velocity;

void setup() {

  // setup serial DEBUG and XBEE
  Serial.begin(9600);
  xBeeSerial.begin(9600);
  Serial1.begin(9600);

  //boot IMU
  imu.begin();
  filter.begin(100);

  // setup pins for ASHLEY vs JAMEY
  pinMode(10, INPUT_PULLUP);
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
  pinMode(12, INPUT_PULLUP);

  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);  // enable access to LEDs

  Serial.println("resetting");
  LEDS.addLeds<WS2801, DATA_PIN_LEFT, CLOCK_PIN_LEFT, RGB>(leds_left, NUM_LEDS);
  LEDS.addLeds<WS2801, DATA_PIN_RIGHT, CLOCK_PIN_RIGHT, RGB>(leds_right, NUM_LEDS);
  LEDS.setBrightness(32);
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

  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;

  // calc imu
  if (imu.available()) {
    // Read the motion sensors
    imu.readMotionSensor(ax, ay, az, gx, gy, gz, mx, my, mz);
    // Update the SensorFusion filter
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
    quaternionValue = (char)map(filter.qPl.q2, -0.1, 0.7, 128, 255) ;
    quaternionValue = constrain(quaternionValue, 128, 255);
    //Serial1.print(filter.qPl.q2);
    Serial.println(filter.qPl.q2);
    // print the heading, pitch and roll
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();
  }

  while (Serial1.available()) {

    char inChar = (char)Serial1.read();
    Serial.println(inChar);
    //Serial.println(sizeof(soundCommand));
    //Serial.println(sizeof(patterns)/4);
    durationMs = regularDuration;
    if (inChar == soundCommand[0]) {
      ellapseTimeMs[0] = 0;
    } else if (inChar == soundCommand[1]) {
      ellapseTimeMs[1] = 0;
    } else if (inChar == soundCommand[2]) {
      ellapseTimeMs[2] = 0;
    } else if (inChar == soundCommand[3]) {
      ellapseTimeMs[3] = 0;
    } else if (inChar == soundCommand[4]) {
      ellapseTimeMs[4] = 0;
    } else if (inChar == soundCommand[5]) {
      ellapseTimeMs[5] = 0;
    } else {
      inboundHue = inChar;
    }
  }

  // send quaternion data via xbee over duration
  if (sendQuaternionMs >  sendQuaternionDur) {
    Serial1.print(quaternionValue);
    sendQuaternionMs = 0;
  }


  choosePattern();


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
        // always use incoming hue from other person
        int color = inboundHue;
        // unless pattern ID when mine
        if(i == MY_PATTERN_ID) {
          color = quaternionValue;
        }
        led[0][j] = patterns[i]->paintLed (position, remaining, led[0][j], color);
        //Serial.println(ledHue[i]);
      }

    }
  }
  
  if ( abs(inboundHue - quaternionValue) < 5) {
    durationMs = fastDuration;
    if (ellapseTimeMs[2] > (fastDuration + 3000)) { // after 5 seconds
      ellapseTimeMs[2] = 0;

    }
  } else {
    durationMs = regularDuration;
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

  // autofire over repeat duration
  if (ellapseTimeMs[MY_PATTERN_ID] > repeatDurationMs) {
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




