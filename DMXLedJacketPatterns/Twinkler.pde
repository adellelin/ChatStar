
public class Twinkler extends Pattern {

  color paintLed (float position, float remaining, color previous) {

   // increases toward larger position
   float closeness = 1.0 - abs((1.0-position) - remaining);
    
   // decreases toward larger position
   float closenessBackwards = 1.0 - abs((1.0-position) - remaining);

    // time and position are close
    if (closeness > .90) {
      return color (160, 0, 255*closeness*remaining);
    }
    
    // time and position are not close
    if (closenessBackwards < .30) {
      return color (100, 0, 255*remaining);
    }

    // randomly add bright pixels
    if (remaining > 0.95) {
      if (5 > random(100)) {
        return color (160, 0, 255);
      }
    } 
/*
    if (remaining > 0.85) {
      if (5 > random(100)) {
        return color (100, 0, 255);
      }
    } */


    if (10 > random(100)) {

      return color(
        hue(previous), 
        saturation(previous), 
        brightness(previous)-20);
    }


    return previous;
    
    
  }
}
