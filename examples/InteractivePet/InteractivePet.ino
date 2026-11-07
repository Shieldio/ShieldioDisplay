#include <ShieldioDisplay.h>
#include <Servo.h>

// OLED: SDA A4, SCL A5. Buttons D11/D12: external pull-ups, pressed LOW.
// Servo signal D9. Set its mechanical limits to suit your cardboard tail.
ShieldioDisplay display;
Servo tail;
using Animal = ShieldioDisplay::Animal;
using Mood = ShieldioDisplay::Mood;
Animal animal = Animal::Cat;
Mood shownMood = Mood::Sleepy;
bool previousNext = false, shownBlink = false;
unsigned long lastPress = 0, lastFrame = 0;

void setup() {
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  tail.attach(9, 1000, 2000);
  tail.write(90);
  display.begin();
  display.face(animal, shownMood);
}

void loop() {
  unsigned long now = millis();
  bool pet = digitalRead(11) == LOW;
  bool next = digitalRead(12) == LOW;
  if (next && !previousNext && now - lastPress >= 200) {
    animal = (Animal)(((uint8_t)animal + 1) % 3);
    lastPress = now;
    display.face(animal, shownMood, shownBlink);
  }
  previousNext = next;
  Mood mood = pet ? Mood::Happy : Mood::Sleepy;
  bool blink = pet && now % 3000 >= 2850;
  if (now - lastFrame >= 50) {
    lastFrame = now;
    if (mood != shownMood || blink != shownBlink) {
      display.face(animal, mood, blink);
      shownMood = mood;
      shownBlink = blink;
    }
    tail.write(pet ? (now % 600 < 300 ? 65 : 115) : 90);
  }
}
