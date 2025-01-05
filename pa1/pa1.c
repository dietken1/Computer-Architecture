/**********************************************************************
 * Copyright (c) 2021-2024
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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#pragma warning(disable : 4996)
unsigned int register_num(char* num);
static unsigned int r_format(unsigned int opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, unsigned int func);
static unsigned int i_format(unsigned int opcode, unsigned int rs, unsigned int rt, int con);

 /*====================================================================*/
 /*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******       */
#define MAX_NR_TOKENS	16	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	32	/* Maximum length of single token */
#define MAX_ASSEMBLY	128 /* Maximum length of assembly string */

/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *   Parse @assembly, and put each assembly token into @tokens[].
 *
 * RETURN VALUE
 *   Return the number of tokens. Characters in @assembly are converted
 *   to lower-cases.
 *
 */
static int parse_command(char* assembly, char* tokens[])
{
	char* curr = assembly;
	int token_started = false;
	int nr_tokens = 0;

	while (*curr != '\0') {
		if (isspace(*curr)) {
			*curr = '\0';
			token_started = false;
		}
		else {
			*curr = tolower(*curr);
			if (!token_started) {
				tokens[nr_tokens++] = curr;
				token_started = true;
			}
		}
		curr++;
	}

	return nr_tokens;
}

static unsigned int translate(int nr_tokens, char* tokens[]);

/***********************************************************************
 * The main function of this program.
 */
