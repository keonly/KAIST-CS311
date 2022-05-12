/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.h                                                     */
/*                                                             */
/***************************************************************/

#ifndef _RUN_H_
#define _RUN_H_

#include "util.h"
#include <stdio.h>

#define OPCODE(INST) (INST)->opcode
#define SET_OPCODE(INST, VAL) (INST)->opcode = (short) (VAL)

#define FUNC(INST) (INST)->func_code
#define SET_FUNC(INST, VAL) (INST)->func_code = (short) (VAL)

#define RS(INST) (INST)->r_t.r_i.rs
#define SET_RS(INST, VAL) (INST)->r_t.r_i.rs = (unsigned char) (VAL)

#define RT(INST) (INST)->r_t.r_i.rt
#define SET_RT(INST, VAL) (INST)->r_t.r_i.rt = (unsigned char) (VAL)

#define RD(INST) (INST)->r_t.r_i.r_i.r.rd
#define SET_RD(INST, VAL) (INST)->r_t.r_i.r_i.r.rd = (unsigned char) (VAL)

#define SHAMT(INST) (INST)->r_t.r_i.r_i.r.shamt
#define SET_SHAMT(INST, VAL) (INST)->r_t.r_i.r_i.r.shamt = (unsigned char) (VAL)

#define IMM(INST) (INST)->r_t.r_i.r_i.imm
#define SET_IMM(INST, VAL) (INST)->r_t.r_i.r_i.imm = (short) (VAL)

#define BASE(INST) RS (INST)
#define SET_BASE(INST, VAL) SET_RS (INST, VAL)

#define IOFFSET(INST) IMM (INST)
#define SET_IOFFSET(INST, VAL) SET_IMM (INST, VAL)
#define IDISP(INST) (SIGN_EX (IOFFSET (INST) << 2))

#define COND(INST) RS (INST)
#define SET_COND(INST, VAL) SET_RS (INST, VAL)

#define TARGET(INST) (INST)->r_t.target
#define SET_TARGET(INST, VAL) (INST)->r_t.target = (uint32_t) (VAL)

/* Sign Extension */
#define SIGN_EX(X) (((X) &0x8000) ? ((X) | 0xffff0000) : (X))

/* Minimum and maximum values that fit in instruction's imm field */
#define IMM_MIN 0xffff8000
#define IMM_MAX 0x00007fff
#define UIMM_MIN (unsigned) 0
#define UIMM_MAX ((unsigned) ((1 << 16) - 1))

#define BRANCH_INST(TEST, TARGET, NULLIFY)                                          \
    {                                                                               \
        if (TEST)                                                                   \
        {                                                                           \
            uint32_t target = (TARGET);                                             \
            JUMP_INST (target)                                                      \
        }                                                                           \
    }

#define JUMP_INST(TARGET)                                                           \
    {                                                                               \
        CURRENT_STATE.PC = (TARGET);                                                \
    }

#define LOAD_INST(DEST_A, LD, MASK)                                                 \
    {                                                                               \
        LOAD_INST_BASE (DEST_A, (LD & (MASK)))                                      \
    }

#define LOAD_INST_BASE(DEST_A, VALUE)                                               \
    {                                                                               \
        *(DEST_A) = (VALUE);                                                        \
    }

/* functions */
instruction *get_inst_info (uint32_t pc);
void process_instruction ();
void extract (int pc,
              uint32_t *op,
              uint32_t *rs,
              uint32_t *rt,
              uint32_t *rd,
              uint32_t *shamt,
              uint32_t *func,
              uint32_t *imm,
              uint32_t *target);
void pipeline_IF_stage ();
void pipeline_ID_stage ();
void pipeline_EX_stage ();
void pipeline_MEM_stage ();
void pipeline_WB_stage ();
#endif
