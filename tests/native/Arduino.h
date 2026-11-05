#pragma once
#include <stdint.h>
#include <stddef.h>
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t*)(p))
inline void delay(unsigned long) {}
