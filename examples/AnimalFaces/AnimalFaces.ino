#include <ShieldioDisplay.h>

ShieldioDisplay display; // No 1024-byte canvas needed.
using Animal = ShieldioDisplay::Animal;
using Mood = ShieldioDisplay::Mood;

void setup() { display.begin(); }

void loop() {
  for (uint8_t animal = 0; animal < 3; ++animal) {
    for (uint8_t mood = 0; mood < 5; ++mood) {
      display.face((Animal)animal, (Mood)mood);
      delay(1200);
      display.face((Animal)animal, (Mood)mood, true); // Blink.
      delay(150);
    }
  }
}
