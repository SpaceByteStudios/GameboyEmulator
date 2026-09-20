#include <cstdint>
#include <stdbool.h>
#include <type_traits>

#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory &memory) : memory(memory) {}

void CPU::step() {
  uint8_t opcode = fetch();
  reg_pc += 1;

  execute(opcode);
}

uint8_t CPU::fetch() const { return memory.read(reg_pc); }

void CPU::execute(uint8_t opcode) {

  // Block 0
  if ((opcode & 0xC0) == 0x00) {
    // NOP
    if (opcode == 0x00) {
      return;
    }

    // ld r16, imm16
    if ((opcode & 0x0F) == 0x01) {
      uint16_t dst = (opcode >> 4) & 0x03;
      uint16_t value = memory.read16(reg_pc);
      reg_pc += 2;

      writeR16(dst, value);
    }

    // ld [r16mem], a
    if ((opcode & 0x0F) == 0x02) {
      uint16_t address = 0;

      switch (opcode >> 4) {
      case 0:
        address = reg_bc.get();
        break;
      case 1:
        address = reg_de.get();
        break;
      case 2:
        address = reg_hl.get();
        reg_hl.set(reg_hl.get() + 1);
        break;
      case 3:
        address = reg_hl.get();
        reg_hl.set(reg_hl.get() - 1);
        break;
      }

      memory.write(address, reg_af.high());
    }

    // ld a, [r16mem]
    if ((opcode & 0x0F) == 0x0A) {
      uint16_t address = 0;

      switch (opcode >> 4) {
      case 0:
        address = reg_bc.get();
        break;
      case 1:
        address = reg_de.get();
        break;
      case 2:
        address = reg_hl.get();
        reg_hl.set(reg_hl.get() + 1);
        break;
      case 3:
        address = reg_hl.get();
        reg_hl.set(reg_hl.get() - 1);
        break;
      }

      reg_af.setHigh(memory.read(address));
    }

    // ld [imm16], sp
    if (opcode == 0x08) {
      uint16_t address = memory.read16(reg_pc);
      reg_pc += 2;

      memory.write16(address, reg_sp);
    }

    // inc r16
    if ((opcode & 0x0F) == 0x03) {
      uint8_t reg = opcode >> 4;
      writeR16(reg, readR16(reg) + 1);
    }

    // dec r16
    if ((opcode & 0x0F) == 0x0B) {
      uint8_t reg = opcode >> 4;
      writeR16(reg, readR16(reg) - 1);
    }

    // add hl, r16
    if ((opcode & 0x0F) == 0x09) {
      uint8_t reg = opcode >> 4;

      uint32_t result = static_cast<uint32_t>(reg_hl.get()) +
                        static_cast<uint32_t>(readR16(reg));

      reg_hl.set(static_cast<uint16_t>(result));

      setFlag(Flag::N, false);
      setFlag(Flag::H,
              ((reg_hl.get() & 0x0FFF) + (readR16(reg) & 0x0FFF)) > 0x0FFF);
      setFlag(Flag::C, result > 0xFFFF);
    }

    // inc r8
    if ((opcode & 0x07) == 0x04) {
      uint8_t reg = opcode >> 3;
      writeR8(reg, readR8(reg) + 1);
    }

    // dec r8
    if ((opcode & 0x07) == 0x05) {
      uint8_t reg = opcode >> 3;
      writeR8(reg, readR8(reg) - 1);
    }

    // ld r8, imm8
    if ((opcode & 0x07) == 0x06) {
      uint8_t reg = opcode >> 3;

      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      writeR8(reg, value);
    }

    // rlca
    if (opcode == 0x07) {
      uint8_t a = reg_af.high();
      uint8_t last_bit = (a & 0x80) >> 7;

      setFlag(Flag::Z, false);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, last_bit);

      reg_af.setHigh((a << 1) + last_bit);
    }

    // rrca
    if (opcode == 0x0F) {
      uint8_t a = reg_af.high();
      uint8_t first_bit = a & 0x01;

      setFlag(Flag::Z, false);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, first_bit);

      reg_af.setHigh((a >> 1) + (first_bit << 7));
    }

    // rla
    if (opcode == 0x17) {
      uint8_t a = reg_af.high();
      uint8_t c = getFlag(Flag::C);
      uint8_t last_bit = (a & 0x80) >> 7;

      setFlag(Flag::Z, false);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, last_bit);

      reg_af.setHigh((a << 1) + c);
    }

    // rra
    if (opcode == 0x1F) {
      uint8_t a = reg_af.high();
      uint8_t c = getFlag(Flag::C);
      uint8_t first_bit = a & 0x01;

      setFlag(Flag::Z, false);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, first_bit);

      reg_af.setHigh((a >> 1) + (c << 7));
    }

    // daa
    if (opcode == 0x27) {
      uint8_t a = reg_af.high();
      uint8_t correction = 0;

      uint8_t n = getFlag(Flag::N);
      uint8_t h = getFlag(Flag::H);
      uint8_t c = getFlag(Flag::C);

      if (!n) {
        // Addition
        if (h || (a & 0x0F) > 0x09)
          correction |= 0x06;

        if (c || a > 0x99) {
          correction |= 0x60;
          setFlag(Flag::C, true);
        }

        a += correction;
      } else {
        // Subtraction
        if (h) {
          correction |= 0x06;
        }

        if (c) {
          correction |= 0x60;
        }

        a -= correction;
      }

      setFlag(Flag::Z, a == 0);
      setFlag(Flag::H, false);

      reg_af.setHigh(a);
    }

    // cpl
    if (opcode == 0x2F) {
      reg_af.setHigh(~reg_af.high());

      setFlag(Flag::N, true);
      setFlag(Flag::H, true);
    }

    // scf
    if (opcode == 0x37) {
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, true);
    }

    // ccf
    if (opcode == 0x3F) {
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, ~getFlag(Flag::C));
    }
  }

  // Block 1
  // 8-bit reg to reg loads
  if ((opcode & 0xC0) == 0x40) {
    // ld r8, r8
    uint8_t dst = (opcode >> 3) & 0x07;
    uint8_t src = opcode & 0x07;

    // Add halt exception

    writeR8(dst, readR8(src));
  }

  // Block 2
  // 8-bit arithmetic
  if ((opcode & 0xC0) == 0x80) {
    uint8_t reg = opcode & 0x07;

    // add a, r8
    if ((opcode & 0xF8) == 0x80) {
      add(readR8(reg), false);
    }

    // adc a, r8
    if ((opcode & 0xF8) == 0x88) {
      add(readR8(reg), true);
    }

    // sub a, r8
    if ((opcode & 0xF8) == 0x90) {
      sub(readR8(reg), false);
    }

    // sbc a, r8
    if ((opcode & 0xF8) == 0x98) {
      sub(readR8(reg), true);
    }

    // and a, r8
    if ((opcode & 0xF8) == 0xA0) {
      reg_af.setHigh(reg_af.high() & readR8(reg));
    }

    // xor a, r8
    if ((opcode & 0xF8) == 0xA8) {
      reg_af.setHigh(reg_af.high() ^ readR8(reg));
    }

    // or a, r8
    if ((opcode & 0xF8) == 0xB0) {
      reg_af.setHigh(reg_af.high() | readR8(reg));
    }

    // cp a, r8
    if ((opcode & 0xF8) == 0xB8) {
      cp(readR8(reg));
    }
  }

  // Block 3
  if ((opcode & 0xC0) == 0xC0) {
  }
}

