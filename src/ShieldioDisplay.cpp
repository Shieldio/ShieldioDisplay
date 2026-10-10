#include "ShieldioDisplay.h"
#include "ShieldioFont.h"
#include <string.h>
#include <stdio.h>

ShieldioDisplay::ShieldioDisplay(uint8_t address) : address_(address), invalid_(true), dimTimeout_(60000UL), lastActivity_(0), normalBrightness_(127), dimBrightness_(8), dimmed_(false) {
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
  clear();
  render();
  lastActivity_ = millis();
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

void ShieldioDisplay::clear() { memset(cells_, ' ', sizeof(cells_)); }

void ShieldioDisplay::text(uint8_t col, uint8_t row, const char* value) {
  if (!value || row >= 8 || col >= 16) return;
  while (*value && col < 16) cells_[row * 16 + col++] = *value++;
}

void ShieldioDisplay::render() {
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

void ShieldioDisplay::setDimTimeout(unsigned long milliseconds) {
  dimTimeout_ = milliseconds;
  wake();
}

void ShieldioDisplay::setBrightness(uint8_t normal, uint8_t dimmed) {
  normalBrightness_ = normal;
  dimBrightness_ = dimmed;
  contrast(dimmed_ ? dimBrightness_ : normalBrightness_);
}

void ShieldioDisplay::wake() {
  lastActivity_ = millis();
  if (dimmed_) {
    dimmed_ = false;
    contrast(normalBrightness_);
  }
}

void ShieldioDisplay::tick() {
  if (dimTimeout_ && !dimmed_ && (unsigned long)(millis() - lastActivity_) >= dimTimeout_) {
    dimmed_ = true;
    contrast(dimBrightness_);
  }
}
