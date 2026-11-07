#include "ShieldioDisplay.h"
#include <string.h>

namespace {
// Draw the same small scene eight times, keeping just one SSD1306 page in RAM.
struct FacePage {
  uint8_t bytes[128];
  uint8_t page;
  void pixel(int x, int y) {
    if (x >= 0 && x < 128 && y >= 0 && y < 64 && y / 8 == page)
      bytes[x] |= (uint8_t)(1U << (y % 8));
  }
  void line(int x, int y, int ex, int ey) {
    int dx = ex > x ? ex - x : x - ex, dy = ey > y ? ey - y : y - ey;
    int sx = x < ex ? 1 : -1, sy = y < ey ? 1 : -1, err = dx - dy;
    for (;;) {
      pixel(x, y);
      if (x == ex && y == ey) break;
      int twice = 2 * err;
      if (twice > -dy) { err -= dy; x += sx; }
      if (twice < dx) { err += dx; y += sy; }
    }
  }
  void circle(int x, int y, int r) {
    int dx = r, dy = 0, err = 1 - r;
    while (dx >= dy) {
      pixel(x+dx,y+dy); pixel(x-dx,y+dy); pixel(x+dx,y-dy); pixel(x-dx,y-dy);
      pixel(x+dy,y+dx); pixel(x-dy,y+dx); pixel(x+dy,y-dx); pixel(x-dy,y-dx);
      ++dy;
      if (err < 0) err += 2*dy+1;
      else { --dx; err += 2*(dy-dx)+1; }
    }
  }
  void eye(int x, ShieldioDisplay::Mood mood, bool blink) {
    using Mood = ShieldioDisplay::Mood;
    if (blink || mood == Mood::Sleepy) { line(x-5,29,x+5,29); return; }
    if (mood == Mood::Happy) { line(x-5,29,x,25); line(x,25,x+5,29); return; }
    circle(x,28,4);
    line(x,27,x,29);
    if (mood == Mood::Sad) line(x-5,21,x+5,18);
    if (mood == Mood::Angry) line(x-5,18,x+5,22);
  }
  void scene(ShieldioDisplay::Animal animal, ShieldioDisplay::Mood mood, bool blink) {
    using Animal = ShieldioDisplay::Animal;
    using Mood = ShieldioDisplay::Mood;
    if (animal == Animal::Lion) {
      // A scalloped mane, kept clear of the face.
      circle(64,32,30);
      for (int x = 40; x <= 88; x += 12) {
        line(x,5,x+3,9); line(x,59,x+3,55);
      }
      circle(43,15,5); circle(85,15,5);
    } else if (animal == Animal::Dog) {
      line(43,15,32,12); line(32,12,28,38); line(28,38,37,43); line(37,43,43,20);
      line(85,15,96,12); line(96,12,100,38); line(100,38,91,43); line(91,43,85,20);
    } else {
      line(42,18,40,3); line(40,3,56,12);
      line(86,18,88,3); line(88,3,72,12);
      line(44,14,43,9); line(84,14,85,9);
    }
    // Rounded cheeks and chin; upper edge leaves room for ears.
    line(44,15,54,11); line(54,11,74,11); line(74,11,84,15);
    line(44,15,39,25); line(39,25,40,43); line(40,43,49,52);
    line(49,52,64,55); line(64,55,79,52); line(79,52,88,43);
    line(88,43,89,25); line(89,25,84,15);
    eye(52,mood,blink); eye(76,mood,blink);
    line(60,35,68,35); line(60,35,64,39); line(68,35,64,39);
    if (mood == Mood::Surprised) circle(64,46,4);
    else if (mood == Mood::Sad || mood == Mood::Angry) {
      line(57,48,64,44); line(64,44,71,48);
    } else if (mood == Mood::Sleepy) line(60,46,68,46);
    else {
      line(64,39,64,44); line(55,43,59,47); line(59,47,64,44);
      line(64,44,69,47); line(69,47,73,43);
    }
    if (animal == Animal::Cat || animal == Animal::Lion) {
      line(34,34,48,37); line(33,42,48,40);
      line(80,37,94,34); line(80,40,95,42);
    }
  }
};
}

void ShieldioDisplay::face(Animal animal, Mood mood, bool blink) {
  FacePage frame;
  for (frame.page = 0; frame.page < 8; ++frame.page) {
    memset(frame.bytes, 0, sizeof(frame.bytes));
    frame.scene(animal, mood, blink);
    if (canvas_) {
      memcpy(canvas_ + (uint16_t)frame.page * 128, frame.bytes, 128);
      dirtyStart_[frame.page] = 0;
      dirtyEnd_[frame.page] = 127;
    } else {
      window(0, 127, frame.page);
      for (uint8_t x = 0; x < 128;) {
        Wire.beginTransmission(address_);
        Wire.write((uint8_t)0x40);
        for (uint8_t n = 0; n < 24 && x < 128; ++n, ++x) Wire.write(frame.bytes[x]);
        Wire.endTransmission();
      }
    }
  }
  // A later text render must restore every cell after the direct screen write.
  invalid_ = true;
  if (canvas_) { memset(cells_, ' ', sizeof(cells_)); render(); }
}
