#pragma once

#include <cstdint>

class Reg16 {
public:
  Reg16() = default;
  explicit Reg16(uint16_t value);

  uint16_t get() const;
  void set(uint16_t value);

  uint8_t low() const;
  uint8_t high() const;

  void setLow(uint8_t value);
  void setHigh(uint8_t value);

private:
  uint16_t value = 0;
};
