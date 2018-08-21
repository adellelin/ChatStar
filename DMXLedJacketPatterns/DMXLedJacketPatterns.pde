// patternmaking sandbox for testing pattern ideas
// created by @mpinner and @adellelin

import ch.bildspur.artnet.*;

ArtNetClient artnet;
byte[] dmxData = new byte[512];

int pattern = 0;

int ledCount = 88;

// set an array of colors for each LED in the mix
color led[][] = new color[4][ledCount]; // [number of strands][led's on each strand]

long ellapseTimeMs[] = new long[4];
long ellapseTimeMsStartTime[] = new long[4];

float durationMs = 3000;

Pattern patterns[] = {
  new Trace(),
  new WhiteSparkle(),
  new TraceBookend(),
  new TraceDown()
};

void setup () {
  size(400, 400);
  colorMode(HSB);

  // definte the initial LED color
  for (int i =0; i < 4; i++) {
    for (int j = 0; j<ledCount; j ++) {
      led[i][j] = color(255/ledCount*j, 255/4*i, 255);
    }
  }

  led[0][0] = color(0, 0, 0);
  
  // create artnet client without buffer (no receving needed)
  artnet = new ArtNetClient(null);
  artnet.start();
}

void draw() {

  for (int i =0; i < 4; i++) 
    
    for (int j = 0; j<ledCount; j ++) {
      // after the duration of 1 loop
      if (ellapseTimeMs[i] > durationMs) {
        //led[i][j] = 0;
      } else {
      float position = j/(float) ledCount;
      float remaining = 1.0 - ellapseTimeMs[i]/durationMs;
      
      led[i][j] = patterns[i].paintLed (position, remaining, led[i][j]);
      }
      
   /*   if (brightness(led[i][j]) == 0) {
        led[i][j] = color(random(255), 0, random(255));
      } else {
        led[i][j] = color(
          hue(led[i][j]), 
          saturation(led[i][j]), 
          brightness(led[i][j])-5);
      }*/
    }
  show();
  updateEllapseTime();
  delay(16);
}

void mousePressed() {
  int strand = mouseX / (width / 4);
  println("strand " + strand + " clicked");
  ellapseTimeMsStartTime[strand] = 0;
  updateEllapseTime();

  for (int i = 0; i<4; i++) {
    if (i!=0) print(", ");
    print(ellapseTimeMs[i]);
  }
  println();
}

// clock function
void updateEllapseTime() {
  for (int i = 0; i<4; i++) {
    if (ellapseTimeMsStartTime[i] == 0) {
      ellapseTimeMsStartTime[i] = millis();
      ellapseTimeMs[i] = 0;
    }
    else
      ellapseTimeMs[i] = millis() - ellapseTimeMsStartTime[i];
  }
}


void show() {
  for (int i =0; i < 4; i++) {
    int startx = width / 4 * i;
    int endx = startx + width / 4;
    for (int j = 0; j<ledCount; j ++) {
      int starty = height / ledCount * j;
      int endy = starty + height / ledCount;
     
      stroke(j%255+i*64);
      int c = color(frameCount % 360, 80, 100);
      fill(j);
      stroke(led[i][j%ledCount]);
      fill(led[i][j%ledCount]);
      //fill(c);

      rect(startx, starty, endx, endy);
    }
  }
    
  // choose pattern to run on LED strip
  pattern = mouseX / (width / 4);
  for (int i = 0; i <ledCount; i++){
    //for (int j = 0; j < 60; j= j+3){
    dmxData[i*3] = (byte) red(led[pattern][i]);
    dmxData[i*3+1] = (byte) green(led[pattern][i]);
    dmxData[i*3+2] = (byte) blue(led[pattern][i]);
    //}
  }
  artnet.broadcastDmx(0, 1, dmxData);
}