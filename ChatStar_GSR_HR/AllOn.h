#ifndef H_AOPATTERN
#define H_AOPATTERN

#include "Pattern.h"

class AllOn : public Pattern {

  CRGB paintLed (float position, float remaining, CRGB previous, int primaryHue) {

      // trace
    //  if (abs(remaining - position) < .20)
        return CHSV(primaryHue, 150, 128);
        //return CHSV(220, 200, 128); // pink


      //fade
    //  return previous.nscale8(192);

    }

};

#endif
