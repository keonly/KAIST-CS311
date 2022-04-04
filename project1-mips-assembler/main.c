/*
 ******************************************************************************
 *                                  main.c                                    *
 *                              MIPS Assembler                                *
 *                      CS311: Computer Organisation                          *
 *                          20200434 Lee, Giyeon                              *
 ******************************************************************************
 */

#include "assembler.h"
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/////////////////////////// Function Declarations ///////////////////////////

void first_pass (FILE *, flags *, char *);
void second_pass (FILE *, flags *, char *);
void third_pass (FILE *, flags *, char *);

//////////////////////////// Function Definitions ////////////////////////////

void
first_pass (FILE *file_ptr, flags *flags, char *binary)
{
    char *input;
    int   data_size, text_size;
    ENTRY label;

    input = (char *) malloc (MAX_LEN);

    while (1) {
        scanf ("%s", input);
        if (feof (file_ptr))
            break;

        switch (parse_line (input)) {
        case DATA:
            flags->program_counter = DATA_ADDR;
            flags->current_region  = DATA_REGION;
            break;

        case TEXT:
            data_size              = flags->program_counter - DATA_ADDR;
            flags->program_counter = TEXT_ADDR;
            flags->current_region  = TEXT_REGION;
            break;

        case LABEL:
            label.key  = separate_label_name (input);
            label.data = flags->program_counter;
            hsearch (label, ENTER);
            if (flags->current_region == DATA_REGION) {
                fgets (input, MAX_LEN - 1, stdin);
                flags->program_counter += 4;
            }
            break;

        case WORD:
            /* Do not care about the word details for now */
            fgets (input, MAX_LEN - 1, stdin);
            flags->program_counter += 4;
            break;

        case INSTR:
            if (check_instr_format (in_instr_set (input)) == PSEUDO) {
                scanf ("%s", input);
                scanf ("%s", input);
                if (!is_lower_zero (input))
                    flags->program_counter += 4;
            }
            /* Do not care about the instruction details for now */
            fgets (input, MAX_LEN - 1, stdin);
            flags->program_counter += 4;
            break;
        }
    }
    text_size = flags->program_counter - TEXT_ADDR;
    strcat (binary, int_to_bin_string (text_size));
    strcat (binary, int_to_bin_string (data_size));
    flags->program_counter = 0;
    free (input);
}

void
second_pass (FILE *file_ptr, flags *flags, char *binary)
{
    char *input;
    int   instruction;
    int   temp;

    input = (char *) malloc (MAX_LEN);
    while (1) {
        scanf ("%s", input);
        if (feof (file_ptr))
            break;
        if (!strcmp (input, "\n"))
            break;
        switch (parse_line (input)) {
        case DATA:
            flags->current_region = DATA_REGION;
            break;

        case TEXT:
            flags->current_region  = TEXT_REGION;
            flags->program_counter = TEXT_ADDR;
            break;

        case INSTR:
            if (flags->current_region == DATA_REGION)
                break;
            temp = strlen (binary);
            strcat (binary, instr_to_bin_string (input, flags->program_counter));
            flags->program_counter += (strlen (binary) - temp) / 8;
            break;

        case WORD:
            fgets (input, MAX_LEN - 1, stdin);

        default:
            break;
        }
    }
    free (input);
}

void
third_pass (FILE *file_ptr, flags *flags, char *binary)
{
    char * input;
    ENTRY *found_label;
    int    temp;

    found_label           = (ENTRY *) malloc (sizeof (ENTRY));
    input                 = (char *) malloc (MAX_LEN);
    flags->current_region = DATA_REGION;

    while (1) {
        scanf ("%s", input);
        if (feof (file_ptr))
            break;
        if (flags->current_region == TEXT_REGION)
            break;
        switch (parse_line (input)) {
        case TEXT:
            free (input);
            return;

        case WORD:
            fgets (input, MAX_LEN, stdin);
            temp = string_to_int (input);
            strncat (binary, int_to_bin_string (temp), 32);
            break;

        default:
            break;
        }
    }
}

int
main (int argc, char *argv[])
{

    if (argc != 2) {
        printf ("Usage: ./runfile <assembly file>\n");
        // Example) ./runfile /sample_input/example1.s
        printf ("Example) ./runfile ./sample_input/example1.s\n");
        exit (0);
    } else {

        /*
         * To help you handle the file IO, the deafult code is provided.
         * If we use freopen, we don't need to use fscanf, fprint,..etc.
         * You can just use scanf or printf function
         * ** You don't need to modify this part **
         * If you are not famailiar with freopen,  you can see the following reference
         * http://www.cplusplus.com/reference/cstdio/freopen/
         */

        char * filename;
        FILE * file_ptr;
        flags *flags;
        char * binary;

        hcreate (MAX_LABELS);
        flags  = malloc (sizeof (struct _flags));
        binary = (char *) malloc (1000000);

        filename = (char *) malloc (strlen (argv[1]) + 3);
        strncpy (filename, argv[1], strlen (argv[1]));

        if ((file_ptr = freopen (filename, "r", stdin)) == 0) {
            printf ("File open Error!\n");
            exit (1);
        }

        init_binary (binary);

        /*
         * First Read
         * Save label infos, write text and data size
         */
        first_pass (file_ptr, flags, binary);

        /* Reset file pointers */
        if ((file_ptr = freopen (filename, "r", stdin)) == 0) {
            printf ("File open Error!\n");
            exit (1);
        }

        /*
         * Second Read
         * Write instruction binaries
         */
        second_pass (file_ptr, flags, binary);

        /* Reset file pointers */
        if ((file_ptr = freopen (filename, "r", stdin)) == 0) {
            printf ("File open Error!\n");
            exit (1);
        }

        /*
         * Third Read
         * Write word data
         */
        third_pass (file_ptr, flags, binary);

        filename[strlen (filename) - 1] = 'o';
        freopen (filename, "w", stdout);

        fflush (stdout);

        printf ("%s", binary);
    }
    return 0;
}
