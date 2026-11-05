#include "ShieldioDisplay.h"
#include "ShieldioFont.h"
#include <string.h>
#include <stdio.h>

ShieldioDisplay::ShieldioDisplay(uint8_t address) : address_(address), invalid_(true), canvas_(nullptr) {
  memset(cells_, ' ', sizeof(cells_));
  memset(shown_, ' ', sizeof(shown_));
}

void ShieldioDisplay::command(uint8_t value) {
  Wire.beginTransmission(address_);
  Wire.write((uint8_t)0x00);
  Wire.write(value);
  Wire.endTransmission();
}

void ShieldioDisplay::begin() {
  Wire.begin();
  Wire.setClock(400000UL);
  delay(30);
  const uint8_t init[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
    0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
    0x81, 0x7F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
  };
  for (uint8_t i = 0; i < sizeof(init); ++i) command(init[i]);
  invalid_ = true;
  if (canvas_) { memset(dirtyStart_, 0, 8); memset(dirtyEnd_, 127, 8); }
  clear();
  render();
  logo();
  delay(1000);
  clear();
  render();
}

void ShieldioDisplay::window(uint8_t start, uint8_t end, uint8_t page) {
  command(0x21); command(start); command(end);
  command(0x22); command(page); command(page);
}

void ShieldioDisplay::glyph(uint8_t code) {
  if (code < 32 || code > 126) code = '?';
  // Font is stored as 8 rows. SSD1306 needs 8 vertical columns.
  for (uint8_t x = 0; x < 8; ++x) {
    uint8_t column = 0;
    for (uint8_t y = 0; y < 8; ++y) {
      if (pgm_read_byte(&shieldioFont[code][y]) & (1 << x)) column |= (1 << y);
    }
    Wire.write(column);
  }
}

void ShieldioDisplay::clear() {
  memset(cells_, ' ', sizeof(cells_));
  if (canvas_) {
    for (int16_t y = 0; y < 64; ++y)
      for (int16_t x = 0; x < 128; ++x) setPixel(x, y, false);
  }
}

void ShieldioDisplay::useCanvas(uint8_t* buffer) {
  canvas_ = buffer;
  memset(canvas_, 0, 1024);
  memset(dirtyStart_, 0, 8);
  memset(dirtyEnd_, 127, 8);
}

void ShieldioDisplay::setPixel(int16_t x, int16_t y, bool on) {
  if (!canvas_ || x < 0 || x >= 128 || y < 0 || y >= 64) return;
  uint8_t page = y / 8, mask = (uint8_t)(1U << (y % 8));
  uint16_t pos = (uint16_t)page * 128 + x;
  uint8_t next = on ? canvas_[pos] | mask : canvas_[pos] & (uint8_t)~mask;
  if (next == canvas_[pos]) return;
  canvas_[pos] = next;
  if (x < dirtyStart_[page]) dirtyStart_[page] = x;
  if (x > dirtyEnd_[page]) dirtyEnd_[page] = x;
}

void ShieldioDisplay::text(uint8_t col, uint8_t row, const char* value) {
  if (!value || row >= 8 || col >= 16) return;
  while (*value && col < 16) {
    uint8_t code = (uint8_t)*value++;
    cells_[row * 16 + col] = code;
    if (canvas_) {
      if (code < 32 || code > 126) code = '?';
      for (uint8_t y = 0; y < 8; ++y) {
        uint8_t bits = pgm_read_byte(&shieldioFont[code][y]);
        for (uint8_t x = 0; x < 8; ++x) setPixel(col * 8 + x, row * 8 + y, bits & (1U << x));
      }
    }
    ++col;
  }
}

void ShieldioDisplay::render() {
  if (canvas_) {
    for (uint8_t page = 0; page < 8; ++page) {
      if (dirtyStart_[page] > dirtyEnd_[page]) continue;
      window(dirtyStart_[page], dirtyEnd_[page], page);
      uint16_t x = dirtyStart_[page];
      while (x <= dirtyEnd_[page]) {
        Wire.beginTransmission(address_);
        Wire.write((uint8_t)0x40);
        for (uint8_t n = 0; n < 24 && x <= dirtyEnd_[page]; ++n, ++x)
          Wire.write(canvas_[(uint16_t)page * 128 + x]);
        Wire.endTransmission();
      }
      dirtyStart_[page] = 128;
      dirtyEnd_[page] = 0;
    }
    invalid_ = false;
    return;
  }
  for (uint8_t row = 0; row < 8; ++row) {
    uint8_t x = 0;
    while (x < 16) {
      if (!invalid_ && cells_[row * 16 + x] == shown_[row * 16 + x]) { ++x; continue; }
      const uint8_t start = x;
      while (x < 16 && (invalid_ || cells_[row * 16 + x] != shown_[row * 16 + x])) ++x;
      window(start * 8, x * 8 - 1, row);
      for (uint8_t col = start; col < x;) {
        Wire.beginTransmission(address_);
        Wire.write((uint8_t)0x40);
        for (uint8_t n = 0; n < 3 && col < x; ++n, ++col) {
          const uint8_t pos = row * 16 + col;
          glyph((uint8_t)cells_[pos]);
          shown_[pos] = cells_[pos];
        }
        Wire.endTransmission();
      }
    }
  }
  invalid_ = false;
}

