/**********************************************************************
 * Copyright (c) 2023-2024
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

#include "types.h"

 /***
  * External entities in other files.
  */
extern struct stage stages[];		/* Pipelining stages */

extern unsigned char memory[];		/* Memory */

extern unsigned int registers[];	/* Registers */

extern unsigned int pc;				/* Program counter */

/**
 * Helper functions that might be useful. See main.c for the details of them
 */
extern bool is_noop(int stage);
extern void make_stall(int stage, int cycles);


/**********************************************************************
 * List of instructions that should be supported
 *
 * | Name   | Format   | Opcode / opcode + funct |
 * | ------ | -------- | ----------------------- |
 * | `add`  | r-format | 0 + 0x20                |
 * | `addi` | i-format | 0x08                    |
 * | `sub`  | r-format | 0 + 0x22                |
 * | `and`  | r-format | 0 + 0x24                |
 * | `andi` | i-format | 0x0c                    |
 * | `or`   | r-format | 0 + 0x25                |
 * | `ori`  | i-format | 0x0d                    |
 * | `nor`  | r-format | 0 + 0x27                |
 * | `sll`  | r-format | 0 + 0x00                |
 * | `srl`  | r-format | 0 + 0x02                |
 * | `sra`  | r-format | 0 + 0x03                |
 * | `lw`   | i-format | 0x23                    |
 * | `sw`   | i-format | 0x2b                    |
 * | `beq`  | i-format | 0x04                    |
 * | `bne`  | i-format | 0x05                    |
 * | `slt`  | r-format | 0 + 0x2a                |
 * | `slti` | i-format | 0x0a                    |
 * | `jr`   | r-format | 0 + 0x08                |
 * | `j`    | j-format | 0x02                    |
 * | `jal`  | j-format | 0x03                    |
 */

void IF_stage(struct IF_ID* if_id)
{
	/***
	 * No need to check whether this stage is idle or not for some reasons...
	 */

	 /* TODO: Read one instruction in machine code from the memory */

	//메모리에서 명령어 읽음
	//현재 pc 값을 가져와서 IF 스테이지의 pc 값으로 설정

	//메모리에서 명령어를 읽음
	unsigned int instr = memory[pc + 3] | (memory[pc + 2] << 8) | (memory[pc + 1] << 16) | (memory[pc] << 24);

	/***
	 * Set @stages[IF].instruction.machine_instr with the read machine code
	 * and @stages[IF].__pc with the current value of the program counter.
	 * DO NOT REMOVE THOSE TWO STATEMENTS, the framework requires these values.
	 */
	stages[IF].instruction.machine_instr = instr;
	stages[IF].__pc = pc;

	/* TODO: Fill in IF-ID interstage register */
	//IF 스테이지에서 명령어를 읽은 후에 pc 값을 증가시킴
	pc += 4;
	if_id->instruction = instr;
	if_id->next_pc = pc;

	/***
	 * The framework processes @stage[IF].instruction.machine_instr under
	 * the hood to allow you to access human-readable stages[*].instruction
	 * in the following stages. Check __run_cycle() in main.c.
	 */
}


void ID_stage(struct IF_ID* if_id, struct ID_EX* id_ex)
{
	struct instruction* instr = &stages[ID].instruction;

	if (is_noop(ID)) return;

	/***
	 * Register write should be taken place in WB_stage,
	 * so actually there is nothing to do here for register write.
	 */

	 /* TODO: Process register read. May use if_id */
	unsigned int rs = (if_id->instruction >> 21) & 0x1f;	//rs 번호
	unsigned int rt = (if_id->instruction >> 16) & 0x1f;	//rt 번호

	 /* TODO: Fill in ID-EX interstage register */
	id_ex->next_pc = if_id->next_pc;	//다음 PC값
    id_ex->reg1_value = registers[rs];	//rs값
    id_ex->reg2_value = registers[rt];	//rt값

    id_ex->instr_20_16 = if_id->instruction >> 16 & 0x1f;	//rt번호
    id_ex->instr_15_11 = if_id->instruction >> 11 & 0x1f;	//rd번호
	unsigned int imm_16 = if_id->instruction & 0xffff;	//명령어의 하위 16비트 가져옴
	unsigned int imm_32;

	if (imm_16 & 0x8000) {	//음수인 경우
		imm_32 = imm_16 | 0xFFFF0000;	//32비트 sign extend
	}
	else {	//양수인 경우
		imm_32 = imm_16;
	}
	id_ex->immediate = imm_32;	//imm값

	if ((instr->opcode == 0x02) || (instr->opcode == 0x03) || (instr->opcode == 0x04) || (instr->opcode == 0x05)) {
		if ((instr->opcode == 0x02) || (instr->opcode == 0x03)) {
			id_ex->immediate = if_id->instruction & 0x03FFFFFF;
			id_ex->immediate = (id_ex->next_pc & 0xF0000000) | (id_ex->immediate << 2);	//상위 4비트는 현재 PC의 상위 4비트와 동일하게 유지되어야 함
		}
		make_stall(IF, 3);
		return;
	}
}

