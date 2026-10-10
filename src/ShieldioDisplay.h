#pragma once
#include <Arduino.h>
#include <Wire.h>

// SSD1306 128x64 I2C, 16 columns x 8 rows, ASCII text.
class ShieldioDisplay {
public:
  explicit ShieldioDisplay(uint8_t address = 0x3C);
  void begin();
  void clear();
  void text(uint8_t col, uint8_t row, const char* value);
  void render();
  void logo();
  void loading(uint8_t percent);
  void menu(const char* title, const char* const* items, uint8_t count, uint8_t selected);
  void contrast(uint8_t value);
  void sleep(bool enabled);
  void setDimTimeout(unsigned long milliseconds = 60000UL);
  void setBrightness(uint8_t normal, uint8_t dimmed = 8);
  void wake();
  void tick();
private:
  uint8_t address_;
  char cells_[128];
  char shown_[128];
  bool invalid_;
  unsigned long dimTimeout_;
  unsigned long lastActivity_;
  uint8_t normalBrightness_;
  uint8_t dimBrightness_;
  bool dimmed_;
  void command(uint8_t value);
  void window(uint8_t start, uint8_t end, uint8_t page);
  void glyph(uint8_t code);
};