
public class Trace extends Pattern {

  color paintLed (float position, float remaining, color previous) {

    // trace
    if (abs(remaining-position) < .1)
      //if ((remaining-position) < .1)
      return color(255, 0, 255);

    //fade
    return color(
      hue(previous), 
      saturation(previous), 
      brightness(previous)-10);
  }
}
