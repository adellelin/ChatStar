
public class TraceDown extends Pattern {

  color paintLed (float position, float remaining, color previous) {

    // trace
    if (abs(remaining/2-position) < .05) 
      return color(255, 0, 255);

    if (abs(1.0-remaining/2-position) < .05) 
      return color(255, 0, 255);

    //fade
    return color(0, 0, 0);
  }
}
