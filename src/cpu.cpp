#include <cstdint>
#include <iostream>
#include <stdbool.h>

#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory &memory) : memory(memory) {
  reg_af.set(0x01B0);
  reg_bc.set(0x0013);
  reg_de.set(0x00D8);
  reg_hl.set(0x014D);

  reg_sp = 0xFFFE;
  reg_pc = 0x0100;

  IME = false;
  next_IME = false;

  halted = false;
  stopped = false;
}

uint8_t CPU::step() {
  uint8_t pending = memory.read(0xFF0F) & memory.read(0xFFFF) & 0x1F;

  if (stopped) {
    return 1;
  }

  if (halted && pending != 0) {
    halted = false;
  }

  if (halted) {
    return 4;
  }

  if (IME && pending) {
    service_pending(pending);
  }

  uint8_t opcode = fetch();

  uint8_t cycles = execute(opcode);

  if (opcode == 0xF3) {
    IME = false;
    next_IME = false;
  } else if (next_IME) {
    IME = true;
    next_IME = false;
  }

  return cycles;
}

void CPU::print_state() {
  if (halted) {
    return;
  }

  uint32_t a = reg_af.high() & 0xFF;
  uint32_t f = reg_af.low() & 0xFF;
  uint32_t b = reg_bc.high() & 0xFF;
  uint32_t c = reg_bc.low() & 0xFF;
  uint32_t d = reg_de.high() & 0xFF;
  uint32_t e = reg_de.low() & 0xFF;
  uint32_t h = reg_hl.high() & 0xFF;
  uint32_t l = reg_hl.low() & 0xFF;

  uint32_t m0 = memory.read(reg_pc + 0) & 0xFF;
  uint32_t m1 = memory.read(reg_pc + 1) & 0xFF;
  uint32_t m2 = memory.read(reg_pc + 2) & 0xFF;
  uint32_t m3 = memory.read(reg_pc + 3) & 0xFF;

  char buf[128];
  std::snprintf(buf, sizeof(buf),
                "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X "
                "SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
                a, f, b, c, d, e, h, l, reg_sp, reg_pc, m0, m1, m2, m3);

  std::cout << buf;
}

bool CPU::is_halted() { return halted; }

uint8_t CPU::fetch() {
  uint8_t opcode = memory.read(reg_pc);
  reg_pc += 1;

  return opcode;
}

