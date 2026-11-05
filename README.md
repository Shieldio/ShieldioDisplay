# ShieldioDisplay

OLED text/menu and drawing library for **SSD1306 128×64 I²C** and **Arduino Nano (ATmega328P)**.

**Status: v0.2.0. Text and graphics compile for Arduino Nano; graphics still require a hardware smoke test. Not yet published in Arduino Library Manager.**

## Features
- 16×8 ASCII character grid, text, menus and progress indicator.
- Automatic one-second startup splash screen from `display.begin()`.
- Changed-character updates (128 B desired + 128 B displayed text buffers).
- Optional `ShieldioCanvas`: pixels, lines, rectangles, squares (equal width/height), outlined and filled circles. Coordinates in pixels (x 0–127, y 0–63); text keeps its character coordinates (column 0–15, row 0–7).
- `ShieldioDisplay` keeps text-only storage; `ShieldioCanvas` adds a fixed 1024 B canvas, with changed spans sent per page and no heap allocation. Leave room for stack and other peripherals on Nano.
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

## Startup splash

`display.begin()` initializes the SSD1306, displays the text Shieldio splash for one second, then clears the screen. No additional `logo()` call is required. To display it again manually, call `display.logo()`.

## Limitations
SSD1306 only (not SH1106), 128×64 only, printable ASCII only; no Czech diacritics.
The boot screen currently uses text instead of the official SVG-derived bitmap. Automatic dimming has been removed because the observed brightness difference was negligible.
Wire transfer errors are not currently reported by this API.

## Release / Arduino Library Manager
1. Ensure Arduino CI passes and test on Nano hardware.
2. Create a GitHub release with semantic tag `v0.2.0`.
3. Submit the repository URL to [Arduino Library Registry](https://github.com/arduino/library-registry).
4. After acceptance, subsequent tagged releases are indexed automatically.

License: MIT.

## Drawing

Use `ShieldioCanvas` in place of `ShieldioDisplay` when you need graphics:

```cpp
#include <ShieldioDisplay.h>
ShieldioCanvas display;
void setup() {
  display.begin(); // Includes the same one-second splash.
  display.text(0, 0, "Shapes");
  display.rect(4, 16, 20, 20);        // Square outline.
  display.rect(30, 16, 32, 20, true); // Filled rectangle.
  display.circle(92, 32, 12);        // Circle outline.
  display.line(0, 60, 127, 60);
  display.pixel(127, 63);
  display.render();
}
void loop() {}
```

Shapes accumulate until `clear()`; use `render()` to send changes to the display.
All drawing functions accept a final `on` argument (default `true`); pass `false`
to erase. Rectangles and circles accept `filled` before `on`. Non-positive
rectangle sizes and radii outside 0–143 draw nothing. Off-screen pixels are clipped.
Text replaces its 8×8 cells, so write text before overlapping shapes.
