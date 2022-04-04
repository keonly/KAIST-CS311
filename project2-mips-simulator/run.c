/***************************************************************
 *                                                             *
 *   MIPS-32 Instruction Level Simulator                       *
 *                                                             *
 *   CS311 KAIST                                               *
 *   run.c                                                     *
 *                                                             *
 ***************************************************************/

#include "run.h"
#include "util.h"
#include <stdio.h>

/***************************************************************
 *                                                             *
 * Procedure: get_inst_info                                    *
 *                                                             *
 * Purpose: Read insturction information                       *
 *                                                             *
 ***************************************************************/
instruction *
get_inst_info (uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************
 *                                                             *
 * Procedure: process_instruction                              *
 *                                                             *
 * Purpose: Process one instrction                             *
 *                                                             *
 ***************************************************************/
void
process_instruction ()
{
    /* Implement this function */
    instruction *curr_instr;

    curr_instr = get_inst_info (CURRENT_STATE.PC);
    CURRENT_STATE.PC += 4;
    switch (OPCODE (curr_instr)) {
        // I Format
    case 0x9: // (0x001001)ADDIU
        CURRENT_STATE.REGS[RT (curr_instr)]
            = CURRENT_STATE.REGS[RS (curr_instr)] + SIGN_EX (IMM (curr_instr));
        break;
    case 0xC: // (0x001100)ANDI
        CURRENT_STATE.REGS[RT (curr_instr)]
            = CURRENT_STATE.REGS[RS (curr_instr)] & ZERO_EX (IMM (curr_instr));
        break;
    case 0xF: // (0x001111)LUI
        CURRENT_STATE.REGS[RT (curr_instr)] = SIGN_EX (IMM (curr_instr)) << 16;
        break;
    case 0xD: // (0x001101)ORI
        CURRENT_STATE.REGS[RT (curr_instr)]
            = CURRENT_STATE.REGS[RS (curr_instr)] | ZERO_EX (IMM (curr_instr));
        break;
    case 0xB: // (0x001011)SLTIU
        CURRENT_STATE.REGS[RT (curr_instr)]
            = (CURRENT_STATE.REGS[RS (curr_instr)] < SIGN_EX (IMM (curr_instr))) ? 1 : 0;
        break;
    case 0x23: // (0x100011)LW
        CURRENT_STATE.REGS[RT (curr_instr)] = mem_read_32 (
            CURRENT_STATE.REGS[RS (curr_instr)] + SIGN_EX (IMM (curr_instr)));
        break;
    case 0x2B: // (0x101011)SW
        mem_write_32 (CURRENT_STATE.REGS[RS (curr_instr)] + SIGN_EX (IMM (curr_instr)),
                      CURRENT_STATE.REGS[RT (curr_instr)]);
        break;
    case 0x4: // (0x000100)BEQ
        if (CURRENT_STATE.REGS[RS (curr_instr)] == CURRENT_STATE.REGS[RT (curr_instr)])
            CURRENT_STATE.PC += SIGN_EX (IMM (curr_instr)) << 2;
        break;
    case 0x5: // (0x000101)BNE
        if (CURRENT_STATE.REGS[RS (curr_instr)] != CURRENT_STATE.REGS[RT (curr_instr)])
            CURRENT_STATE.PC += SIGN_EX (IMM (curr_instr)) << 2;
        break;

        // R Format
    case 0x0:
        switch (FUNC (curr_instr)) {
        case 0x21: // (0x100001)ADDU
            CURRENT_STATE.REGS[RD (curr_instr)] = CURRENT_STATE.REGS[RS (curr_instr)]
                                                  + CURRENT_STATE.REGS[RT (curr_instr)];
            break;
        case 0x24: // (0x100100)AND
            CURRENT_STATE.REGS[RD (curr_instr)] = CURRENT_STATE.REGS[RS (curr_instr)]
                                                  & CURRENT_STATE.REGS[RT (curr_instr)];
            break;
        case 0x8: // (0x001000)JR
            CURRENT_STATE.PC = CURRENT_STATE.REGS[RS (curr_instr)];
            break;
        case 0x27: // (0x100111)NOR
            CURRENT_STATE.REGS[RD (curr_instr)]
                = ~(CURRENT_STATE.REGS[RS (curr_instr)]
                    | CURRENT_STATE.REGS[RT (curr_instr)]);
            break;
        case 0x25: // (0x100101)OR
            CURRENT_STATE.REGS[RD (curr_instr)] = CURRENT_STATE.REGS[RS (curr_instr)]
                                                  | CURRENT_STATE.REGS[RT (curr_instr)];
            break;
        case 0x2B: // (0x101100)SLTU
            CURRENT_STATE.REGS[RD (curr_instr)] = (CURRENT_STATE.REGS[RS (curr_instr)]
                                                   < CURRENT_STATE.REGS[RT (curr_instr)])
                                                      ? 1
                                                      : 0;
            break;
        case 0x00: // (0x000000)SLL
            CURRENT_STATE.REGS[RD (curr_instr)] = CURRENT_STATE.REGS[RT (curr_instr)]
                                                  << SHAMT (curr_instr);
            break;
        case 0x02: // (0x000010)SRL
            CURRENT_STATE.REGS[RD (curr_instr)]
                = CURRENT_STATE.REGS[RT (curr_instr)] >> SHAMT (curr_instr);
            break;
        case 0x23: // (0x100011)SUBU
            CURRENT_STATE.REGS[RD (curr_instr)] = CURRENT_STATE.REGS[RS (curr_instr)]
                                                  - CURRENT_STATE.REGS[RT (curr_instr)];
            break;
        }
        break;

        // J Format
    case 0x2: // (0x000010)J
        CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (TARGET (curr_instr) << 2);
        break;
    case 0x3: // (0x000011)JAL
        CURRENT_STATE.REGS[31] = CURRENT_STATE.PC;
        CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (TARGET (curr_instr) << 2);
        break;

    default:
        printf ("Error in run.c\n");
        exit (1);
    }

    if (CURRENT_STATE.PC >= MEM_TEXT_START + 4 * NUM_INST)
        RUN_BIT = FALSE;
}
