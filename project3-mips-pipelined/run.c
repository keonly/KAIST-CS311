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

instruction *
get_inst_info (uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

//////////////////////////// Helper functions ////////////////////////////

control_bits
set_control_bits (instruction *instr)
{
    control_bits control;

    control.ALU_SRC
        = (OPCODE (instr) == 0x04 || OPCODE (instr) == 0x05 || OPCODE (instr) == 0x0)
              ? 0
              : 1;
    control.ALU_OP     = OPCODE (instr);
    control.REG_DST    = OPCODE (instr) == 0x0 ? 1 : 0;
    control.MEM_WRITE  = OPCODE (instr) == 0x2b ? 1 : 0;
    control.MEM_READ   = OPCODE (instr) == 0x23 ? 1 : 0;
    control.BRANCH     = (OPCODE (instr) == 0x4 || OPCODE (instr) == 0x5) ? 1 : 0;
    control.MEM_TO_REG = OPCODE (instr) == 0x23 ? 1 : 0;
    switch (OPCODE (instr))
    {
    case 0x4:
    case 0x5:
    case 0x2:
    case 0x3:
    case 0x2b:
        control.REG_WRITE = 0;
        break;
    case 0x0:
        control.REG_WRITE = FUNC (instr) == 0x8 ? 0 : 1;
        break;
    default:
        control.REG_WRITE = 1;
        break;
    }

    return control;
}

uint32_t
alu_operation (uint32_t op1, uint32_t op2)
{
    uint32_t shamt, func;

    shamt = (CURRENT_STATE.ID_EX_IMM >> 6) & 0x1f;
    func  = CURRENT_STATE.ID_EX_IMM & 0x3f;

    switch (CURRENT_STATE.ID_EX_CTRL.ALU_OP)
    {
    // I Format
    case 0x09: //(0x001001)ADDIU
        return op1 + op2;
    case 0x0c: //(0x001100)ANDI
        return op1 & op2;
    case 0x0f: //(0x001111)LUI
        return op2 << 16;
    case 0x0d: //(0x001101)ORI
        return op1 | op2;
    case 0x0b: //(0x001011)SLTIU
        return op1 < op2;
    case 0x23: //(0x100011)LW
    case 0x2b: //(0x101011)SW
        CURRENT_STATE.EX_MEM_W_VALUE = op1 + op2;
        break;
    case 0x04: //(0x000100)BEQ
        CURRENT_STATE.EX_MEM_BR_TARGET
            = CURRENT_STATE.PIPE[EX_STAGE] + (CURRENT_STATE.ID_EX_IMM << 2) + 4;
        CURRENT_STATE.EX_MEM_BR_TAKE = (op1 == op2);
        break;
    case 0x05: //(0x000101)BNE
        CURRENT_STATE.EX_MEM_BR_TARGET
            = CURRENT_STATE.PIPE[EX_STAGE] + (CURRENT_STATE.ID_EX_IMM << 2) + 4;
        CURRENT_STATE.EX_MEM_BR_TAKE = (op1 != op2);
        break;

    // R Format
    case 0x00:
        switch (func)
        {
        case 0x00: // (0x000000)SLL
            return op2 << shamt;
        case 0x02: // (0x000010)SRL
            return op2 >> shamt;
        case 0x08: // (0x001000)JR
            CURRENT_STATE.PIPE[ID_STAGE] = 0;
            break;
        case 0x21: // (0x100001)ADDU
            return op1 + op2;
        case 0x23: // (0x100011)SUBU
            return op1 - op2;
        case 0x24: // (0x100100)AND
            return op1 & op2;
        case 0x25: // (0x100101)OR
            return op1 | op2;
        case 0x27: // (0x100111)NOR
            return ~(op1 | op2);
        case 0x2b: // (0x101100)SLTU
            return (op1 < op2);
        default:
            printf ("Wrong instruction in ALU operation\n");
            exit (0);
        }
        break;

    // J Format
    case 0x02: // (0x000010)J
    case 0x03: // (0x000011)JAL
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        break;

    default:
        printf ("Wrong instruction in ALU operation\n");
        exit (0);
    }
    return 0;
}

void
hazard_detection_unit (instruction *instr)
{
    if (CURRENT_STATE.PIPE[EX_STAGE] != 0
        && OPCODE (get_inst_info (CURRENT_STATE.PIPE[EX_STAGE])) == 0x23)
    {
        if (CURRENT_STATE.EX_MEM_DEST == RS (instr)
            || CURRENT_STATE.EX_MEM_DEST == RT (instr))
        {
            CURRENT_STATE.PIPE_STALL[IF_STAGE] = TRUE;
            CURRENT_STATE.PIPE_STALL[ID_STAGE] = TRUE;
            CURRENT_STATE.PC_WRITE             = FALSE;
            return;
        }
    }
}

void
forwarding_unit (uint32_t *alu_input_1, uint32_t *alu_input_2)
{
    // RS
    if (CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_RS)
        *alu_input_1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
    else if (CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_RS)
        *alu_input_1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    else
        *alu_input_1 = CURRENT_STATE.ID_EX_REG1;

    // RT
    if (CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_RT)
        *alu_input_2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
    else if (CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_RT)
        *alu_input_2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    else
        *alu_input_2 = CURRENT_STATE.ID_EX_REG2;
}

///////////////////////// End of Helper functions /////////////////////////

void
propagate_pipeline (void)
{
    for (int i = 3; i >= 0; i--)
        if (!CURRENT_STATE.PIPE_STALL[i])
        {
            CURRENT_STATE.PIPE[i + 1] = CURRENT_STATE.PIPE[i];
            CURRENT_STATE.PIPE[i]     = 0;
        }

    CURRENT_STATE.EX_MEM_FORWARD_REG   = CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;
    CURRENT_STATE.MEM_WB_FORWARD_REG   = CURRENT_STATE.MEM_WB_DEST;
    CURRENT_STATE.MEM_WB_FORWARD_VALUE = (CURRENT_STATE.MEM_WB_MEM_OUT != 0)
                                             ? CURRENT_STATE.MEM_WB_MEM_OUT
                                             : CURRENT_STATE.MEM_WB_ALU_OUT;
    CURRENT_STATE.PC_WRITE             = TRUE;
    CURRENT_STATE.PC_SRC               = FETCH_SRC;
    CURRENT_STATE.JUMP_PC              = 0;
    CURRENT_STATE.BRANCH_PC            = 0;
    CURRENT_STATE.PIPE_STALL[IF_STAGE] = FALSE;
    CURRENT_STATE.PIPE_STALL[ID_STAGE] = FALSE;
    CURRENT_STATE.PIPE_STALL[EX_STAGE] = FALSE;
}

void
pipeline_WB_stage (void)
{
    if (CURRENT_STATE.PIPE[WB_STAGE] == 0)
        return;

    INSTRUCTION_COUNT++;

    if (CURRENT_STATE.MEM_WB_CTRL.REG_WRITE)
        CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST]
            = CURRENT_STATE.MEM_WB_CTRL.MEM_TO_REG ? CURRENT_STATE.MEM_WB_MEM_OUT
                                                   : CURRENT_STATE.MEM_WB_ALU_OUT;
}

