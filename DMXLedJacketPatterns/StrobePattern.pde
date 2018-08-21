
public class StrobePattern extends Pattern {

  color paintLed (float position, float remaining, color previous) {

    int stobeClock = (int) (remaining * 100.0);

    if ( 0 == (stobeClock % 2) ) {
      return color(120, 0, 255*remaining);
    }


    //fade
    return color(
      hue(previous), 
      saturation(previous), 
      brightness(previous)*remaining);
  }
}
