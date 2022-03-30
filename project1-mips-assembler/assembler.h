/*
 ******************************************************************************
 *                               assembler.h                                  *
 *                              MIPS Assembler                                *
 *                      CS311: Computer Organisation                          *
 *                          20200434 Lee, Giyeon                              *
 ******************************************************************************
 */
#include <search.h>
#include <stdbool.h>

/////////////////////////////////// Macros ////////////////////////////////////

/*******************
 * Deafult Address *
 *******************/
 #define TEXT_ADDR 0x400000
 #define DATA_ADDR 0x10000000

 #define DATA_REGION 0
 #define TEXT_REGION 1

/***************************
 * Maximum Length per Line *
 ***************************/
 #define MAX_LEN 100

 /****************************
  * Maximum Number of Labels *
  ****************************/
 #define MAX_LABELS 100

////////////////////////////// Global Variables ///////////////////////////////

/********************
 * Format Indicator *
 ********************/
typedef enum {
    R_FORMAT,
    I_FORMAT,
    J_FORMAT,
    PSEUDO
} format_type;

/***********************
 * Line Type Indicator *
 ***********************/
typedef enum {
    END,
    DATA,
    TEXT,
    LABEL,
    WORD,
    INSTR
} line_type;

/////////////////////////////////// Structs ///////////////////////////////////

typedef struct _R_format {
    int op; // 6 bits
    int rs; // 5 bits
    int rt; // 5 bits
    int rd; // 5 bits
    int sh; // 5 bits
    int fn; // 6 bits
} r_format;

typedef struct _I_format {
    int op; // 6 bits
    int rs; // 5 bits
    int rt; // 5 bits
    int im; // 16 bits
} i_format;

typedef struct _J_format {
    int op;   // 6 bits
    int addr; // 26 bits
} j_format;

/*********
 * Flags *
 *********/
typedef struct _flags {
    int program_counter;
    int current_region;
} flags;

////////////////////////////// Inline Functions //////////////////////////////

static inline bool
is_eof (char *input)
{
    return !(strcmp (input, "\n"));
}

static inline bool
is_data (char *input)
{
    return !(strcmp (input, ".data"));
}

static inline bool
is_text (char *input)
{
    return !(strcmp (input, ".text"));
}

static inline bool
is_label (char *input)
{
    return (input[strlen (input) - 1] == ':');
}

static inline bool
is_word (char *input)
{
    return !(strcmp (input, ".word"));
}

/////////////////////////// Function Declarations ///////////////////////////

line_type parse_line (char *);
char *strslice (char *, int, int);

char *instr_to_bin_string (char *, int);

int in_instr_set (char *);
format_type check_instr_format (int);

char *int_to_bin_string (int);
int string_to_int (char *);

char *separate_label_name (char *);

ENTRY *search_in_hash_table (char *);

char *write_r_format (r_format);
char *write_i_format (i_format);
char *write_j_format (j_format);

bool is_lower_zero (char *);
void init_binary (char *);