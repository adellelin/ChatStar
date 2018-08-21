
abstract public class Pattern {
  // position is current position of LED within strand
  // remaining is proportionate time to end of duration
  // previous is previous LED color
  abstract color paintLed (float position, float remaining, color previous);
}
