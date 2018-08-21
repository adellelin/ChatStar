
public class RainbowSparkle extends Pattern {

color paintLed (float position, float remaining, color previous) {
 
 // println("position:" + position +", remaining:" + remaining); 
  if (random(1000) < 1)
     return color(position*255,255, 255*remaining);
 
  return color(
          hue(previous), 
          saturation(previous), 
          brightness(previous)-5);
          
}

}
