#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0

typedef enum
{
    MODE_TEXT,
    MODE_BINARY
} Mode;

// http://www.cse.yorku.ca/~oz/hash.html
// djb2 by dan bernstein
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
    {

        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

void write_out_chars(char c1, char c2, FILE *out, int *counter)
{
    fputc(c1, out);
    fputc(c2 >> 8, out);
    *counter++;
}

void write_out_short(short val, FILE *out, int *counter)
{
    fputc(val, out);
    fputc(val >> 8, out);
    *counter++;
}

int lookup_label(long *labelTable, int label_count, long labelHash)
{
    // printf("Searching for %ld\n", labelHash);
    for (int i = 0; i < label_count; i += 2)
    {
        long currentHash = labelTable[i];
        long currentLoc = labelTable[i + 1];
        // printf("Got %ld at %ld\n", currentHash, currentLoc);
        if (currentHash == labelHash)
        {
            return currentLoc;
        }
    }
    return -1;
}

// char *ALL_INSTRUCTIONS[] = {"DAT", "LDA", "STA", "ADD", "SUB", "INP", "OUT", "HLT", "BRA", "BRZ", "BRP", 0};
char *ALL_INSTRUCTIONS[] = {"DAT", "LDA", "STO", "ADD", "SUB", "IN", "OUT", "HLT", "BR", "BRZ", "BRP", "P_BR", "DDAT", 0};

int is_instruction(char *word)
{
    int current = 0;
    while (ALL_INSTRUCTIONS[current] != 0)
    {
        if (strcmp(word, ALL_INSTRUCTIONS[current++]) == 0)
        {
            return true;
        }
    }
    return false;
}

const int MAX_LINE_LENGTH = 100;
const int MAX_VARIABLES = 100;

int main(int argc, char **argv)
{
    Mode assembly_mode = MODE_TEXT;
    char *input_filename = 0;
    char *output_filename = "out.lmb";
    if (argc > 1)
    {
        input_filename = argv[1];
    }
    else
    {
        printf("Error: no filename provided\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 2; i < argc; i++)
    {
        char *flag = argv[i];
        // printf("Got arg '%s'\n", flag);
        if (strcmp(flag, "-t") == 0)
        {
            assembly_mode = MODE_TEXT;
        }
        else if (strcmp(flag, "-o") == 0)
        {
            // printf("Output filename provided");
            output_filename = argv[++i];
        }
        else
        {
            printf("Unknown extra arguments\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE *in;
    if (!(in = fopen(input_filename, "r")))
    {
        printf("Error opening input file!\n");
        exit(EXIT_FAILURE);
    }

    FILE *out;
    if (!(out = fopen(output_filename, "w")))
    {
        printf("Error opening output file\n");
        exit(EXIT_FAILURE);
    }

    long labels[MAX_VARIABLES * 2];
    int label_count = 0;

    char line[MAX_LINE_LENGTH];
    int counter = 0;

    printf("Labels ");
    // Find all variable locations
    while (fgets(line, sizeof(line), in))
    {
        char first[MAX_LINE_LENGTH];
        char second[MAX_LINE_LENGTH];
        char third[MAX_LINE_LENGTH];

        int number = sscanf(line, "%s %s %s\n", first, second, third);

        if (number < 3)
        {
            *third = 0; // set null byte at first index of string
        }
        if (number < 2)
        {
            *second = 0; // set null byte at first index of string
        }
        if (number < 1)
        {
            *first = 0;
        }

        if ((first[0] == '#') || (*first == 0))
        {
            // printf("Skipping comment\n"); // notably counter isnt incremented
            continue;
        }

        // printf("Label Pass: Number: %i, First: %s, Second: %s\n", number, first, second);

        if (is_instruction(second) && !(first[0] == '#'))
        {
            printf("%s:%i, ", first, counter);
            labels[label_count++] = hash(first);
            labels[label_count++] = (long)counter;
        }

        if ((strcmp(first, "P_BR") == 0) || (strcmp(second, "P_BR") == 0))
        {
            counter += 4;
        }
        else if ((strcmp(first, "DDAT") == 0) || (strcmp(second, "DDAT") == 0))
        {
        }
        else
        {
            counter++;
        }
    }

    printf("\n");
    fseek(in, 0, SEEK_SET);
    counter = 0;

    char variables[100][100] = {};
    int no_of_variables = 0;

    printf("Labels found, writing final output\n");

    while (fgets(line, sizeof(line), in))
    {
        char first[MAX_LINE_LENGTH];
        char second[MAX_LINE_LENGTH];
        char third[MAX_LINE_LENGTH];

        int number = sscanf(line, "%s %s %s\n", first, second, third);

        if (number < 3)
        {
            *third = 0; // set null byte at first index of string
        }
        if (number < 2)
        {
            *second = 0; // set null byte at first index of string
        }
        if (number < 1)
        {
            *first = 0; // set null byte at first index of string
        }

        // printf("Number: %i, First: %s, Second: %s, Third: %s\n", number, first, second, third);

        if ((first[0] == '#') || (*first == 0))

        {
            // printf("Skipping comment\n");
            fprintf(out, "%s", line);
            continue;
        }

        char *opcode = first;
        char *operand = second;
        char *label = "";
        if (is_instruction(second))
        {
            opcode = second;
            operand = third;
            label = first;
        }

        if (strcmp(opcode, "DAT") == 0)
        {
            if (*operand)
            {
                char *end;
                long val = strtol(operand, &end, 10);
                if (operand == end) // If there was no number
                {
                    // it must be a label
                    val = lookup_label(labels, label_count, hash(operand));
                }
                // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
                if (val == -1)
                {
                    printf("Error: Invalid DAT operand, '%s' is not a label or number literal\n", operand);
                    exit(EXIT_FAILURE);
                }

                if (val < 0 || val > 999)
                {
                    printf("Error: Invalid DAT operand, '%ld' is not in the range [0, 999]", val);
                    exit(EXIT_FAILURE);
                }
                fprintf(out, "%s\t%s\t%li\n", label, opcode, val);
                counter++;
            }
            else
            {
                fprintf(out, "%s\t%s\t%s\n", label, opcode, operand);
                counter++;
            }
        }
        else if (strcmp(opcode, "DDAT") == 0)
        {
            fprintf(out, "# %s was moved from here\n", label);
            char varline[100];
            snprintf(varline, 100, "%s\tDAT\t%s\n", label, operand);
            strcpy(variables[no_of_variables++], varline);
        }
        else if (strcmp(opcode, "P_BR") == 0)
        {
            fprintf(out, "# START P_BR\n");
            fprintf(out, "%s\tLDA\t%s\n", label, operand);
            fprintf(out, "\tADD\tBR_NUM\n");
            counter += 2;
            fprintf(out, "\tSTO\t__lat%i\n", counter + 1);
            fprintf(out, "__lat%i\tDAT\n", counter + 1);
            counter += 2;
            fprintf(out, "# END P_BR\n");
        }
        else
        {
            if (operand[0] == '&')
            {
                fprintf(out, "# %s was declared for this\n", operand);
                long operand_label_location = lookup_label(labels, label_count, hash(operand + 1));
                char varline[100];
                snprintf(varline, 100, "__p_%s\tDAT\t%li\n", operand + 1, operand_label_location);
                strcpy(variables[no_of_variables++], varline);
                fprintf(out, "%s\t%s\t__p_%s\n", label, opcode, operand + 1);
            }
            else
            {
                fprintf(out, "%s\t%s\t%s\n", label, opcode, operand);
            }
            counter++;
        }
    }
    fprintf(out, "# Deref-generated constants\n");
    fprintf(out, "BR_NUM\tDAT\t600\n");

    fprintf(out, "# Delayed DATs\n");
    for (int i = 0; i < no_of_variables; i++)
    {
        fprintf(out, "%s", variables[i]);
    }

    printf("\n\n");

    fclose(in);
    fclose(out);

    return 0;
}