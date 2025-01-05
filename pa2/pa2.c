/**********************************************************************
 * Copyright (c) 2019-2024
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

 /*====================================================================*/
 /*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */

 /* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_COMMAND		256 /* Maximum length of command string */

/**
 * memory[] emulates the memory of the machine
 */
static unsigned char memory[1 << 20] = {	/* 1MB memory at 0x0000 0000 -- 0x0100 0000 */
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00,
	'h',  'e',  'l',  'l',  'o',  ' ',  'w',  'o',
	'r',  'l',  'd',  '!',  '!',  0x00, 0x00, 0x00,
	'a',  'w',  'e',  's',  'o',  'm',  'e',  ' ',
	'c',  'o',  'm',  'p',  'u',  't',  'e',  'r',
	' ',  'a',  'r',  'c',  'h',  'i',  't',  'e',
	'c',  't',  'u',  'r',  'e',  '.',  0x00, 0x00,
};

#define ENTRY_PC	0x1000	/* Initial value for PC register */
#define INITIAL_SP	0x8000	/* Initial location for stack pointer */

/**
 * Registers of the machine
 */
static unsigned int registers[32] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x10, ENTRY_PC, 0x20, 3, 0xbadacafe, 0xcdcdcdcd, 0xffffffff, 7,
	0, 0, 0, 0, 0, INITIAL_SP, 0, 0,
};

/**
 * Names of the registers. Note that $zero is shorten to zr
 */