uint8_t CPU::execute(uint8_t opcode) {
  // Block 0
  if ((opcode & 0xC0) == 0x00) {
    // NOP
    if (opcode == 0x00) {
      return 1;
    }

    // ld r16, imm16
    if ((opcode & 0xCF) == 0x01) {
      uint16_t dst = (opcode >> 4) & 0x03;
      uint16_t value = memory.read16(reg_pc);
      reg_pc += 2;

      writeR16(dst, value);

      return 3;
    }

    // ld [r16mem], a
    if ((opcode & 0xCF) == 0x02) {
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

      return 2;
    }

    // ld a, [r16mem]
    if ((opcode & 0xCF) == 0x0A) {
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

      return 2;
    }

    // ld [imm16], sp
    if (opcode == 0x08) {
      uint16_t address = memory.read16(reg_pc);
      reg_pc += 2;

      memory.write16(address, reg_sp);

      return 5;
    }

    // inc r16
    if ((opcode & 0xCF) == 0x03) {
      uint8_t reg = opcode >> 4;
      writeR16(reg, readR16(reg) + 1);

      return 2;
    }

    // dec r16
    if ((opcode & 0xCF) == 0x0B) {
      uint8_t reg = opcode >> 4;
      writeR16(reg, readR16(reg) - 1);

      return 2;
    }

    // add hl, r16
    if ((opcode & 0xCF) == 0x09) {
      uint8_t reg = opcode >> 4;

      uint32_t result = static_cast<uint32_t>(reg_hl.get()) +
                        static_cast<uint32_t>(readR16(reg));

      setFlag(Flag::N, false);
      setFlag(Flag::H,
              ((reg_hl.get() & 0x0FFF) + (readR16(reg) & 0x0FFF)) > 0x0FFF);
      setFlag(Flag::C, result > 0xFFFF);

      reg_hl.set(static_cast<uint16_t>(result));

      return 2;
    }

    // inc r8
    if ((opcode & 0xC7) == 0x04) {
      uint8_t reg = opcode >> 3;

      uint8_t value = readR8(reg);
      uint8_t result = value + 1;

      writeR8(reg, result);

      setFlag(Flag::Z, result == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, (value & 0x0F) == 0x0F);

      // (HL) Register
      if (reg == 0x06) {
        return 3;
      }

      return 1;
    }

    // dec r8
    if ((opcode & 0xC7) == 0x05) {
      uint8_t reg = opcode >> 3;

      uint8_t value = readR8(reg);
      uint8_t result = value - 1;

      writeR8(reg, result);

      setFlag(Flag::Z, result == 0);
      setFlag(Flag::N, true);
      setFlag(Flag::H, (value & 0x0F) == 0x00);

      // (HL) Register
      if (reg == 0x06) {
        return 3;
      }

      return 1;
    }

    // ld r8, imm8
    if ((opcode & 0xC7) == 0x06) {
      uint8_t reg = opcode >> 3;

      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      writeR8(reg, value);

      return 2;
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

      return 1;
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

      return 1;
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

      return 1;
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

      return 1;
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

      return 1;
    }

    // cpl
    if (opcode == 0x2F) {
      reg_af.setHigh(~reg_af.high());

      setFlag(Flag::N, true);
      setFlag(Flag::H, true);

      return 1;
    }

    // scf
    if (opcode == 0x37) {
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, true);

      return 1;
    }

    // ccf
    if (opcode == 0x3F) {
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, !getFlag(Flag::C));

      return 1;
    }

    // jr imm8
    if (opcode == 0x18) {
      int8_t value = memory.read(reg_pc);
      reg_pc += 1;

      reg_pc += value;

      return 3;
    }

    // jr cond, imm8
    if ((opcode & 0xE7) == 0x20) {
      int8_t value = memory.read(reg_pc);
      reg_pc += 1;

      uint8_t cond = (opcode & 0x18) >> 3;

      if (condition(cond)) {
        reg_pc += value;
        return 3;
      }

      return 2;
    }

    // stop
    if (opcode == 0x10) {
      fetch();
      stopped = true;
      memory.write(0xFF04, 0x00);

      return 1;
    }
  }

  // Block 1
  // 8-bit reg to reg loads
  if ((opcode & 0xC0) == 0x40) {

    // halt
    if (opcode == 0x76) {
      halted = true;
      return 1;
    }

    // ld r8, r8
    uint8_t dst = (opcode >> 3) & 0x07;
    uint8_t src = opcode & 0x07;

    writeR8(dst, readR8(src));

    return 1;
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
      add(readR8(reg), getFlag(Flag::C));
    }

    // sub a, r8
    if ((opcode & 0xF8) == 0x90) {
      sub(readR8(reg), false);
    }

    // sbc a, r8
    if ((opcode & 0xF8) == 0x98) {
      sub(readR8(reg), getFlag(Flag::C));
    }

    // and a, r8
    if ((opcode & 0xF8) == 0xA0) {
      reg_af.setHigh(reg_af.high() & readR8(reg));

      setFlag(Flag::Z, (reg_af.get() & 0xFF00) == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, true);
      setFlag(Flag::C, false);
    }

    // xor a, r8
    if ((opcode & 0xF8) == 0xA8) {
      reg_af.setHigh(reg_af.high() ^ readR8(reg));

      setFlag(Flag::Z, (reg_af.get() & 0xFF00) == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, false);
    }

    // or a, r8
    if ((opcode & 0xF8) == 0xB0) {
      reg_af.setHigh(reg_af.high() | readR8(reg));

      setFlag(Flag::Z, (reg_af.get() & 0xFF00) == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, false);
    }

    // cp a, r8
    if ((opcode & 0xF8) == 0xB8) {
      cp(readR8(reg));
    }

    // (HL) Register
    if (reg == 0x06) {
      return 2;
    } else {
      return 1;
    }
  }

  // Block 3
  if ((opcode & 0xC0) == 0xC0) {
    // add a, imm8
    if (opcode == 0xC6) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      add(value, false);

      return 2;
    }

    // adc a, imm8
    if (opcode == 0xCE) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      add(value, getFlag(Flag::C));

      return 2;
    }

    // sub a, imm8
    if (opcode == 0xD6) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      sub(value, false);

      return 2;
    }

    // sbc a, imm8
    if (opcode == 0xDE) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      sub(value, getFlag(Flag::C));

      return 2;
    }

    // and a, imm8
    if (opcode == 0xE6) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      reg_af.setHigh(reg_af.high() & value);

      setFlag(Flag::Z, (reg_af.get() & 0xFF00) == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, true);
      setFlag(Flag::C, false);

      return 2;
    }

    // xor a, imm8
    if (opcode == 0xEE) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      reg_af.setHigh(reg_af.high() ^ value);

      setFlag(Flag::Z, (reg_af.get() & 0xFF00) == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, false);

      return 2;
    }

    // or a, imm8
    if (opcode == 0xF6) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      reg_af.setHigh(reg_af.high() | value);

      setFlag(Flag::Z, (reg_af.get() & 0xFF00) == 0);
      setFlag(Flag::N, false);
      setFlag(Flag::H, false);
      setFlag(Flag::C, false);

      return 2;
    }

    // cp a, imm8
    if (opcode == 0xFE) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      cp(value);

      return 2;
    }

    // ret
    if (opcode == 0xC9) {
      reg_pc = memory.read16(reg_sp);
      reg_sp += 2;

      return 4;
    }

    // ret cond
    if ((opcode & 0xE7) == 0xC0) {
      uint8_t cond = (opcode & 0x18) >> 3;

      if (condition(cond)) {
        reg_pc = memory.read16(reg_sp);
        reg_sp += 2;

        return 5;
      } else {
        return 2;
      }
    }

    // reti
    if (opcode == 0xD9) {
      IME = true;

      reg_pc = memory.read16(reg_sp);
      reg_sp += 2;

      return 4;
    }

    // jp imm16
    if (opcode == 0xC3) {
      uint16_t value = memory.read16(reg_pc);
      reg_pc += 2;

      reg_pc = value;

      return 4;
    }

    // jp cond, imm16
    if ((opcode & 0xE7) == 0xC2) {
      uint16_t value = memory.read16(reg_pc);
      reg_pc += 2;

      uint8_t cond = (opcode & 0x18) >> 3;

      if (condition(cond)) {
        reg_pc = value;

        return 4;
      } else {
        return 3;
      }
    }

    // jp hl
    if (opcode == 0xE9) {
      reg_pc = reg_hl.get();

      return 1;
    }

    // call imm16
    if (opcode == 0xCD) {
      uint16_t value = memory.read16(reg_pc);
      reg_pc += 2;

      reg_sp -= 2;
      memory.write16(reg_sp, reg_pc);

      reg_pc = value;

      return 6;
    }

    // call cond, imm16
    if ((opcode & 0xE7) == 0xC4) {
      uint16_t value = memory.read16(reg_pc);
      reg_pc += 2;

      uint8_t cond = (opcode & 0x18) >> 3;

      if (condition(cond)) {
        reg_sp -= 2;
        memory.write16(reg_sp, reg_pc);

        reg_pc = value;

        return 6;
      } else {
        return 3;
      }
    }

    // rst tgt3
    if ((opcode & 0xC7) == 0xC7) {
      reg_sp -= 2;
      memory.write16(reg_sp, reg_pc);

      reg_pc = opcode & 0x38;

      return 4;
    }

    // push r16stk
    if ((opcode & 0xCF) == 0xC5) {
      uint8_t r16stk = (opcode & 0x30) >> 4;

      uint16_t value = 0;

      switch (r16stk) {
      case 0:
        value = reg_bc.get();
        break;
      case 1:
        value = reg_de.get();
        break;
      case 2:
        value = reg_hl.get();
        break;
      case 3:
        value = reg_af.get();
        break;
      }

      reg_sp -= 2;
      memory.write16(reg_sp, value);

      return 4;
    }

    // pop r16stk
    if ((opcode & 0xCF) == 0xC1) {
      uint8_t r16stk = (opcode & 0x30) >> 4;

      uint16_t value = memory.read16(reg_sp);
      reg_sp += 2;

      switch (r16stk) {
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
        reg_af.set(value & 0xFFF0);
        break;
      }

      return 3;
    }

    // 0xCB prefix
    if (opcode == 0xCB) {
      uint8_t next_opcode = fetch();

      uint8_t reg = next_opcode & 0x07;
      uint8_t bit = (next_opcode & 0x38) >> 3;

      uint8_t value = readR8(reg);

      // rlc r8
      if ((next_opcode & 0xF8) == 0x00) {
        uint8_t last_bit = (value & 0x80) >> 7;

        uint8_t result = (value << 1) + last_bit;

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, last_bit);

        writeR8(reg, result);
      }

      // rrc r8
      if ((next_opcode & 0xF8) == 0x08) {
        uint8_t first_bit = value & 0x01;

        uint8_t result = (value >> 1) + (first_bit << 7);

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, first_bit);

        writeR8(reg, result);
      }

      // rl r8
      if ((next_opcode & 0xF8) == 0x10) {
        uint8_t c = getFlag(Flag::C);
        uint8_t last_bit = (value & 0x80) >> 7;

        uint8_t result = (value << 1) + c;

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, last_bit);

        writeR8(reg, result);
      }

      // rr r8
      if ((next_opcode & 0xF8) == 0x18) {
        uint8_t c = getFlag(Flag::C);
        uint8_t first_bit = value & 0x01;

        uint8_t result = (value >> 1) + (c << 7);

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, first_bit);

        writeR8(reg, result);
      }

      // sla r8
      if ((next_opcode & 0xF8) == 0x20) {
        uint8_t last_bit = (value & 0x80) >> 7;

        uint8_t result = value << 1;

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, last_bit);

        writeR8(reg, result);
      }

      // sra r8
      if ((next_opcode & 0xF8) == 0x28) {
        uint8_t first_bit = value & 0x01;
        uint8_t last_bit = (value & 0x80);

        uint8_t result = (value >> 1) + last_bit;

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, first_bit);

        writeR8(reg, result);
      }

      // swap r8
      if ((next_opcode & 0xF8) == 0x30) {
        uint8_t upper_bits = value & 0xF0;
        uint8_t lower_bits = value & 0x0F;

        uint8_t result = (upper_bits >> 4) + (lower_bits << 4);

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, false);

        writeR8(reg, result);
      }

      // srl r8
      if ((next_opcode & 0xF8) == 0x38) {
        uint8_t first_bit = value & 0x01;

        uint8_t result = value >> 1;

        setFlag(Flag::Z, result == 0);
        setFlag(Flag::N, false);
        setFlag(Flag::H, false);
        setFlag(Flag::C, first_bit);

        writeR8(reg, result);
      }

      // bit b3, r8
      if ((next_opcode & 0xC0) == 0x40) {
        setFlag(Flag::N, false);
        setFlag(Flag::H, true);

        if ((value >> bit) & 0x01) {
          setFlag(Flag::Z, false);
        } else {
          setFlag(Flag::Z, true);
        }
      }

      // res b3, r8
      if ((next_opcode & 0xC0) == 0x80) {
        uint8_t mask = ~(0x01 << bit);
        writeR8(reg, value & mask);
      }

      // set b3, r8
      if ((next_opcode & 0xC0) == 0xC0) {
        writeR8(reg, value | (0x01 << bit));
      }

      // (HL) Register
      if (reg == 0x06) {
        return 2;
      } else {
        return 1;
      }
    }

    // ldh [c], a
    if (opcode == 0xE2) {
      uint16_t address = 0xFF00 + reg_bc.low();
      memory.write(address, reg_af.high());

      return 2;
    }

    // ldh [imm8], a
    if (opcode == 0xE0) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      uint16_t address = 0xFF00 + value;
      memory.write(address, reg_af.high());

      return 3;
    }

    // ld [imm16], a
    if (opcode == 0xEA) {
      uint16_t address = memory.read16(reg_pc);
      reg_pc += 2;

      memory.write(address, reg_af.high());

      return 4;
    }

    // ldh a, [c]
    if (opcode == 0xF2) {
      uint16_t address = 0xFF00 + reg_bc.low();
      reg_af.setHigh(memory.read(address));

      return 2;
    }

    // ldh a, [imm8]
    if (opcode == 0xF0) {
      uint8_t value = memory.read(reg_pc);
      reg_pc += 1;

      uint16_t address = 0xFF00 + value;
      reg_af.setHigh(memory.read(address));

      return 3;
    }

    // ld a, [imm16]
    if (opcode == 0xFA) {
      uint16_t address = memory.read16(reg_pc);
      reg_pc += 2;

      reg_af.setHigh(memory.read(address));

      return 4;
    }

    // add sp, imm8
    if (opcode == 0xE8) {
      int8_t value = memory.read(reg_pc);
      reg_pc += 1;

      setFlag(Flag::Z, false);
      setFlag(Flag::N, false);
      setFlag(Flag::H, (reg_sp & 0x0F) + (value & 0x0F) > 0x0F);
      setFlag(Flag::C, (reg_sp & 0xFF) + static_cast<uint8_t>(value) > 0xFF);

      reg_sp += value;
    }

    // ld hl, sp + imm8
    if (opcode == 0xF8) {
      int8_t value = memory.read(reg_pc);
      reg_pc += 1;

      uint16_t result = reg_sp + value;

      setFlag(Flag::Z, false);
      setFlag(Flag::N, false);
      setFlag(Flag::H, (reg_sp & 0x0F) + (value & 0x0F) > 0x0F);
      setFlag(Flag::C, (reg_sp & 0xFF) + static_cast<uint8_t>(value) > 0xFF);

      writeR16(0x02, result);

      return 3;
    }

    // ld sp, hl
    if (opcode == 0xF9) {
      reg_sp = readR16(0x02);

      return 2;
    }

    // ei
    if (opcode == 0xFB) {
      next_IME = true;

      return 1;
    }

    // di
    if (opcode == 0xF3) {
      next_IME = false;
      IME = false;

      return 1;
    }
  }

  return 0;
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

bool CPU::condition(uint8_t cond) const {
  switch (cond) {
  case 0:
    return !getFlag(Flag::Z);
  case 1:
    return getFlag(Flag::Z);
  case 2:
    return !getFlag(Flag::C);
  case 3:
    return getFlag(Flag::C);
  default:
    return false;
  }
}

void CPU::service_pending(uint8_t pending) {

  for (int i = 0; i < 5; i++) {
    uint8_t bit_value = (pending >> i) & 0x01;

    if (!bit_value) {
      continue;
    }

    IME = false;

    memory.write(0xFF0F, memory.read(0xFF0F) & ~(1 << i));

    uint16_t address = 0;

    switch (i) {
    case 0:
      address = 0x40;
      break;
    case 1:
      address = 0x48;
      break;
    case 2:
      address = 0x50;
      break;
    case 3:
      address = 0x58;
      break;
    case 4:
      address = 0x60;
      break;
    }

    reg_sp -= 2;
    memory.write16(reg_sp, reg_pc);

    reg_pc = address;

    break;
  }
}
