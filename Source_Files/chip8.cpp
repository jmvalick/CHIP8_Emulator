// source: https://austinmorlan.com/posts/chip8_emulator/

#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>


const unsigned int START_ADDRESS = 0x200;	// for interpreter reserves

const unsigned int FONTSET_SIZE = 80;		// for displayed fonts
const unsigned int FONTSET_START_ADDRESS = 0x50;
uint8_t fontset[FONTSET_SIZE] =
{
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
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


class Chip8
{
public:
	// in bytes
	uint8_t registers[16]{};	// CPU registers (16 8-bit registers)
	uint8_t memory[4096]{};		// memory (stores interpreter reserves, ROM instructions, free space)
	uint16_t index{};			// memory index register
	uint16_t pc{};				// program counter
	uint16_t stack[16]{};		// stack of return locations in memory
	uint8_t sp{};				// stack pointer
	uint8_t delayTimer{};		//
	uint8_t soundTimer{};		//
	uint8_t keypad[16]{};		//
	uint8_t video[64 * 32]{};	//
	uint16_t opcode;			// current instruction


	// constructor
	Chip8() {
		pc = START_ADDRESS;

		memcpy(memory + START_ADDRESS, fontset, FONTSET_SIZE * sizeof(fontset[0]));
	}


	// random number generator
	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;
	Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
		randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
	}

	// load ROM file
	void LoadROM(char const* filename){

		std::ifstream file(filename, std::ios::binary | std::ios::ate);  // open file, go to end

		if (file.is_open()) {
			std::streampos size = file.tellg();		// get size
			char* buffer = new char[size];			// create buffer

			file.seekg(0, std::ios::beg);	// go to beginning
			file.read(buffer, size);		// copy file to buffer
			file.close();					// close file

			std::memcpy(memory + START_ADDRESS, buffer, size * sizeof(buffer[0]));  // copy to chip8 memory

			delete[] buffer;	// clear buffer
		}
	}


	// Instructions

	// 00E0: CLS
	// clear the display
	void OP_00E0() {
		memset(video, 0, sizeof(video));
	}

	// 00EE: RET
	// return from a subroutine
	void OP_00EE() {
		--sp;
		pc = stack[sp];
	}
	
	// 1nnn: JP addr
	// jump to location nnn
	void OP_1nnn() {
		uint16_t address = opcode & 0x0FFFu;

		pc = address;
	}

	// 2nn: CALL addr
	// call subroutine at nnn
	void OP_2nnn() {
		uint16_t address = opcode & 0x0FFFu;

		stack[sp] = pc;
		++sp;
		pc = address;
	}

	// 3xkk: SE Vx, byte
	// skip next instruction if Vx = kk
	void OP_3xkk() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	}

};