const char* register_names[] = {
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

/**
 * Arithmetic registers
 */
static unsigned int hi = 0xbeef500d;
static unsigned int lo = 0xcdcdcdcd;

/**
 * Program counter register
 */
static unsigned int pc = ENTRY_PC;

static unsigned int translate(int, char* []);
static bool process_instruction(unsigned int);
static unsigned int load_program(unsigned int, char* const);
static void run_program(void);

static void __show_registers(char* const register_name)
{
	int from = 0, to = 0;
	bool include_pc = false;
	bool include_hi = false;
	bool include_lo = false;

	if (strcmp(register_name, "all") == 0) {
		from = 0;
		to = 32;
		include_pc = true;
		include_hi = true;
		include_lo = true;
	}
	else if (strcmp(register_name, "pc") == 0) {
		include_pc = true;
	}
	else if (strcmp(register_name, "hi") == 0) {
		include_hi = true;
	}
	else if (strcmp(register_name, "lo") == 0) {
		include_lo = true;
	}
	else {
		for (int i = 0; i < sizeof(register_names) / sizeof(*register_names); i++) {
			if (strcmp(register_name, register_names[i]) == 0) {
				from = i;
				to = i + 1;
			}
		}
	}

	for (int i = from; i < to; i++) {
		fprintf(stderr, "[%02d:%2s] 0x%08x    %u\n", i, register_names[i], registers[i], registers[i]);
	}
	if (include_pc) {
		fprintf(stderr, "[  pc ] 0x%08x\n", pc);
	}
	if (include_hi) {
		fprintf(stderr, "[  hi ] 0x%08x    %u\n", hi, hi);
	}
	if (include_lo) {
		fprintf(stderr, "[  lo ] 0x%08x    %u\n", lo, lo);
	}
}

static void __dump_memory(unsigned int addr, size_t length)
{
	for (size_t i = 0; i < length; i += 4) {
		fprintf(stderr, "0x%08lx:  %02x %02x %02x %02x    %c %c %c %c\n",
			addr + i,
			memory[addr + i], memory[addr + i + 1],
			memory[addr + i + 2], memory[addr + i + 3],
			isprint(memory[addr + i]) ? memory[addr + i] : '.',
			isprint(memory[addr + i + 1]) ? memory[addr + i + 1] : '.',
			isprint(memory[addr + i + 2]) ? memory[addr + i + 2] : '.',
			isprint(memory[addr + i + 3]) ? memory[addr + i + 3] : '.');
	}
}

static void __process_command(int argc, char* argv[])
{
	if (argc == 0) return;

	if (strcmp(argv[0], "load") == 0) {
		if (argc == 3) {
			unsigned int nr_insts = load_program(strtoumax(argv[1], NULL, 0), argv[2]);
			fprintf(stderr, "Loaded %u instructions\n", nr_insts);
		}
		else {
			printf("Usage: load [start address] [program filename]\n");
		}
	}
	else if (strcmp(argv[0], "run") == 0) {
		if (argc == 1) {
			run_program();
		}
		else {
			printf("Usage: run\n");
		}
	}
	else if (strcmp(argv[0], "show") == 0) {
		if (argc == 1) {
			__show_registers("all");
		}
		else if (argc == 2) {
			__show_registers(argv[1]);
		}
		else {
			printf("Usage: show { [register name] }\n");
		}
	}
	else if (strcmp(argv[0], "dump") == 0) {
		if (argc == 3) {
			__dump_memory(strtoumax(argv[1], NULL, 0), strtoumax(argv[2], NULL, 0));
		}
		else {
			printf("Usage: dump [start address] [length]\n");
		}
	}
	else {
		unsigned int instr = strtoumax(argv[0], NULL, 0);

		if (!instr) {
			instr = translate(argc, argv);
		}
		process_instruction(instr);
	}
}

static int __parse_command(char* command, int* nr_tokens, char* tokens[])
{
	char* curr = command;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0') {
		if (isspace(*curr)) {
			*curr = '\0';
			token_started = false;
		}
		else {
			if (!token_started) {
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}
		curr++;
	}

	/* Exclude comments from tokens */
	for (int i = 0; i < *nr_tokens; i++) {
		if (strncmp(tokens[i], "//", 2) == 0 || strncmp(tokens[i], "#", 1) == 0) {
			*nr_tokens = i;
			tokens[i] = NULL;
		}
	}

	return 0;
}

int main(int argc, char* const argv[])
{
	char command[MAX_COMMAND] = { '\0' };
	FILE* input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[1]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin) {
		printf("*****************************************************\n");
		printf("           SCE212 MIPS Termlink v0.24-S\n");
		printf("\n");
		printf("\n");
		printf(">> ");
	}

	while (fgets(command, sizeof(command), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		for (size_t i = 0; i < strlen(command); i++) {
			command[i] = tolower(command[i]);
		}

		if (__parse_command(command, &nr_tokens, tokens) < 0)
			continue;

		__process_command(nr_tokens, tokens);

		if (input == stdin) printf(">> ");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}

/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/**********************************************************************
 * translate(nr_tokens, tokens[])
 *
 * DESCRIPTION
 *   Translate the given assembly to a MIPS instruction. This is optional
 *   so you may copy-paste your translation feature from PA1 or just leave
 *   it as is.
 *
 * RETURN VALUE
 *   MIPS instruction for the given assembly
 *   0 if translation is not possible
 */
static unsigned int translate(int nr_tokens, char* tokens[])
{
	return 0;
}


/**********************************************************************
 * process_instruction
 *
 * DESCRIPTION
 *   Execute the machine code given through @instr. The following table lists
 *   up the instructions to support. Note that the instrunctions with '*' are
 *   newly added to PA2.
 *
 * | Name   | Format    | Opcode / opcode + funct |
 * | ------ | --------- | ----------------------- |
 * | `add`  | r-format  | 0 + 0x20                |
 * | `addi` | i-format  | 0x08                    |
 * | `sub`  | r-format  | 0 + 0x22                |
 * | `and`  | r-format  | 0 + 0x24                |
 * | `andi` | i-format  | 0x0c                    |
 * | `or`   | r-format  | 0 + 0x25                |
 * | `ori`  | i-format  | 0x0d                    |
 * | `nor`  | r-format  | 0 + 0x27                |
 * | `sll`  | r-format  | 0 + 0x00                |
 * | `srl`  | r-format  | 0 + 0x02                |
 * | `sra`  | r-format  | 0 + 0x03                |
 * | `lw`   | i-format  | 0x23                    |
 * | `sw`   | i-format  | 0x2b                    |
 * | `slt`  | r-format* | 0 + 0x2a                |
 * | `slti` | i-format* | 0x0a                    |
 * | `beq`  | i-format* | 0x04                    |
 * | `bne`  | i-format* | 0x05                    |
 * | `jr`   | r-format* | 0 + 0x08                |
 * | `j`    | j-format* | 0x02                    |
 * | `jal`  | j-format* | 0x03                    |
 *
 * RETURN VALUE
 *   true if successfully processed the instruction.
 *   false if @instr is unknown instructions
 */
static bool process_instruction(unsigned int instr) {
	unsigned int opcode = instr >> 26; //opcode 추출
	unsigned int funct = instr & 0x3F; //funct 추출

	if (opcode == 0x00) { // R-format 명령어

		switch (funct) {
		case 0x20: // add
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = registers[rs] + registers[rt];
			break;
		}
		case 0x22: // sub
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = registers[rs] - registers[rt];
			break;
		}
		case 0x24:	//and
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = registers[rs] & registers[rt];
			break;
		}
		case 0x25:	//or
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = registers[rs] | registers[rt];
			break;
		}
		case 0x27:	//nor
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = ~(registers[rs] | registers[rt]);
			break;
		}
		case 0x00:	//sll
		{
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			unsigned int shamt = (instr >> 6) & 0x1F;	//shamt 추출
			registers[rd] = registers[rt] << shamt;	//왼쪽으로 쉬프트
			break;
		}
		case 0x02:	//srl
		{
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			unsigned int shamt = (instr >> 6) & 0x1F;	//shamt 추출
			registers[rd] = registers[rt] >> shamt;	//오른쪽으로 쉬프트
			break;
		}
		case 0x03:	//sra
		{
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			unsigned int shamt = (instr >> 6) & 0x1F;	//shamt 추출
			registers[rd] = (int)registers[rt] >> shamt;	//오른쪽으로 산술쉬프트
			break;
		}
		case 0x2a:	//slt
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			if (((int)registers[rs] & 0x80000000) != ((int)registers[rt] & 0x80000000)) { //부호가 다른 경우
				if ((int)registers[rs] & 0x80000000) {	//rs가 음수이면 더작음
					registers[rd] = 1;
				}
				else {
					registers[rd] = 0;
				}
			}
			else { //부호가 같은 경우
				if ((int)registers[rs] < (int)registers[rt]) {
					registers[rd] = 1;
				}
				else {
					registers[rd] = 0;
				}
			}
			break;
		}
		case 0x18:	//mult
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int64_t result = (int64_t)registers[rs] * (int64_t)registers[rt];	//64비트로 확장
			hi = (unsigned int)(result >> 32);	//MSB 저장
			lo = (unsigned int)result;	//LSB 저장
			break;
		}
		case 0x10:	//mfhi
		{
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = hi;
			break;
		}
		case 0x12:	//mflo
		{
			unsigned int rd = (instr >> 11) & 0x1F;	//rd 추출
			registers[rd] = lo;
			break;
		}
		case 0x08:	//jr
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			pc = registers[rs];  //PC <= rs
			break;
		}
		default:
		{
			printf("없는 명령어 입력함\n");
			return false;
		}
		}
		return true;
	}
	else {	//I, J format 명령어
		switch (opcode) {
		case 0x08: //addi
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			short cons = (short)(instr & 0xFFFF);	//con 추출
			registers[rt] = registers[rs] + (int)cons;  // rs와 상수 값을 더하여 rt에 저장
			break;
		}
		break;
		case 0x0c: //andi
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int cons = (int)(instr & 0xFFFF);	//con 추출
			registers[rt] = registers[rs] & cons;
			break;
		}
		case 0x0d: //ori
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int cons = (int)(instr & 0xFFFF);	//con 추출
			registers[rt] = registers[rs] | cons;
			break;
		}
		case 0x23: //lw
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			unsigned int offset = instr & 0xFFFF;	//offset 추출
			unsigned address = registers[rs] + offset;
			int data = (memory[address] << 24) | (memory[address + 1] << 16) | (memory[address + 2] << 8) | memory[address + 3];
			registers[rt] = data;
			break;
		}
		case 0x2b: //sw
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int offset = (instr & 0xFFFF);	//offset 추출
			if (offset & 0x8000) {	//offset이 음수일 경우
				offset |= 0xFFFF0000;
			}
			// 메모리에 데이터 저장
			memory[registers[rs] + offset] = (registers[rt] >> 24) & 0xFF;	//상위 8비트
			memory[registers[rs] + offset + 1] = (registers[rt] >> 16) & 0xFF;	//다음 8비트
			memory[registers[rs] + offset + 2] = (registers[rt] >> 8) & 0xFF;	//다음 8비트
			memory[registers[rs] + offset + 3] = registers[rt] & 0xFF;	//하위 8비트
			break;
		}
		case 0x04: //beq
		{
			unsigned int rs = (instr >> 21) & 0x1F;    //rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;    //rt 추출
			int offset = (instr & 0xFFFF);             //오프셋 추출
			if (offset & 0x8000) {	//음수일 경우
				offset = offset | 0xFFFF0000;
			}
			if (registers[rs] == registers[rt]) {
				pc = pc + (offset << 2); //점프
			}
			break;
		}
		case 0x05: //bne
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int offset = (instr & 0xFFFF);	//오프셋 추출
			if (offset & 0x8000) {	//음수일 경우
				offset = offset | 0xFFFF0000;
			}
			if (registers[rs] != registers[rt]) {
				pc = pc + (offset << 2); //점프
			}
			break;
		}
		case 0x0a: //slti
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int cons = (int)(instr & 0xFFFF);	//cons 추출
			if (cons & 0x8000) {	//음수일 경우
				cons |= 0xFFFF0000;
			}
			if ((int)registers[rs] < cons) {
				registers[rt] = 1;
			}
			else {
				registers[rt] = 0;
			}
			break;
		}
		case 0x24: //lbu
		{
			unsigned int rs = (instr >> 21) & 0x1F;	//rs 추출
			unsigned int rt = (instr >> 16) & 0x1F;	//rt 추출
			int offset = (instr & 0xFFFF);	//오프셋 추출
			if (offset & 0x8000) {
				offset |= 0xFFFF0000;
			}
			registers[rt] = memory[registers[rs] + offset];
			break;
		}
		case 0x02: //j
		{
			unsigned int ta = instr & 0x03FFFFFF; //하위 26비트 추출
			pc = (pc & 0xF0000000) | (ta << 2); //상위 4비트는 현재 PC의 상위 4비트와 동일하게 유지되어야 함
			break;
		}
		case 0x03: //jal
		{
			unsigned int ta = instr & 0x03FFFFFF; //하위 26비트 추출
			registers[31] = pc; //ra <- pc
			pc = (pc & 0xF0000000) | (ta << 2);	//상위 4비트는 현재 PC의 상위 4비트와 동일하게 유지되어야 함
			break;
		}
		case 0x3f: //halt
		{
			return false;
		}
		default:
		{
			return false;
		}
		}
		return true;
	}
	return false;
}

