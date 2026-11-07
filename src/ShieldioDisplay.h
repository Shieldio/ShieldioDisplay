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
  enum class Animal : uint8_t { Cat, Dog, Lion };
  enum class Mood : uint8_t { Happy, Sad, Surprised, Sleepy, Angry };
  // Replaces the screen immediately. Uses a 128-byte page, no full canvas.
  void face(Animal animal = Animal::Cat, Mood mood = Mood::Happy, bool blink = false);
private:
  uint8_t address_;
  char cells_[128];
  char shown_[128];
  bool invalid_;
protected:
  // The caller owns the optional 1024-byte canvas; text-only programs keep the small buffers.
  void useCanvas(uint8_t* buffer);
  void setPixel(int16_t x, int16_t y, bool on);
  uint8_t* canvas_;
  uint8_t dirtyStart_[8], dirtyEnd_[8];
private:
  void command(uint8_t value);
  void window(uint8_t start, uint8_t end, uint8_t page);
  void glyph(uint8_t code);
};

// Choose ShieldioCanvas only when drawing shapes. No heap allocation.
class ShieldioCanvas : public ShieldioDisplay {
public:
  explicit ShieldioCanvas(uint8_t address = 0x3C) : ShieldioDisplay(address) { useCanvas(buffer_); }
  void pixel(int16_t x, int16_t y, bool on = true) { setPixel(x, y, on); }
  void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool on = true);
  void rect(int16_t x, int16_t y, int16_t width, int16_t height, bool filled = false, bool on = true);
  void circle(int16_t x, int16_t y, int16_t radius, bool filled = false, bool on = true);
private:
  uint8_t buffer_[1024];
};

