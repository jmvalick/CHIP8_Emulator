#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>
#include <chrono>

const unsigned int START_ADDRESS = 0x200;   // for interpreter reserves

const unsigned int FONTSET_SIZE = 80;       // for displayed fonts
const unsigned int FONTSET_START_ADDRESS = 0x50;

const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

class Chip8 {
public:
    Chip8();

    void LoadROM(const char* filename);
    void Cycle();

    uint8_t keypad[16]{};		// stores values of keys pressed
    uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};	// stores picture

private:
    // in bytes
    uint8_t registers[16]{};	// CPU registers (16 8-bit registers)
    uint8_t memory[4096]{};		// memory (stores interpreter reserves, ROM instructions, free space)
    uint16_t index{};			// memory index register
    uint16_t pc{};				// program counter
    uint16_t stack[16]{};		// stack of return locations in memory
    uint8_t sp{};				// stack pointer
    uint8_t delayTimer{};		//
    uint8_t soundTimer{};		//
    uint16_t opcode;			// current instruction

    typedef void (Chip8::* Chip8Func)();  // declares type alias for a pointer to a member function
    Chip8Func table[0xF + 1];
    Chip8Func table0[0xE + 1];
    Chip8Func table8[0xE + 1];
    Chip8Func tableE[0xE + 1];
    Chip8Func tableF[0x65 + 1];


    void Table0();
    void Table8();
    void TableE();
    void TableF();
    void OP_NULL();

    void OP_00E0();
    void OP_00EE();
    void OP_1nnn();
    void OP_2nnn();
    void OP_3xkk();
    void OP_4xkk();
    void OP_5xy0();
    void OP_6xkk();
    void OP_7xkk();
    void OP_8xy0();
    void OP_8xy1();
    void OP_8xy2();
    void OP_8xy3();
    void OP_8xy4();
    void OP_8xy5();
    void OP_8xy6();
    void OP_8xy7();
    void OP_8xyE();
    void OP_9xy0();
    void OP_Annn();
    void OP_Bnnn();
    void OP_Cxkk();
    void OP_Dxyn();
    void OP_Ex9E();
    void OP_ExA1();
    void OP_Fx07();
    void OP_Fx0A();
    void OP_Fx15();
    void OP_Fx18();
    void OP_Fx1E();
    void OP_Fx29();
    void OP_Fx33();
    void OP_Fx55();
    void OP_Fx65();

    // random number generator
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint16_t> randByte;
};