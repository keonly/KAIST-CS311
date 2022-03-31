/*
 ******************************************************************************
 *                               assembler.c                                  *
 *                              MIPS Assembler                                *
 *                      CS311: Computer Organisation                          *
 *                          20200434 Lee, Giyeon                              *
 ******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assembler.h"

////////////////////////////// Global Variables ///////////////////////////////

/**************************
 * Instruction Format Set *
 **************************/
char *instr_set[] = {
    // R Format Instructions
    "addu", // 0
    "and",  // 1
    "jr",   // 2
    "nor",  // 3
    "or",   // 4
    "sltu", // 5
    "sll",  // 6
    "srl",  // 7
    "subu", // 8
    // I Format Instructions
    "addiu", // 9
    "andi",  // 10
    "beq",   // 11
    "bne",   // 12
    "lui",   // 13
    "lw",    // 14
    "ori",   // 15
    "sltiu", // 16
    "sw",    // 17
    // J format Instructions
    "j",   // 18
    "jal", // 19
    // Pseudo-instruction
    "la" // 20
};

/***************************
 * Instruction to Hex Maps *
 ***************************/
struct
{
    const int instr;
    const int hex;
} instr_to_hex_map[] = {
    {0, 0x21},
    {1, 0x24},
    {2, 0x08},
    {3, 0x27},
    {4, 0x25},
    {5, 0x2B},
    {6, 0x00},
    {7, 0x02},
    {8, 0x23},
    {9, 0x09},
    {10, 0x0C},
    {11, 0x04},
    {12, 0x05},
    {13, 0x0F},
    {14, 0x23},
    {15, 0x0D},
    {16, 0x0B},
    {17, 0x2B},
    {18, 0x02},
    {19, 0x03},
    {NULL, NULL}
};

//////////////////////////// Function Definitions ////////////////////////////

line_type
parse_line(char *input)
{
    line_type type;
    if (is_data(input)) {
        type = DATA;
    } else if (is_text(input)) {
        type = TEXT;
    } else if (is_label(input)) {
        type = LABEL;
    } else if (is_word(input)) {
        type = WORD;
    } else {
        type = INSTR;
    }
    return type;
}

char *
strslice(char *str, int from, int to)
{
    char *substr;
    substr = malloc(sizeof(char) * (to - from + 1));

    strncpy(substr, str + from, to - from);
    substr[to - from] = '\0';
    return substr;
}

char *
instr_to_bin_string(char *input, int program_counter)
{
    int instruction;
    char *binary;
    ENTRY *found_label;

    found_label = (ENTRY *)malloc(sizeof(ENTRY));
    binary = (char *)malloc(64);
    instruction = in_instr_set(input);

    switch (check_instr_format(instruction))
    {
    case R_FORMAT:
    {
        r_format r_code;
        r_code.op = 0;
        if (instruction == 2) {
            /* jr instruction */
            scanf("%s", input);
            r_code.rs = reg_to_int(input);
            r_code.rt = 0;
            r_code.rd = 0;
            r_code.sh = 0;
        } else {
            scanf("%s", input);
            r_code.rd = reg_to_int(input);

            if (instruction == 6 || instruction == 7) {
                /* Shift Operations */
                r_code.rs = 0;
            } else {
                scanf("%s", input);
                r_code.rs = reg_to_int(input);
            }
            scanf("%s", input);
            r_code.rt = reg_to_int(input);
            if (instruction == 6 || instruction == 7) {
                /* Shift Operations */
                scanf("%s", input);
                r_code.sh = string_to_int(input);
            } else {
                r_code.sh = 0;
            }
        }
        r_code.fn = instr_to_hex_map[instruction].hex;
        binary = write_r_format(r_code);
        break;
    }
    case I_FORMAT:
    {
        i_format i_code;
        i_code.op = instr_to_hex_map[instruction].hex;
        scanf("%s", input);
        if (instruction == 11 || instruction == 12) {
            /* Branch */
            i_code.rs = reg_to_int(input);
            scanf("%s", input);
            i_code.rt = reg_to_int(input);
            scanf("%s", input);
            found_label = search_in_hash_table(input);

            i_code.im = (((int)(found_label->data - (program_counter + 4)) >> 2)) & 0xFFFF;
        } else if (instruction == 14 || instruction == 17) {
            /* Load/Save Words */
            i_code.rt = reg_to_int(input);
            scanf("%s", input);
            i_code.im = (string_to_int(strtok(input, "("))) & 0xFFFF;
            i_code.rs = reg_to_int(strtok(NULL, ")"));
        } else if (instruction == 13) {
            /* Load Unsigned Immediate */
            i_code.rs = 0;
            i_code.rt = reg_to_int(input);
            scanf("%s", input);
            i_code.im = string_to_int(input);
        } else if (
            instruction == 9 ||
            instruction == 10 ||
            instruction == 15 ||
            instruction == 16)
        {
            /* addiu, andi, ori, sltiu */
            i_code.rt = reg_to_int(input);
            scanf("%s", input);
            i_code.rs = reg_to_int(input);
            scanf("%s", input);
            i_code.im = string_to_int(input);
        } else {
            i_code.rs = reg_to_int(input);
            scanf("%s", input);
            i_code.rt = reg_to_int(input);
            scanf("%s", input);
            i_code.im = string_to_int(input);
        }
        binary = write_i_format(i_code);
        break;
    }
    case J_FORMAT:
    {
        j_format j_code;
        j_code.op = instr_to_hex_map[instruction].hex;
        scanf("%s", input);
        if ((found_label = search_in_hash_table(input)) != NULL) {
            j_code.addr = ((int)found_label->data >> 2);
        } else {
            j_code.addr = string_to_int(input);
        }
        binary = write_j_format(j_code);
        break;
    }
    case PSEUDO:
    {
        i_format lui_code;
        int immediate;

        lui_code.op = instr_to_hex_map[13].hex;
        lui_code.rs = 0;

        scanf("%s", input);
        lui_code.rt = reg_to_int(input);

        scanf("%s", input);
        found_label = search_in_hash_table(input);
        immediate = found_label->data;
        lui_code.im = (immediate >> 16) & 0xFFFF;

        if ((immediate & 0xFFFF) == 0) {
            binary = write_i_format(lui_code);
        } else {
            i_format ori_code;
            ori_code.op = instr_to_hex_map[15].hex;
            ori_code.rs = lui_code.rt;
            ori_code.rt = lui_code.rt;
            ori_code.im = immediate & 0xFFFF;
            binary = strcat(write_i_format(lui_code), write_i_format(ori_code));
        }
        break;
    }
    }
    return binary;
}

