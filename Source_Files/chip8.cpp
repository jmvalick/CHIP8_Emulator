// source: https://austinmorlan.com/posts/chip8_emulator/

#include <cstdint>

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
	uint16_t opcode;			//

};