#include "chip8.h"

// source: https://austinmorlan.com/posts/chip8_emulator/

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


// constructor
Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
	// initialize random random num generator
	randByte = std::uniform_int_distribution<uint16_t>(0, 255U);

	// copy font data to memory
	pc = START_ADDRESS;
	memcpy(memory + START_ADDRESS, fontset, FONTSET_SIZE * sizeof(fontset[0]));

	// set up function pointer table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	for (size_t i = 0; i <= 0xE; i++) {
		table0[i] = &Chip8::OP_NULL;
		table8[i] = &Chip8::OP_NULL;
		tableE[i] = &Chip8::OP_NULL;
	}

	table[0x0] = &Chip8::OP_00E0;
	table[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &Chip8::OP_NULL;
	}

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}

// function invocation
void Chip8::Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF()
{
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL()
{}

	
// Instructions

// 00E0: CLS
// clear the display
void Chip8::OP_00E0() {
	memset(video, 0, sizeof(video));
}

// 00EE: RET
// return from a subroutine
void Chip8::OP_00EE() {
	--sp;
	pc = stack[sp];
}

// 1nnn: JP addr
// jump to location nnn
void Chip8::OP_1nnn() {
	uint16_t address = opcode & 0x0FFFu;

	pc = address;
}

// 2nn: CALL addr
// call subroutine at nnn
void Chip8::OP_2nnn() {
	uint16_t address = opcode & 0x0FFFu;

	stack[sp] = pc;
	++sp;
	pc = address;
}

// 3xkk: SE Vx, byte
// skip next instruction if Vx = kk
void Chip8::OP_3xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte) {
		pc += 2;
	}
}

// 4xkk: SE Vx, byte
// skip next instruction if Vx != kk
void Chip8::OP_4xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte) {
		pc += 2;
	}
}

// 5xy0: SE Vx, Vy
// skip next instruction if Vx = Vy
void Chip8::OP_5xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy]) {
		pc += 2;
	}
}

// 6xkk: LD Vx, byte
// set Vx = kk
void Chip8::OP_6xkk() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FFU;

	registers[Vx] = byte;
}

// 7xkk: ADD Vx, byte
// set Vx = Vx + kk
void Chip8::OP_7xkk() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FFU;

	registers[Vx] += byte;
}

// 8xkk: LD Vx, Vy
// set Vx = Vy
void Chip8::OP_8xy0() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	registers[Vx] = registers[Vy];
}

// 8xy1: OR Vx, Vy
// set Vx = Vx OR Vy
void Chip8::OP_8xy1() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	registers[Vx] |= registers[Vy];
}

// 8xy2: AND Vx, Vy
// set Vx = Vx AND Vy
void Chip8::OP_8xy2() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	registers[Vx] &= registers[Vy];
}

// 8xy2: XOR Vx, Vy
// set Vx = Vx XOR Vy
void Chip8::OP_8xy3() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	registers[Vx] ^= registers[Vy];
}

// 8xy4: ADD Vx, Vy
// set Vx = Vx + Vy, set VF = carry
void Chip8::OP_8xy4() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	registers[0xF] = (sum & 0b0000000100000000) >> 8u;
	registers[Vx] = sum & 0xFFu;
}

// 8xy5: SUB Vx, Vy
// set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::OP_8xy5() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	registers[0xF] = (registers[Vx] > registers[Vy]);
	registers[Vx] -= registers[Vy];
}

// 8xy6: SHR Vx
// set Vx = Vx SHR 1  
void Chip8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0F000u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

// 8xy7: SUBN Vx, Vy
// set Vx = Vy - Vx, set VF = Not borrow
void Chip8::OP_8xy7() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	registers[0xF] = (registers[Vy] > registers[Vx]);
	registers[Vx] = registers[Vy] - registers[Vx];
}

// 8xyE: SHL Vx {, Vy}
// set Vx = Vx SHL 1
void Chip8::OP_8xyE() {
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	registers[0xF] = (registers[Vx] & 0x8u);

	registers[Vx] <<= 1;
}

// 9xy0: SNE Vx, Vy
// skip next instruction if Vx != Vy
void Chip8::OP_9xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy]) {
		pc += 2;
	}
}

// Annn: LD I, addr
// set I = nnn
void Chip8::OP_Annn() {
	uint16_t address = opcode & 0x0FFFu;

	index = address;
}

// Bnnn: JP V0, addr
// jump to location nnn + V0
void Chip8::OP_Bnnn() {
	uint16_t address = opcode & 0x0FFFu;

	pc = registers[0] + address;
}

// Cxkk: RND Vx, byte
// set Vx = random byte AND kk
void Chip8::OP_Cxkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = static_cast<uint8_t>(randByte(randGen) & 0x00FF) & byte;
}

// Dxyn: DRW Vx, Vy, nibble
// display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void Chip8::OP_Dxyn() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index + row];
		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			if (spritePixel) {
				if (*screenPixel == 0xFFFFFFFF) {
					registers[0xF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

// Ex9E: SKP Vx
// skip next instruction if key with the value of Vx is pressed
void Chip8::OP_Ex9E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		pc += 2;
	}
}

// ExA1: SKNP Vx
// skip next instruction if key with the value of Vx is not pressed
void Chip8::OP_ExA1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		pc += 2;
	}
}

// Fx07: LD Vx, DT
// set Vx = delay timer value
void Chip8::OP_Fx07() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delayTimer;
}

// Fx0A: LD Vx, k
// wait for a key press, store the value of the key in Vx
void Chip8::OP_Fx0A() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (int i = 0; i < 16; ++i) {
		if (keypad[i]) {
			registers[Vx] = i;
			return;
		}
	}

	pc -= 2;
}

// Fx15: LD DT, Vx
// set delay timer = Vx
void Chip8::OP_Fx15() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delayTimer = registers[Vx];
}

// Fx18: LD ST, Vx
// set sound timer = Vx
void Chip8::OP_Fx18() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	soundTimer = registers[Vx];
}

// Fx1E: ADD I, Vx
// set I = I + Vx
void Chip8::OP_Fx1E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}

// Fx29: LD F, Vx
// set I = location of sprite for digit Vx
void Chip8::OP_Fx29() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index = FONTSET_START_ADDRESS + (5 * digit);
}

// Fx33: LD B, Vx
// store BCD representation of Vx in memory location I, I+1 ,and I+2
void Chip8::OP_Fx33() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	memory[index + 2] = value % 10;
	value /= 10;

	memory[index + 1] = value % 10;
	value /= 10;

	memory[index] = value % 10;
}

// Fx55: LD [I], Vx
// store registers V0 through Vx in memory starting at location I
void Chip8::OP_Fx55() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		memory[index + i] = registers[i];
	}
}

// Fx65: LD Vx, [i]
// read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		registers[i] = memory[index + i];
	}
}

// load ROM file
void Chip8::LoadROM(char const* filename) {

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


// Fetch, Decode, Execute Cylce
void Chip8::Cycle() {
	// fetch
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	// increment PC
	pc += 2;

	// decode and execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();  // basically Chip8.function()

	// decrement the delay timer if it is set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// decrement the sound timer if it is set
	if (soundTimer > 0)
	{
		--soundTimer;
	}

}