/**********************************************************************
 * load_program(start_addr, filename)
 *
 * DESCRIPTION
 *   Load the instructions in the file @filename onto the memory starting at
 *   @start_addr. Each line in the program file looks like;
 *
 *	 [MIPS instruction with 0x as a prefix]  // or # optional comments
 *
 *   For example,
 *
 *   0x8c090008
 *   0xac090020	// sw t1, zero + 32
 *   0x8c080000 # this is also a comment
 *
 *   implies three MIPS instructions to load. Each machine instruction may
 *   be followed by comments like the second instruction. Hint: that you can
 *   simply call strtoumax(linebuffer, NULL, 0) to read the machine code while
 *   ignoring the comment parts.
 *
 *	 Refer to the @main() for reading data from files. (fopen, fgets, fclose).
 *
 * RETURN
 *	 Number of successfully loaded instructions
 *
 */
static unsigned int load_program(unsigned int start_addr, char* const filename)
{
	FILE* file = fopen(filename, "r"); //파일 열기
	unsigned int addr = start_addr; //시작 주소 설정
	char line[MAX_COMMAND];

	while (fgets(line, sizeof(line), file)) {
		unsigned int instr = strtoul(line, NULL, 16); //명령어 16진수로 변환
		instr = ((instr & 0xFF) << 24) | ((instr & 0xFF00) << 8) | ((instr >> 8) & 0xFF00) | ((instr >> 24) & 0xFF);	//빅엔디안 방식 저장을 위한 비트마스킹
		*(unsigned int*)&memory[addr] = instr;
		addr += 4; //메모리 다음줄 이동
	}
	fclose(file); //파일 닫기
	return (addr - start_addr) / 4;	//입력된 명령어 개수 반환
}

/**********************************************************************
 * run_program
 *
 * DESCRIPTION
 *   Start running the program that is loaded by @load_program function above.
 *   If you implement @load_program() properly, the first instruction is placed
 *   at @ENTRY_PC. Using @pc, which is the program counter of this processor,
 *   you can emulate the MIPS processor by
 *
 *   1. Read instruction from @pc
 *   2. Increment @pc by 4
 *   3. Call @process_instruction(instruction)
 *   4. Repeat until @process_instruction() returns 0
 *
 */
static void run_program(void)
{
	pc = ENTRY_PC;
	while (true) {
		unsigned int instr = memory[pc + 3] | (memory[pc + 2] << 8) | (memory[pc + 1] << 16) | (memory[pc] << 24);	//빅엔디안으로 명령어불러오기
		pc += 4;	//pc에 4더해줌(다음 명령어로 이동)
		if (process_instruction(instr) == false) { //명령어 실행(halt를 만나면 프로그램 종료)
			break;
		}
	}
}