int main(int argc, char* const argv[])
{
	char assembly[MAX_ASSEMBLY] = { '\0' };
	FILE* input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin) {
		printf("*********************************************************\n");
		printf("*          >> SCE212 MIPS translator  v0.10 <<          *\n");
		printf("*                                                       *\n");
		printf("*          https://git.ajou.ac.kr/sslab/ca-pa1          *\n");
		printf("*                                  2024 Spring          *\n");
		printf("*********************************************************\n\n");
		printf(">> ");
	}

	while (fgets(assembly, sizeof(assembly), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens;
		unsigned int instruction;

		nr_tokens = parse_command(assembly, tokens);

		if (nr_tokens <= 0) continue;

		instruction = translate(nr_tokens, tokens);

		fprintf(stderr, "0x%08x\n", instruction);

		if (input == stdin) printf(">> ");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
/*          ****** DO NOT MODIFY ANYTHING ABOVE THIS LINE ******      */
/*====================================================================*/


/***********************************************************************
 * translate()
 *
 * DESCRIPTION
 *   Translate assembly represented in @tokens[] into a MIPS instruction.
 *   This translate should support following 15 assembly commands
 *
 *    - add
 *    - addi
 *    - sub
 *    - and
 *    - andi
 *    - or
 *    - ori
 *    - nor
 *    - lw
 *    - sw
 *    - sll
 *    - srl
 *    - sra
 *    - beq
 *    - bne
 *
 * RETURN VALUE
 *   Return a 32-bit MIPS instruction
 *
 */
static unsigned int translate(int nr_tokens, char* tokens[]){

	unsigned int result = 0;

	if (strcmp(tokens[0], "add") == 0) {	//add
		unsigned int opcode = 0; // 5비트 opcode
		unsigned int rs = register_num(tokens[2]); // 5비트 rs 레지스터
		unsigned int rt = register_num(tokens[3]); // 5비트 rt 레지스터
		unsigned int rd = register_num(tokens[1]); // 5비트 rd 레지스터
		unsigned int shamt = 0; // 5비트 쉬프트 어마운트
		unsigned int func = 32; // 6비트 함수 코드 (0x20)
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "sub") == 0) {	//sub
		unsigned int opcode = 0;
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[3]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt = 0;
		unsigned int func = 34;	//0x22
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "and") == 0) {	//and
		unsigned int opcode = 0;
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[3]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt = 0;
		unsigned int func = 36;	//0x24
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "or") == 0) {	//or
		unsigned int opcode = 0;
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[3]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt = 0;
		unsigned int func = 37;	//0x25
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "nor") == 0) {	//nor
		unsigned int opcode = 0;
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[3]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt = 0;
		unsigned int func = 39;	//0x27
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "sll") == 0) {	//sll
		unsigned int opcode = 0;
		unsigned int rs = 0;
		unsigned int rt = register_num(tokens[2]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt;
		if (tokens[3][0] == '0' && tokens[3][1] == 'x') {
			shamt = strtol(tokens[3], NULL, 16);
		}
		else {
			shamt = atoi(tokens[3]);
		}
		
		unsigned int func = 0;	//0x00
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "srl") == 0) {	//srl
		unsigned int opcode = 0;
		unsigned int rs = 0;
		unsigned int rt = register_num(tokens[2]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt;
		if (tokens[3][0] == '0' && tokens[3][1] == 'x') {
			shamt = strtol(tokens[3], NULL, 16);
		}
		else {
			shamt = atoi(tokens[3]);
		}

		unsigned int func = 2;	//0x02
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "sra") == 0) {	//sra
		unsigned int opcode = 0;
		unsigned int rs = 0;
		unsigned int rt = register_num(tokens[2]);
		unsigned int rd = register_num(tokens[1]);
		unsigned int shamt;
		if (tokens[3][0] == '0' && tokens[3][1] == 'x') {
			shamt = strtol(tokens[3], NULL, 16);
		}
		else {
			shamt = atoi(tokens[3]);
		}

		unsigned int func = 3;	//0x03
		result = r_format(opcode, rs, rt, rd, shamt, func);
	}

	if (strcmp(tokens[0], "addi") == 0) {	//addi
		int con;
		unsigned int opcode = 8;	//0x08
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[3][1] == '0' && tokens[3][2] == 'x') || (tokens[3][0] == '0' && tokens[3][1] == 'x')) {	//16진수일때
			con = strtol(tokens[3], NULL, 16);
		}
		else {	//10진수일때
			con = strtol(tokens[3], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	if (strcmp(tokens[0], "andi") == 0) {	//andi
		int con;
		unsigned int opcode = 12;	//0x0c
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[3][1] == '0' && tokens[3][2] == 'x') || (tokens[3][0] == '0' && tokens[3][1] == 'x')) {
			con = strtol(tokens[3], NULL, 16);
		}
		else {
			con = strtol(tokens[3], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	if (strcmp(tokens[0], "ori") == 0) {	//ori
		int con;
		unsigned int opcode = 13;	//0x0d
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[3][1] == '0' && tokens[3][2] == 'x') || (tokens[3][0] == '0' && tokens[3][1] == 'x')) {
			con = strtol(tokens[3], NULL, 16);
		}
		else {
			con = strtol(tokens[3], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	if (strcmp(tokens[0], "lw") == 0) {	//lw
		int con;
		unsigned int opcode = 35;	//0x23
		unsigned int rs = register_num(tokens[3]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[2][1] == '0' && tokens[2][2] == 'x') || (tokens[2][0] == '0' && tokens[2][1] == 'x')) {
			con = strtol(tokens[2], NULL, 16);
		}
		else {
			con = strtol(tokens[2], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	if (strcmp(tokens[0], "sw") == 0) {	//sw
		int con;
		unsigned int opcode = 43;	//0x2b
		unsigned int rs = register_num(tokens[3]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[2][1] == '0' && tokens[2][2] == 'x') || (tokens[2][0] == '0' && tokens[2][1] == 'x')) {
			con = strtol(tokens[2], NULL, 16);
		}
		else {
			con = strtol(tokens[2], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	if (strcmp(tokens[0], "beq") == 0) {	//beq
		int con;
		unsigned int opcode = 4;	//0x04
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[3][1] == '0' && tokens[3][2] == 'x') || (tokens[3][0] == '0' && tokens[3][1] == 'x')) {
			con = strtol(tokens[3], NULL, 16);
		}
		else {
			con = strtol(tokens[3], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	if (strcmp(tokens[0], "bne") == 0) {	//bne
		int con;
		unsigned int opcode = 5;	//0x05
		unsigned int rs = register_num(tokens[2]);
		unsigned int rt = register_num(tokens[1]);
		if ((tokens[3][1] == '0' && tokens[3][2] == 'x') || (tokens[3][0] == '0' && tokens[3][1] == 'x')) {
			con = strtol(tokens[3], NULL, 16);
		}
		else {
			con = strtol(tokens[3], NULL, 10);
		}
		result = i_format(opcode, rs, rt, con);
	}

	return result;
}

static unsigned int r_format(unsigned int opcode, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, unsigned int func) {	//R-Format함수

	unsigned int result = 0;

	// opcode, rs, rt, rd, shamt, func를 result에 추가
	result = result | (opcode << 26); // opcode추가(27~32)
	result = result | (rs << 21);     // rs추가(22~26)
	result = result | (rt << 16);     // rt추가(17~21)
	result = result | (rd << 11);     // rd추가(12~16)
	result = result | (shamt << 6);   // shamt추가(7~11)
	result = result | func;            // func추가(1~6)

	return result;
}

static unsigned int i_format(unsigned int opcode, unsigned int rs, unsigned int rt, int con) {	//I-Format함수

	unsigned int result = 0;
	con = con & 0xFFFF; // 하위 16비트만 유효
	// opcode, rs, rt, con을 result에 추가
	result = result | (opcode << 26); // opcode추가(27~32)
	result = result | (rs << 21);     // rs추가(22~26)
	result = result | (rt << 16);     // rt추가(17~21)
	result = result | (unsigned int)con;    // con추가(1~16)

	return result;
}

// 레지스터 번호를 반환하는 함수
unsigned int register_num(char* num) {
	if (strcmp(num, "zero") == 0) return 0;
	else if (strcmp(num, "at") == 0) return 1;
	else if (strcmp(num, "v0") == 0) return 2;
	else if (strcmp(num, "v1") == 0) return 3;
	else if (strcmp(num, "a0") == 0) return 4;
	else if (strcmp(num, "a1") == 0) return 5;
	else if (strcmp(num, "a2") == 0) return 6;
	else if (strcmp(num, "a3") == 0) return 7;
	else if (strcmp(num, "t0") == 0) return 8;
	else if (strcmp(num, "t1") == 0) return 9;
	else if (strcmp(num, "t2") == 0) return 10;
	else if (strcmp(num, "t3") == 0) return 11;
	else if (strcmp(num, "t4") == 0) return 12;
	else if (strcmp(num, "t5") == 0) return 13;
	else if (strcmp(num, "t6") == 0) return 14;
	else if (strcmp(num, "t7") == 0) return 15;
	else if (strcmp(num, "s0") == 0) return 16;
	else if (strcmp(num, "s1") == 0) return 17;
	else if (strcmp(num, "s2") == 0) return 18;
	else if (strcmp(num, "s3") == 0) return 19;
	else if (strcmp(num, "s4") == 0) return 20;
	else if (strcmp(num, "s5") == 0) return 21;
	else if (strcmp(num, "s6") == 0) return 22;
	else if (strcmp(num, "s7") == 0) return 23;
	else if (strcmp(num, "t8") == 0) return 24;
	else if (strcmp(num, "t9") == 0) return 25;
	else if (strcmp(num, "k0") == 0) return 26;
	else if (strcmp(num, "k1") == 0) return 27;
	else if (strcmp(num, "gp") == 0) return 28;
	else if (strcmp(num, "sp") == 0) return 29;
	else if (strcmp(num, "fp") == 0) return 30;
	else if (strcmp(num, "ra") == 0) return 31;
	else return -1;
}
