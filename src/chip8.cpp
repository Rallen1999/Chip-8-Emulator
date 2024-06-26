#include "../headers/chip8.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

uint8_t fontset[FONTSIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};
Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
  pc = START_ADDRESS;
  for (unsigned int i = 0; i < FONTSIZE; i++) {
    memory[FONTSET_START_ADDRESS + i] = fontset[i];
  }
}

void Chip8::LoadROM(char const *filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    std::streampos size = file.tellg();
    char *buffer = new char[size];
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    for (long i = 0; i < size; i++) {
      memory[START_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;
  }
}

void Chip8::OP_00E0() {
  // clear display
  memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE() {
  // Return from a subroutine
  --sp;
  pc = stack[sp];
}

void Chip8::OP_1nnn() {
  // Jump to location nnn
  uint16_t address = opcode & 0x0FFFu;

  pc = address;
}

void Chip8::OP_2nnn() {
  // Call subroutine at nnn
  uint16_t address = opcode & 0x0FFFu;

  stack[sp] = pc;
  ++sp;
  pc = address;
}

void Chip8::OP_3xkk() {
  // Skip next instructions if Vx = kk
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  if (registers[Vx] != byte) {
    pc += 2;
  }
}

void Chip8::OP_4xkk() {
  // Skip next instructions if Vx != kk
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  if (registers[Vx] != byte) {
    pc += 2;
  }
}

void Chip8::OP_5xy0() {
  // Skip next instructions if Vx= Vy
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] == registers[Vy]) {
    pc += 2;
  }
}

void Chip8::OP_6xkk() {
  // Set Vx = kk
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  registers[Vx] = byte;
}

void Chip8::OP_7xkk() {
  // Set Vx = Vx + kk
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;
  registers[Vx] += byte;
}

void Chip8::OP_8xy0() {
  // Set Vx = Vy
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1() {
  // Set Vx = Vx or Vy
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2() {
  // Set Vx = Vx AND Vy
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3() {
  // Set Vx = Vx XOR Vy
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4() {
  // Set Vx = Vx + Vy. Set VF = carry
  // If Sum is greater than 255 (8 bits/1 byte) set VF = 1, else, set VF = 0
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  uint16_t sum = registers[Vx] + registers[Vy];

  if (sum > 255u) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }
  registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5() {
  // Set Vx = Vx - Vy. Set VF = NOT borrow
  // If Vx > Vy, set VF = 1 else, set VF = 0
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] > registers[Vy]) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }
  registers[Vx] -= registers[Vy];
}
