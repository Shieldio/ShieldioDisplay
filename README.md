# ShieldioDisplay

Minimal OLED text/menu library for **SSD1306 128×64 I²C** and **Arduino Nano (ATmega328P)**.

**Status: v0.1.0 development snapshot. Not yet hardware-validated or published in Arduino Library Manager.**

## Features
- 16×8 ASCII character grid, text, menus and progress indicator.
- Changed-character updates (128 B desired + 128 B displayed text buffers).
- Uses Arduino `Wire` at 400 kHz; no external display library.
- Text-only Shieldio boot placeholder. Official logo bitmap is planned.

## Wiring
OLED SDA → A4, SCL → A5; power according to the OLED module specifications.
Example buttons: D11 / D12 with **external pull-up resistors**, pressed = LOW.

## Example
```cpp
#include <ShieldioDisplay.h>
ShieldioDisplay display;
void setup() {
  display.begin();
  display.text(0, 0, "Hello World!");
  display.render();
}
void loop() {}
```

## Editing and contributing
Change implementation in `src/`, demo in `examples/ShieldioDemo/`.
Open a branch, commit, and submit a pull request. The Arduino compile workflow
checks the example for `arduino:avr:nano` on each push/PR. It does **not**
replace a real-device test. See [CONTRIBUTING.md](CONTRIBUTING.md).

## Installation
Before registry publication: Arduino IDE → Sketch → Include Library → Add .ZIP Library,
using the repository's ZIP download. After registry acceptance: search for
`ShieldioDisplay` in Library Manager.

## Limitations
SSD1306 only (not SH1106), 128×64 only, printable ASCII only; no Czech diacritics.
The boot screen currently uses text instead of the official SVG-derived bitmap.
Wire transfer errors are not currently reported by this API.

## Release / Arduino Library Manager
1. Ensure Arduino CI passes and test on Nano hardware.
2. Create a GitHub release with semantic tag `v0.1.0`.
3. Submit the repository URL to [Arduino Library Registry](https://github.com/arduino/library-registry).
4. After acceptance, subsequent tagged releases are indexed automatically.

License: MIT.
