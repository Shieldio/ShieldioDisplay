#include <ShieldioDisplay.h>

ShieldioDisplay display;
const uint8_t BUTTON_NEXT = 11;
const uint8_t BUTTON_SELECT = 12;
const char* const options[] = {"Hello World", "Counter", "Loading", "About"};
uint8_t selected = 0;
bool detail = false;
uint16_t counter = 0;
uint8_t progress = 0;
uint8_t lastNext = HIGH, lastSelect = HIGH;
unsigned long nextReady = 0, selectReady = 0;

void showScreen() {
  if (!detail) { display.menu("SHIELDIO RED", options, 4, selected); return; }
  display.clear();
  if (selected == 0) display.text(0, 2, "Hello World!");
  if (selected == 1) {
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "Count: %u", counter);
    display.text(0, 2, buffer);
  }
  if (selected == 2) {
    display.text(0, 0, "PROGRESS");
    display.loading(progress);
    return;
  }
  if (selected == 3) {
    display.text(0, 1, "SSD1306 128x64");
    display.text(0, 2, "I2C 400 kHz");
  }
  display.text(0, 7, "D12: BACK");
  display.render();
}

void setup() {
  // Requires external pull-up resistors: button press = LOW.
  pinMode(BUTTON_NEXT, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  display.begin();
  display.setDimTimeout(60000UL);
  display.setBrightness(127, 8);
  display.logo();
  delay(1000);
  showScreen();
}

void loop() {
  display.tick();
  unsigned long now = millis();
  uint8_t next = digitalRead(BUTTON_NEXT);
  uint8_t select = digitalRead(BUTTON_SELECT);
  if (lastNext == HIGH && next == LOW && now >= nextReady) {
    nextReady = now + 180;
    display.wake();
    if (!detail) selected = (selected + 1) % 4;
    else if (selected == 1) ++counter;
    else if (selected == 2) progress = (progress + 10) % 110;
    showScreen();
  }
  if (lastSelect == HIGH && select == LOW && now >= selectReady) {
    selectReady = now + 180;
    display.wake();
    detail = !detail;
    showScreen();
  }
  lastNext = next;
  lastSelect = select;
}
