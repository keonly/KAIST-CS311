/***************************************************************
 *                                                             *
 *   MIPS-32 Instruction Level Simulator                       *
 *                                                             *
 *   CS311 KAIST                                               *
 *   parse.c                                                   *
 *                                                             *
 ***************************************************************/

#include <stdio.h>

#include "parse.h"
#include "run.h"
#include "util.h"

int text_size;
int data_size;

instruction
parsing_instr (const char *buffer, const int index)
{
    /* Implement this function */
    instruction *instr;
    int          binary;

    instr  = (instruction *) malloc (sizeof (instruction));
    binary = fromBinary (buffer);

    instr->opcode = (short) ((binary >> 26) & 0x3F);
    switch (OPCODE (instr)) {
    // I Format
    case 0x9:  // (0x001001)ADDIU
    case 0xC:  // (0x001100)ANDI
    case 0xF:  // (0x001111)LUI
    case 0xD:  // (0x001101)ORI
    case 0xB:  // (0x001011)SLTIU
    case 0x23: // (0x100011)LW
    case 0x2B: // (0x101011)SW
    case 0x4:  // (0x000100)BEQ
    case 0x5:  // (0x000101)BNE
        SET_RS (instr, (binary >> 21) & 0x1F);
        SET_RT (instr, (binary >> 16) & 0x1F);
        SET_IMM (instr, binary & 0xFFFF);
        break;

    // R Format
    case 0x0: // (0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
        SET_RS (instr, (binary >> 21) & 0x1F);
        SET_RT (instr, (binary >> 16) & 0x1F);
        SET_RD (instr, (binary >> 11) & 0x1F);
        SET_SHAMT (instr, (binary >> 6) & 0x1F);
        SET_FUNC (instr, binary & 0x3F);
        break;

    // J Format
    case 0x2: // (0x000010)J
    case 0x3: // (0x000011)JAL
        SET_TARGET (instr, binary & 0x3FFFFFF);
        break;

    default:
        printf ("Error in parse.c\n");
        exit (1);
    }

    return *instr;
}

void
parsing_data (const char *buffer, const int index)
{
    /* Implement this function */
    int binary;

    binary = fromBinary (buffer);
    mem_write_32 (MEM_DATA_START + index, binary);
}

void
print_parse_result ()
{
    int i;
    printf ("Instruction Information\n");

    for (i = 0; i < text_size / 4; i++) {
        printf ("INST_INFO[%d].value : %x\n", i, INST_INFO[i].value);
        printf ("INST_INFO[%d].opcode : %d\n", i, INST_INFO[i].opcode);

        switch (INST_INFO[i].opcode) {
        // I Format
        case 0x9:  // (0x001001)ADDIU
        case 0xC:  // (0x001100)ANDI
        case 0xF:  // (0x001111)LUI
        case 0xD:  // (0x001101)ORI
        case 0xB:  // (0x001011)SLTIU
        case 0x23: // (0x100011)LW
        case 0x2B: // (0x101011)SW
        case 0x4:  // (0x000100)BEQ
        case 0x5:  // (0x000101)BNE
            printf ("INST_INFO[%d].rs : %d\n", i, INST_INFO[i].r_t.r_i.rs);
            printf ("INST_INFO[%d].rt : %d\n", i, INST_INFO[i].r_t.r_i.rt);
            printf ("INST_INFO[%d].imm : %d\n", i, INST_INFO[i].r_t.r_i.r_i.imm);
            break;

        // R Format
        case 0x0: // (0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
            printf ("INST_INFO[%d].func_code : %d\n", i, INST_INFO[i].func_code);
            printf ("INST_INFO[%d].rs : %d\n", i, INST_INFO[i].r_t.r_i.rs);
            printf ("INST_INFO[%d].rt : %d\n", i, INST_INFO[i].r_t.r_i.rt);
            printf ("INST_INFO[%d].rd : %d\n", i, INST_INFO[i].r_t.r_i.r_i.r.rd);
            printf ("INST_INFO[%d].shamt : %d\n", i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
            break;

        // J Format
        case 0x2: // (0x000010)J
        case 0x3: // (0x000011)JAL
            printf ("INST_INFO[%d].target : %d\n", i, INST_INFO[i].r_t.target);
            break;

        default:
            printf ("Not available instruction\n");
            assert (0);
        }
    }

    printf ("Memory Dump - Text Segment\n");
    for (i = 0; i < text_size; i += 4)
        printf ("text_seg[%d] : %x\n", i, mem_read_32 (MEM_TEXT_START + i));
    for (i = 0; i < data_size; i += 4)
        printf ("data_seg[%d] : %x\n", i, mem_read_32 (MEM_DATA_START + i));
    printf ("Current PC: %x\n", CURRENT_STATE.PC);
}
