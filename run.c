/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"
#include "parse.h"

enum op_type_
{
	R,
	I,
	J

};


enum op_type_ identify_type_(short op)
{

	enum op_type_ type;

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
/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}


void process_R_instruction(instruction* inst_to_exec)
{
    short funct_field = FUNC(inst_to_exec);
    unsigned char rs = RS(inst_to_exec),rd = RD(inst_to_exec),rt = RT(inst_to_exec);
    unsigned char shamt = (unsigned char)SHAMT(inst_to_exec);
    


    if (funct_field == 0x21) // addu
    {
        CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];

    }
    else if (funct_field == 0x24) //and
    {
        CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];

    }
    else if (funct_field == 0x27) // nor
    {
        CURRENT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);

    }
    else if (funct_field == 0x25) // or
    {
        CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];

    }
    else if (funct_field == 0x2B) //sltu

    {
        CURRENT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt])? TRUE : FALSE;

    }
    else if (funct_field == 0x00 ) //sll
    {
        CURRENT_STATE.REGS[rd] =  CURRENT_STATE.REGS[rt] << shamt;

    }
    else if (funct_field == 0x02 ) //srl
    {
        CURRENT_STATE.REGS[rd] =  CURRENT_STATE.REGS[rt] >> shamt;

    }
    else if (funct_field == 0x23 ) //subu
    {
        CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];

    }
    else if (funct_field == 0x8) //jr
    {

        CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
        return;

    }

    CURRENT_STATE.PC += 4;

}




void process_I_instruction(instruction* inst_to_exec)
{
    short opcode = OPCODE(inst_to_exec);
    unsigned char rs = RS(inst_to_exec),rt = RT(inst_to_exec);
    short immediate = IMM(inst_to_exec);
    int sign_extended = SIGN_EX(immediate);
    uint32_t zero_extended = (unsigned short)immediate;

    if (opcode == 0x9) //addiu
    {
        CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + sign_extended;
    }
    else if (opcode == 0xC ) //andi
    {
        CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & zero_extended;
        
    }
    else if (opcode == 0xF ) //lui
    {
        CURRENT_STATE.REGS[rt] = zero_extended << 16;
        
    }
    else if (opcode == 0xD ) //ori
    {
        CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | zero_extended;
        
    }
    else if (opcode == 0xB ) //sltiu
    {
        CURRENT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] < sign_extended)? TRUE : FALSE;
        
    }
    else if (opcode == 0x23 ) //lw
    {
        uint32_t addr_mem = sign_extended + CURRENT_STATE.REGS[rs];
        CURRENT_STATE.REGS[rt] = mem_read_32(addr_mem);
    }
    else if (opcode == 0x2B ) //sw
    {
        uint32_t addr_mem = sign_extended + CURRENT_STATE.REGS[rs];
        mem_write_32(addr_mem,CURRENT_STATE.REGS[rt]);
        
    }
    else if (opcode ==  0x4) //BEQ
    {

        if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt])
        {
            CURRENT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extended << 2);
            return;

        }
    
        
    }
    else if (opcode == 0x5 ) //BNE
    {
        if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt])
        {
            CURRENT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extended << 2);
            return;

        }

        
    }

    CURRENT_STATE.PC += 4;

}


void process_J_instruction(instruction* inst_to_exec)
{
    short opcode = OPCODE(inst_to_exec);
    uint32_t target = TARGET(inst_to_exec);
    uint32_t jump_target = (target << 2) & 0x0FFFFFFF;
    uint32_t upper_4_bits = (CURRENT_STATE.PC + 4) & 0xF0000000;

    if (opcode == 0x2) //j
    {
        CURRENT_STATE.PC = upper_4_bits | jump_target;
    }
    else if(opcode == 0x3) //jal
    {
        CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
        CURRENT_STATE.PC = upper_4_bits | jump_target;

    }



}







/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/

void process_instruction()
{
    uint32_t pc = CURRENT_STATE.PC;
    


    if (pc - MEM_TEXT_START >= text_size)
    {
        RUN_BIT = FALSE;
        return;
    }

    instruction* current_instr = get_inst_info(pc);



    short opcode = current_instr->opcode;

    enum op_type_ type = identify_type_(opcode);

    if (type == R)
    {
        process_R_instruction(current_instr);

    }

    else if (type == J)
    {
        process_J_instruction(current_instr);
    }

    else
    {
        process_I_instruction(current_instr);
        
    }
    
	
}