void
pipeline_MEM_stage (void)
{
    uint32_t value;

    if (CURRENT_STATE.PIPE[MEM_STAGE] == 0)
    {
        CURRENT_STATE.MEM_WB_ALU_OUT = 0;
        CURRENT_STATE.MEM_WB_MEM_OUT = 0;
        CURRENT_STATE.MEM_WB_DEST    = 0;
        CURRENT_STATE.MEM_WB_CTRL    = EMPTY_CONTROL;
        return;
    }

    if (CURRENT_STATE.EX_MEM_CTRL.BRANCH && CURRENT_STATE.EX_MEM_BR_TAKE)
    {
        CURRENT_STATE.PC_WRITE       = FALSE;
        CURRENT_STATE.PC_SRC         = BRANCH_SRC;
        CURRENT_STATE.BRANCH_PC      = CURRENT_STATE.EX_MEM_BR_TARGET;
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
    }

    CURRENT_STATE.MEM_WB_MEM_OUT = (CURRENT_STATE.EX_MEM_CTRL.MEM_READ)
                                       ? mem_read_32 (CURRENT_STATE.EX_MEM_W_VALUE)
                                       : 0;

    if (CURRENT_STATE.EX_MEM_CTRL.MEM_WRITE)
    {
        value = (CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.EX_MEM_RT)
                    ? CURRENT_STATE.MEM_WB_FORWARD_VALUE
                    : CURRENT_STATE.REGS[CURRENT_STATE.EX_MEM_RT];
        mem_write_32 (CURRENT_STATE.EX_MEM_W_VALUE, value);
    }

    CURRENT_STATE.MEM_WB_DEST    = CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
    CURRENT_STATE.MEM_WB_CTRL    = CURRENT_STATE.EX_MEM_CTRL;
}

void
pipeline_EX_stage (void)
{
    uint32_t alu_input_1, alu_input_2;

    if (CURRENT_STATE.PIPE[EX_STAGE] == 0)
    {
        CURRENT_STATE.EX_MEM_ALU_OUT   = 0;
        CURRENT_STATE.EX_MEM_W_VALUE   = 0;
        CURRENT_STATE.EX_MEM_BR_TARGET = 0;
        CURRENT_STATE.EX_MEM_BR_TAKE   = FALSE;
        CURRENT_STATE.EX_MEM_RT        = 0;
        CURRENT_STATE.EX_MEM_DEST      = 0;
        CURRENT_STATE.EX_MEM_CTRL      = EMPTY_CONTROL;
        return;
    }

    forwarding_unit (&alu_input_1, &alu_input_2);

    if (CURRENT_STATE.ID_EX_CTRL.ALU_SRC)
        alu_input_2 = CURRENT_STATE.ID_EX_IMM;

    CURRENT_STATE.EX_MEM_BR_TAKE = FALSE;
    CURRENT_STATE.EX_MEM_RT      = CURRENT_STATE.ID_EX_RT;
    CURRENT_STATE.EX_MEM_DEST    = CURRENT_STATE.ID_EX_DEST;
    CURRENT_STATE.EX_MEM_ALU_OUT = alu_operation (alu_input_1, alu_input_2);
    CURRENT_STATE.EX_MEM_CTRL    = CURRENT_STATE.ID_EX_CTRL;
}