int in_instr_set(char *input)
{
    for (int i = 0; i < 21; i++)
    {
        if (!strcmp(input, instr_set[i]))
            return i;
    }
    return -1;
}

format_type
check_instr_format(int instr_num)
{
    format_type type;
    if (0 <= instr_num && instr_num <= 8) {
        type = R_FORMAT;
    } else if (9 <= instr_num && instr_num <= 17) {
        type = I_FORMAT;
    } else if (18 <= instr_num && instr_num <= 19) {
        type = J_FORMAT;
    } else if (instr_num == 20) {
        type = PSEUDO;
    }
    return type;
}

char *
int_to_bin_string(int num)
{
    char *string;
    string = (char *)malloc(33);

    for (int i = 31; i >= 0; i--)
    {
        string[i] = (num & 1) + '0';
        num >>= 1;
    }
    string[33] = '\0';
    return string;
}

int string_to_int(char *str)
{
    if (str[0] == '\t')
    {
        str[strlen(str) - 1] = '\0';
        str = str + 1;
    }
    if (strstr(str, "0x"))
        return (int)strtol(str, NULL, 16);
    else
        return atoi(str);
}

char *
separate_label_name(char *str)
{
    return strslice(str, 0, strlen(str) - strlen(strpbrk(str, ":")));
}

int reg_to_int(char *reg)
{
    if (reg[strlen(reg) - 1] == ',') {
        return atoi(strslice(reg, 1, strlen(reg) - 1));
    } else {
        return atoi(strslice(reg, 1, strlen(reg)));
    }
}

ENTRY *
search_in_hash_table(char *query)
{
    ENTRY *found_label;
    ENTRY search;

    found_label = (ENTRY *)malloc(sizeof(ENTRY));
    search.key = query;
    if ((found_label = hsearch(search, FIND)) != NULL) {
        return found_label;
    } else {
        return NULL;
    }
}

char *
write_r_format(r_format code)
{
    return int_to_bin_string((code.op << 26) + (code.rs << 21) + (code.rt << 16) + (code.rd << 11) + (code.sh << 6) + code.fn);
}

char *
write_i_format(i_format code)
{
    return int_to_bin_string((code.op << 26) + (code.rs << 21) + (code.rt << 16) + code.im);
}

char *
write_j_format(j_format code)
{
    return int_to_bin_string((code.op << 26) + code.addr);
}

bool is_lower_zero(char *input)
{
    ENTRY *found_label;
    found_label = (ENTRY *)malloc(sizeof(ENTRY));
    found_label = search_in_hash_table(input);
    return !(((int)found_label->data) & 0xFFFF);
}

void init_binary(char *binary)
{
    for (int i = 0; i < strlen(binary); i++)
    {
        binary[i] = '\0';
    }
}