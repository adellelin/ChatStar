
public class WhiteSparkle extends Pattern {

  color paintLed (float position, float remaining, color previous) {

    //println("position:" + position +", remaining:" + remaining); 
    if (random(100) < 1)
      return color(255, 0, 255);

    return color(
      hue(previous), 
      saturation(previous), 
      brightness(previous)-10);
  }
}