void EX_stage(struct ID_EX* id_ex, struct EX_MEM* ex_mem)
{
	struct instruction* instr = &stages[EX].instruction;

	if (is_noop(EX)) return;

	/* TODO: Good luck! */

	ex_mem->next_pc = id_ex->next_pc;

	switch (instr->format) {
	//R-format 명령어
	case r_format:
		ex_mem->write_reg = id_ex->instr_15_11;	//rd의 번호
		switch (instr->r_format.funct) {
		case 0x20:	//add
			ex_mem->alu_out = id_ex->reg1_value + id_ex->reg2_value;
			break;
		case 0x22:	//sub
			ex_mem->alu_out = id_ex->reg1_value - id_ex->reg2_value;
			break;
		case 0x24:	//and
			ex_mem->alu_out = id_ex->reg1_value & id_ex->reg2_value;
			break;
		case 0x25:	//or
			ex_mem->alu_out = id_ex->reg1_value | id_ex->reg2_value;
			break;
		case 0x27:	//nor
			ex_mem->alu_out = ~(id_ex->reg1_value | id_ex->reg2_value);
			break;
		case 0x00:	//sll(쉬프트 명령어는 rs사용X -> rt만 사용)
			ex_mem->alu_out = id_ex->reg2_value << instr->r_format.shamt;
			break;
		case 0x02:	//srl
			ex_mem->alu_out = id_ex->reg2_value >> instr->r_format.shamt;
			break;
		case 0x03:	//sra
		{
			int sign_bit = (id_ex->reg2_value >> 31) & 0x1; //	rt의 부호비트
			if (sign_bit == 1) {	//음수일경우
				ex_mem->alu_out = (id_ex->reg2_value >> instr->r_format.shamt) | ((0xFFFFFFFF) << (32 - instr->r_format.shamt));
			}
			else {	//양수일경우
				ex_mem->alu_out = (id_ex->reg2_value >> instr->r_format.shamt);
			}
			break;
		}
		case 0x2a:	//slt
		{
			if ((int)id_ex->reg1_value < (int)id_ex->reg2_value) {
				ex_mem->alu_out = 1;
			}
			else {
				ex_mem->alu_out = 0;
			}
		}
		break;
		case 0x08:  //jr
		{
			ex_mem->next_pc = id_ex->reg1_value;
		}
		break;
		default:
			break;
		}
		break;

	// I-format 명령어
	case i_format:
		ex_mem->write_reg = id_ex->instr_20_16;	//rt의 번호
		switch (instr->opcode) {
		case 0x08:	//addi
			ex_mem->alu_out = id_ex->reg1_value + id_ex->immediate;
			break;
		case 0x0c:	//andi
			ex_mem->alu_out = id_ex->reg1_value & (id_ex->immediate & 0x0000FFFF);
			break;
		case 0x0d:	//ori
			ex_mem->alu_out = id_ex->reg1_value | (id_ex->immediate & 0x0000FFFF);
			break;
		case 0x23:	//lw
			ex_mem->alu_out = id_ex->reg1_value + id_ex->immediate;	//메모리에서 값을 빼올 주소
			break;
		case 0x2B:	//sw
			ex_mem->alu_out = id_ex->reg1_value + (int)id_ex->immediate;	//rt값을 저장할 주소 - sw  rt (imm)rs
			ex_mem->write_value = id_ex->reg2_value;	//rt의 값
			break;
		case 0x04:	//beq
			ex_mem->next_pc = id_ex->next_pc + (id_ex->immediate << 2);	//점프뛸 주소
			ex_mem->alu_out = id_ex->reg1_value - id_ex->reg2_value;
			break;
		case 0x05:	//bne
			ex_mem->next_pc = id_ex->next_pc + (id_ex->immediate << 2);	//점프뛸 주소
			ex_mem->alu_out = id_ex->reg1_value - id_ex->reg2_value;
			break;
		case 0x0a: // slti
			if ((int)id_ex->reg1_value < (int)id_ex->immediate) {	//immediate값과 rs값 비교
				ex_mem->alu_out = 1;
			} else {
				ex_mem->alu_out = 0;
			}
			break;
		default:
			break;
		}
		break;

	case j_format:
		switch (instr->opcode) {
		case 0x02: // j
			//일단 점프할주소 저장
			ex_mem->next_pc = id_ex->immediate; //하위 26비트 추출
			break;
		case 0x03: // jal
			//ra에 현재PC저장하고 점프할주소 저장
			registers[31] = ex_mem->next_pc;
			ex_mem->next_pc = id_ex->immediate;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void MEM_stage(struct EX_MEM* ex_mem, struct MEM_WB* mem_wb)
{
	struct instruction* instr = &stages[MEM].instruction;

	if (is_noop(MEM)) return;

	switch (instr->format) {
	case r_format:  //r-format 명령어
		mem_wb->write_reg = ex_mem->write_reg;	//rd번호 그대로 전달
		mem_wb->alu_out = ex_mem->alu_out;
		break;
	case i_format:  //i-format 명령어
		if (instr->opcode == 0x23) {    //lw
			mem_wb->write_reg = ex_mem->write_reg;
			int con = (memory[ex_mem->alu_out] << 24) | (memory[ex_mem->alu_out + 1] << 16) | (memory[ex_mem->alu_out + 2] << 8) | memory[ex_mem->alu_out + 3];	//메모리에서 빼온값 전달
			mem_wb->mem_out = con;
		}
		else if (instr->opcode == 0x2b) {	//sw
			memory[ex_mem->alu_out] = (ex_mem->write_value >> 24) & 0xFF;	//상위 8비트
			memory[ex_mem->alu_out + 1] = (ex_mem->write_value >> 16) & 0xFF;	//다음 8비트
			memory[ex_mem->alu_out + 2] = (ex_mem->write_value >> 8) & 0xFF;	//다음 8비트
			memory[ex_mem->alu_out + 3] = ex_mem->write_value & 0xFF;	//하위 8비트
		}
		else if (instr->opcode == 0x04) {	//beq
			if (ex_mem->alu_out == 0) {	//rs==rt
				pc = ex_mem->next_pc;
			}
		}
		else if (instr->opcode == 0x05) {	//bne
			if (ex_mem->alu_out != 0) {	//rs!=rt
				pc = ex_mem->next_pc;
			}
		}
		else {
			mem_wb->alu_out = ex_mem->alu_out;
			mem_wb->write_reg = ex_mem->write_reg;
		}
		break;
	case j_format:  // j-format 명령어
		pc = ex_mem->next_pc;
		break;
	}
}

void WB_stage(struct MEM_WB* mem_wb)
{
	struct instruction* instr = &stages[WB].instruction;

	if (is_noop(WB)) return;

	switch (instr->format) {
	case r_format:  // r-format 명령어
		registers[mem_wb->write_reg] = mem_wb->alu_out;
		break;
	case i_format:  // i-format 명령어
		if (instr->opcode == 0x23) {	//lw
			registers[mem_wb->write_reg] = mem_wb->mem_out;
		}
		else if(instr->opcode == 0x2b){	//sw
			break;
		}
		else if (instr->opcode == 0x04) {	//beq
			break;
		}
		else if (instr->opcode == 0x05) {	//bne
			break;
		}
		else {	//나머지 i-format 명령어
			registers[mem_wb->write_reg] = mem_wb->alu_out;
		}
		break;
	case j_format:  //j-format 명령어
		break;
	}
}
