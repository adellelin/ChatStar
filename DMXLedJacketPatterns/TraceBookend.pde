
public class TraceBookend extends Pattern {

  color paintLed (float position, float remaining, color previous) {

    // trace
    if ((abs(sq(position - .5)-remaining)) >.7)
      return color(255, 0, 255);
 
    //if ((abs(-sq(position - .5)-remaining)) <.85)
    //  return color(255, 0, 255);

    //fade
    //return color(0, 0, 0);
    
    return color(
     hue(previous), 
     saturation(previous), 
     brightness(previous)-10);
  }
}
