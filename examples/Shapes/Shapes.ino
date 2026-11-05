#include <ShieldioDisplay.h>
ShieldioCanvas display;
void setup() {
  display.begin();
  display.text(0, 0, "Shapes");
  display.rect(4, 16, 20, 20);
  display.rect(30, 16, 32, 20, true);
  display.circle(92, 32, 12);
  display.line(0, 60, 127, 60);
  display.pixel(127, 63);
  display.render();
}
void loop() {}
