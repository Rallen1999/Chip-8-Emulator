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

void Chip8::OP_8xy6() {
  // Set Vx = Vx SHR 1
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // Save LSB in VF
  registers[0xF] = (registers[Vx] & 0x1u);

  registers[Vx] >>= 1;
}

void Chip8::OP_8xy7() {
  // Set Vx = Vy - Vx, set VF = not borrow
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vy] > registers[Vx]) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }

  registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE() {
  // Shift Left
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

  registers[Vx] <<= 1;
}

void Chip8::OP_9xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] != registers[Vy]) {
    pc += 2;
  }
}

void Chip8::OP_Annn() {
  // Set I = nnn
  uint16_t address = opcode & 0x0FFFu;

  index = address;
}

void Chip8::OP_Bnnn() {
  // Jump to location nnn + V0
  uint16_t address = opcode & 0x0FFFu;

  pc = registers[0] + address;
}

void Chip8::OP_Cxkk() {
  // Set Vx = random byte AND kk
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn() {
  // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF =
  // collision
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;
  uint8_t height = opcode & 0x00F0u;

  uint8_t xPos = registers[Vx] % DISPLAY_Width;
  uint8_t yPos = registers[Vy] % DISPLAY_Height;

  registers[0xF] = 0;

  for (unsigned int row = 0; row < height; ++row) {
    uint8_t spriteByte = memory[index + row];

    for (unsigned int col = 0; col < 8; ++col) {
      uint8_t spritePixel = spriteByte & (0x80u >> col);
      uint32_t *screenPixel =
          &video[(yPos + row) * DISPLAY_Width + (xPos + col)];

      if (spritePixel) {
        if (*screenPixel == 0xFFFFFFFF) {
          registers[0xF] = 1;

          *screenPixel ^= 0xFFFFFFFF;
        }
      }
    }
  }
}

void Chip8::OP_Ex9E() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  uint8_t key = registers[Vx];

  if (keypad[key]) {
    pc += 2;
  }
}

void Chip8::OP_ExA1() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  uint8_t key = registers[Vx];

  if (!keypad[key]) {
    pc += 2;
  }
}

void Chip8::OP_Fx07() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  if (keypad[0]) {
    registers[Vx] = 0;
  } else if (keypad[1]) {
    registers[Vx] = 1;
  } else if (keypad[2]) {
    registers[Vx] = 2;
  } else if (keypad[3]) {
    registers[Vx] = 3;
  } else if (keypad[4]) {
    registers[Vx] = 4;
  } else if (keypad[5]) {
    registers[Vx] = 5;
  } else if (keypad[6]) {
    registers[Vx] = 6;
  } else if (keypad[7]) {
    registers[Vx] = 7;
  } else if (keypad[8]) {
    registers[Vx] = 8;
  } else if (keypad[9]) {
    registers[Vx] = 9;
  } else if (keypad[10]) {
    registers[Vx] = 10;
  } else if (keypad[11]) {
    registers[Vx] = 11;
  } else if (keypad[12]) {
    registers[Vx] = 12;
  } else if (keypad[13]) {
    registers[Vx] = 13;
  } else if (keypad[14]) {
    registers[Vx] = 14;
  } else if (keypad[15]) {
    registers[Vx] = 15;
  } else {
    pc -= 2;
  }
}

void Chip8::OP_Fx15() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  delayTimer = registers[Vx];
}

void Chip8::OP_Fx18() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  index += registers[Vx];
}

void Chip8::OP_Fx29() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t digit = registers[Vx];

  index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx55() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  for (uint8_t i = 0; i <= Vx; ++i) {
    memory[index + i] = registers[i];
  }
}

void Chip8::OP_Fx65() {
  uint8_t Vx = (opcode & 0x0F00U) >> 8U;
  for (uint8_t i = 0; i <= Vx; i++) {
    registers[i] = memory[index + i];
  }
}

void Chip8::OP_Fx33() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t value = registers[Vx];

  memory[index + 2] = value % 10;
  value /= 10;

  memory[index + 1] = value % 10;
  value /= 10;

  memory[index] = value % 10;
}
