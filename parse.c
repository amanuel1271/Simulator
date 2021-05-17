/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"
#include "run.h"

#define size_opcode 6
#define loc_rs 6
#define loc_rt 11
#define loc_rd 16
#define loc_shamt 21
#define loc_func 26
#define size_reg_op 5
#define size_immediate 16
#define size_target 26

int text_size;
int data_size;

enum op_type
{
	R,
	I,
	J

};


enum op_type identify_type(unsigned int op)
{

	enum op_type type;

	if (op == 0x0)
	{
		type = R;
	}

	else if (op == 0x2 || op == 0x3)
	{
		type = J;
	}

	else
	{
		type = I;
	}

	
	return type;

}



instruction handle_R_inst(unsigned int op, const char* buffer)
{
	instruction instr;


	char* rs = calloc(size_reg_op + 1,sizeof(char));
	char* rt = calloc(size_reg_op + 1,sizeof(char));
	char* rd = calloc(size_reg_op + 1,sizeof(char));
	char* shamt = calloc(size_reg_op + 1,sizeof(char));
	char* funct = calloc(size_opcode + 1,sizeof(char));


	strncpy(rs,buffer + loc_rs,size_reg_op);
	strncpy(rt,buffer + loc_rt,size_reg_op);
	strncpy(rd,buffer + loc_rd,size_reg_op);
	strncpy(shamt,buffer + loc_shamt,size_reg_op);
	strncpy(funct,buffer + loc_func,size_opcode);


	SET_OPCODE(&instr,op); 
	SET_RS(&instr,(unsigned int)fromBinary(rs));
	SET_RT(&instr,(unsigned int)fromBinary(rt));
	SET_RD(&instr,(unsigned int)fromBinary(rd));
	SET_SHAMT(&instr,(unsigned int)fromBinary(shamt));
	SET_FUNC(&instr,(unsigned int)fromBinary(funct));



	return instr;

}


instruction handle_I_inst(unsigned int op, const char* buffer)
{
	instruction instr;

	char* rs = calloc(size_reg_op + 1,sizeof(char));
	char* rt = calloc(size_reg_op + 1,sizeof(char));
	char* imm = calloc(size_immediate + 1,sizeof(char));

	strncpy(rs,buffer + loc_rs,size_reg_op);
	strncpy(rt,buffer + loc_rt,size_reg_op);
	strncpy(imm,buffer + loc_rd,size_immediate);



	SET_OPCODE(&instr,op);
	SET_RS(&instr,(unsigned int)fromBinary(rs));
	SET_RT(&instr,(unsigned int)fromBinary(rt));
	SET_IMM(&instr,(int)fromBinary(imm)); // should be signed


	return instr;

}


instruction handle_J_inst(unsigned int op, const char* buffer)
{
	instruction instr;

	char* target = calloc(size_target + 1,sizeof(char));

	strncpy(target,buffer + loc_rs,size_target);

	SET_OPCODE(&instr,op);
	SET_TARGET(&instr,(unsigned int)fromBinary(target));


	return instr;

}


instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;

	mem_write_32(MEM_TEXT_START + index, fromBinary((char*)buffer));


	char* get_opcode = calloc(size_opcode + 1,sizeof(char));
	strncpy(get_opcode,buffer,size_opcode);
	unsigned int opcode = fromBinary(get_opcode);



	enum op_type type =  identify_type(opcode);
	

	if (type == R)
	{
		instr = handle_R_inst(opcode,buffer);
	}

	else if (type == I)
	{
		instr = handle_I_inst(opcode,buffer);
	}

	else if(type == J)
	{
		instr = handle_J_inst(opcode,buffer);
	}

    return instr;
}



void parsing_data(const char *buffer, const int index)
{

	mem_write_32(MEM_DATA_START + index,fromBinary((char *)buffer));
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //I format
	    case 0x9:		//ADDIU
	    case 0xc:		//ANDI
	    case 0xf:		//LUI	
	    case 0xd:		//ORI
	    case 0xb:		//SLTIU
	    case 0x23:		//LW	
	    case 0x2b:		//SW
	    case 0x4:		//BEQ
	    case 0x5:		//BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	    //R format
	    case 0x0:		//ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	    //J format
	    case 0x2:		//J
	    case 0x3:		//JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
