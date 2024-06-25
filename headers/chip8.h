#pragma once
#include <cstdint>
#include <random>

const unsigned int KEY_COUNT = 16;
const unsigned int DISPLAY_Width = 64;
const unsigned int DISPLAY_Height = 32;
const unsigned int MEMORY = 4096;
const unsigned int STACK = 16;
const unsigned int REGISTERS = 16;

class Chip8 {
public:
  Chip8();
  void LoadROM(char const *filename);
  uint8_t registers[REGISTERS]{};
  uint8_t memory[MEMORY]{};
  uint16_t index{};
  uint16_t pc{};
  uint16_t stack[STACK]{};
  uint8_t sp{};
  uint8_t delayTimer{};
  uint8_t soundTimer{};
  uint8_t keypad[KEY_COUNT]{};
  uint32_t video[DISPLAY_Height * DISPLAY_Width]{};
  uint16_t opcode;

private:
  std::default_random_engine randGen;
  std::uniform_int_distribution<uint8_t> randByte;

  void OP_00E0();
  void OP_00EE();
  void OP_3xkk();
  void OP_2nnn();
  void OP_4xkk();
  void OP_5xy0();
};