uint8_t CPU::readR8(uint8_t r8) const {
  switch (r8) {
  case 0:
    return reg_bc.high();
  case 1:
    return reg_bc.low();
  case 2:
    return reg_de.high();
  case 3:
    return reg_de.low();
  case 4:
    return reg_hl.high();
  case 5:
    return reg_hl.low();
  case 6:
    return memory.read(reg_hl.get());
  case 7:
    return reg_af.high();
  default:
    return 0;
  }
}

void CPU::writeR8(uint8_t r8, uint8_t value) {
  switch (r8) {
  case 0:
    reg_bc.setHigh(value);
    break;
  case 1:
    reg_bc.setLow(value);
    break;
  case 2:
    reg_de.setHigh(value);
    break;
  case 3:
    reg_de.setLow(value);
    break;
  case 4:
    reg_hl.setHigh(value);
    break;
  case 5:
    reg_hl.setLow(value);
    break;
  case 6:
    memory.write(reg_hl.get(), value);
    break;
  case 7:
    reg_af.setHigh(value);
    break;
  default:
    break;
  }
}

uint16_t CPU::readR16(uint8_t r16) const {
  switch (r16) {
  case 0:
    return reg_bc.get();
  case 1:
    return reg_de.get();
  case 2:
    return reg_hl.get();
  case 3:
    return reg_sp;
  default:
    return 0;
  }
}

void CPU::writeR16(uint8_t r16, uint16_t value) {
  switch (r16) {
  case 0:
    reg_bc.set(value);
    break;
  case 1:
    reg_de.set(value);
    break;
  case 2:
    reg_hl.set(value);
    break;
  case 3:
    reg_sp = value;
    break;
  default:
    break;
  }
}

bool CPU::getFlag(Flag flag) const {
  uint8_t flags = reg_af.low();
  uint8_t mask = 1 << static_cast<uint8_t>(flag);
  return (flags & mask) != 0;
}

void CPU::setFlag(Flag flag, bool value) {
  uint8_t flags = reg_af.low();
  uint8_t mask = 1 << static_cast<uint8_t>(flag);

  if (value) {
    flags |= mask;
  } else {
    flags &= ~mask;
  }

  // Lower 4 bits of F are always zero on Game Boy
  reg_af.setLow(flags & 0xF0);
}

void CPU::add(uint8_t b, bool carry) {
  uint8_t a = reg_af.high();
  uint16_t result = static_cast<uint16_t>(a) + static_cast<uint16_t>(b) +
                    static_cast<uint16_t>(carry);

  uint8_t value = static_cast<uint8_t>(result);

  setFlag(Flag::Z, value == 0);
  setFlag(Flag::N, false);
  setFlag(Flag::H, ((a & 0x0F) + (b & 0x0F) + carry) > 0x0F);
  setFlag(Flag::C, result > 0xFF);

  reg_af.setHigh(value);
}

void CPU::sub(uint8_t b, bool carry) {
  uint8_t a = reg_af.high();
  uint16_t result = static_cast<uint16_t>(a) - static_cast<uint16_t>(b) -
                    static_cast<uint16_t>(carry);

  uint8_t value = static_cast<uint8_t>(result);

  setFlag(Flag::Z, value == 0);
  setFlag(Flag::N, true);
  setFlag(Flag::H, (a & 0x0F) < ((b & 0x0F) + carry));
  setFlag(Flag::C, static_cast<uint16_t>(a) < static_cast<uint16_t>(b) + carry);

  reg_af.setHigh(value);
}

void CPU::cp(uint8_t b) {
  uint8_t a = reg_af.high();
  uint16_t result = static_cast<uint16_t>(a) - static_cast<uint16_t>(b);

  uint8_t value = static_cast<uint8_t>(result);

  setFlag(Flag::Z, value == 0);
  setFlag(Flag::N, true);
  setFlag(Flag::H, (a & 0x0F) < ((b & 0x0F)));
  setFlag(Flag::C, static_cast<uint16_t>(a) < static_cast<uint16_t>(b));
}
