#include "registers.h"

Reg16::Reg16(uint16_t value) : value(value) {}

uint16_t Reg16::get() const { return value; }

void Reg16::set(uint16_t value) { this->value = value; }

uint8_t Reg16::low() const { return static_cast<uint8_t>(value & 0x00FF); }

uint8_t Reg16::high() const {
  return static_cast<uint8_t>((value >> 8) & 0x00FF);
}

void Reg16::setLow(uint8_t v) { value = (value & 0xFF00) | v; }

void Reg16::setHigh(uint8_t v) {
  value = (value & 0x00FF) | (static_cast<uint16_t>(v) << 8);
}