void ShieldioDisplay::logo() {
  // Text-only placeholder until official Shieldio SVG is converted and reviewed.
  clear();
  text(2, 2, "SHIELDIO");
  text(4, 4, "RED");
  render();
}

void ShieldioDisplay::loading(uint8_t percent) {
  if (percent > 100) percent = 100;
  char label[17];
  snprintf(label, sizeof(label), "Loading: %3u%%", percent);
  text(0, 6, "                ");
  text(0, 6, label);
  char bar[17];
  uint8_t filled = (uint16_t)percent * 16 / 100;
  for (uint8_t i = 0; i < 16; ++i) bar[i] = i < filled ? '#' : '-';
  bar[16] = 0;
  text(0, 7, bar);
  render();
}

void ShieldioDisplay::menu(const char* title, const char* const* items, uint8_t count, uint8_t selected) {
  clear();
  text(0, 0, title);
  text(0, 1, "----------------");
  if (count && items) {
    if (selected >= count) selected = count - 1;
    uint8_t first = selected >= 5 ? selected - 4 : 0;
    for (uint8_t i = 0; i < 5 && first + i < count; ++i) {
      uint8_t index = first + i;
      text(0, i + 2, index == selected ? ">" : " ");
      text(2, i + 2, items[index]);
    }
  }
  render();
}

void ShieldioDisplay::contrast(uint8_t value) { command(0x81); command(value); }
void ShieldioDisplay::sleep(bool enabled) { command(enabled ? 0xAE : 0xAF); }


void ShieldioCanvas::line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool on) {
  // Wide arithmetic keeps endpoint subtraction safe on 16-bit AVR.
  int32_t x = x0, y = y0;
  int32_t dx = (int32_t)x1 - x0, dy = (int32_t)y1 - y0;
  int8_t sx = dx < 0 ? -1 : 1, sy = dy < 0 ? -1 : 1;
  if (dx < 0) dx = -dx;
  if (dy < 0) dy = -dy;
  int32_t err = dx - dy;
  while (true) {
    setPixel((int16_t)x, (int16_t)y, on);
    if (x == x1 && y == y1) break;
    int32_t twice = 2 * err;
    if (twice > -dy) { err -= dy; x += sx; }
    if (twice < dx) { err += dx; y += sy; }
  }
}

void ShieldioCanvas::rect(int16_t x, int16_t y, int16_t width, int16_t height, bool filled, bool on) {
  if (width <= 0 || height <= 0) return;
  int32_t right = (int32_t)x + width - 1, bottom = (int32_t)y + height - 1;
  int16_t leftClip = x < 0 ? 0 : x, topClip = y < 0 ? 0 : y;
  int16_t rightClip = right > 127 ? 127 : (int16_t)right;
  int16_t bottomClip = bottom > 63 ? 63 : (int16_t)bottom;
  for (int16_t py = topClip; py <= bottomClip; ++py)
    for (int16_t px = leftClip; px <= rightClip; ++px)
      if (filled || px == x || px == right || py == y || py == bottom) setPixel(px, py, on);
}

void ShieldioCanvas::circle(int16_t x, int16_t y, int16_t radius, bool filled, bool on) {
  // Radius is bounded to the display diagonal; negative radii draw nothing.
  if (radius < 0 || radius > 143) return;
  int16_t dx = radius, dy = 0, err = 1 - radius;
  while (dx >= dy) {
    if (filled) {
      // Clip before narrowing coordinates to protect against off-screen centres.
      const int32_t rows[4] = {(int32_t)y + dy, (int32_t)y - dy, (int32_t)y + dx, (int32_t)y - dx};
      const int16_t spans[4] = {dx, dx, dy, dy};
      for (uint8_t i = 0; i < 4; ++i) {
        if (rows[i] < 0 || rows[i] > 63) continue;
        int32_t left = (int32_t)x - spans[i], right = (int32_t)x + spans[i];
        if (left < 0) left = 0;
        if (right > 127) right = 127;
        for (int16_t px = (int16_t)left; px <= right; ++px) setPixel(px, (int16_t)rows[i], on);
      }
    } else {
      const int32_t ox[8] = {dx, -dx, dx, -dx, dy, -dy, dy, -dy};
      const int32_t oy[8] = {dy, dy, -dy, -dy, dx, dx, -dx, -dx};
      for (uint8_t i = 0; i < 8; ++i) {
        int32_t px = (int32_t)x + ox[i], py = (int32_t)y + oy[i];
        if (px >= 0 && px < 128 && py >= 0 && py < 64) setPixel(px, py, on);
      }
    }
    ++dy;
    if (err < 0) err += 2 * dy + 1;
    else { --dx; err += 2 * (dy - dx) + 1; }
  }
}