void
pipeline_ID_stage (void)
{
    instruction *instr;

    instr = get_inst_info (CURRENT_STATE.PIPE[ID_STAGE]);

    CURRENT_STATE.ID_EX_DEST = 0;

    if (CURRENT_STATE.PIPE[ID_STAGE] == 0)
    {
        CURRENT_STATE.ID_EX_REG1 = 0;
        CURRENT_STATE.ID_EX_REG2 = 0;
        CURRENT_STATE.ID_EX_IMM  = 0;
        CURRENT_STATE.ID_EX_RS   = 0;
        CURRENT_STATE.ID_EX_RT   = 0;
        CURRENT_STATE.ID_EX_DEST = 0;
        CURRENT_STATE.ID_EX_CTRL = EMPTY_CONTROL;
        return;
    }

    hazard_detection_unit (instr);

    CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[RS (instr)];
    CURRENT_STATE.ID_EX_RS   = RS (instr);
    CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[RT (instr)];
    CURRENT_STATE.ID_EX_RT   = RT (instr);
    CURRENT_STATE.ID_EX_CTRL = set_control_bits (instr);

    switch (CURRENT_STATE.ID_EX_CTRL.ALU_OP)
    {
    // R Format
    case 0x0:
        CURRENT_STATE.ID_EX_IMM = (SHAMT (instr) << 6 | FUNC (instr));
        switch (FUNC (instr))
        {
        case 0x8: //(0x001000)JR
            CURRENT_STATE.JUMP_PC = CURRENT_STATE.REGS[RS (instr)];
            CURRENT_STATE.PC_SRC  = JUMP_SRC;
            break;
        default:
            CURRENT_STATE.ID_EX_DEST = RD (instr);
            break;
        }
        break;

    // J Format
    case 0x03: //(0x000011)JAL
        CURRENT_STATE.REGS[31] = CURRENT_STATE.PIPE[ID_STAGE] + 4;
    case 0x02: //(0x000010)J
        CURRENT_STATE.ID_EX_IMM = SIGN_EX (IMM (instr));
        CURRENT_STATE.JUMP_PC   = TARGET (instr) << 2;
        CURRENT_STATE.PC_SRC    = JUMP_SRC;
        break;

    // I Format
    default:
        CURRENT_STATE.ID_EX_IMM  = SIGN_EX (IMM (instr));
        CURRENT_STATE.ID_EX_DEST = RT (instr);
        break;
    }
}

void
pipeline_IF_stage (void)
{
    if ((CURRENT_STATE.PC - MEM_TEXT_START) >= NUM_INST << 2)
        CURRENT_STATE.PC_WRITE = FALSE;

    if (CURRENT_STATE.PIPE_STALL[IF_STAGE] == FALSE)
    {
        switch (CURRENT_STATE.PC_WRITE)
        {
        case FALSE:
            CURRENT_STATE.PIPE[IF_STAGE] = 0;
            break;
        case TRUE:
            CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
            CURRENT_STATE.IF_PC          = CURRENT_STATE.PC + 4;
            break;
        }
    } else
        CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;

    switch (CURRENT_STATE.PC_SRC)
    {
    case FETCH_SRC:
        CURRENT_STATE.PC = CURRENT_STATE.IF_PC;
        break;
    case BRANCH_SRC:
        CURRENT_STATE.PC = CURRENT_STATE.BRANCH_PC;
        break;
    case JUMP_SRC:
        CURRENT_STATE.PC = CURRENT_STATE.JUMP_PC;
        break;
    }
}

void
halt_pipeline (void)
{
    if (CURRENT_STATE.PIPE[WB_STAGE] - MEM_TEXT_START == (NUM_INST - 1) << 2)
        if ((CURRENT_STATE.PIPE[IF_STAGE] | CURRENT_STATE.PIPE[ID_STAGE]
             | CURRENT_STATE.PIPE[EX_STAGE] | CURRENT_STATE.PIPE[MEM_STAGE])
            == 0)
            RUN_BIT = 0;
}

/***************************************************************
 *                                                             *
 * Procedure: process_instruction                              *
 *                                                             *
 * Purpose: Process one instrction                             *
 *                                                             *
 ***************************************************************/

void
process_instruction (void)
{
    propagate_pipeline ();

    pipeline_WB_stage ();
    pipeline_MEM_stage ();
    pipeline_EX_stage ();
    pipeline_ID_stage ();
    pipeline_IF_stage ();

    halt_pipeline ();
}
