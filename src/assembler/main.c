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

char *ALL_INSTRUCTIONS[] = {"DAT", "LDA", "STA", "ADD", "SUB", "INP", "OUT", "HLT", "BRA", "BRZ", "BRP", 0};

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
    Mode assembly_mode = MODE_BINARY;
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
        if (strcmp(flag, "-t"))
        {
            assembly_mode = MODE_TEXT;
        }
        else if (strcmp(flag, "-o") == 0)
        {
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
    if (!(out = fopen(output_filename, "wb")))
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

        if (first[0] == '#')
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

        counter++;
    }

    printf("\n");
    fseek(in, 0, SEEK_SET);
    counter = 0;

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

        // printf("Number: %i, First: %s, Second: %s, Third: %s\n", number, first, second, third);

        if (first[0] == '#')
        {
            // printf("Skipping comment\n");
            continue;
        }

        char *opcode = first;
        char *operand = second;
        if (is_instruction(second))
        {
            opcode = second;
            operand = third;
        }

        if (strcmp(opcode, "DAT") == 0)
        {
            if (*operand)
            {
                char *end;
                unsigned short val = strtol(operand, &end, 10);
                write_out_short(val, out, &counter);
            }
            else
            {
                write_out_short(0, out, &counter);
            }
        }
        else if (strcmp(opcode, "INP") == 0)
        {
            write_out_short(901, out, &counter);
        }
        else if (strcmp(opcode, "OUT") == 0)
        {
            write_out_short(902, out, &counter);
        }
        else if (strcmp(opcode, "HLT") == 0)
        {
            write_out_short(000, out, &counter);
        }
        else if (strcmp(opcode, "STA") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid STA operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid STA operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 300 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
        else if (strcmp(opcode, "LDA") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid LDA operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid LDA operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 500 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
        else if (strcmp(opcode, "ADD") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid ADD operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid ADD operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 100 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
        else if (strcmp(opcode, "SUB") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid SUB operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid SUB operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 200 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
        else if (strcmp(opcode, "BRZ") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid BRZ operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid BRZ operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 700 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
        else if (strcmp(opcode, "BRP") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid BRP operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid BRP operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 800 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
        else if (strcmp(opcode, "BRA") == 0)
        {
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid BRA operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }

            if (label_instruction_location < 0 || label_instruction_location > 99)
            {
                printf("Error: Invalid BRA operand, '%ld' is not in the range [0, 99]", label_instruction_location);
                exit(EXIT_FAILURE);
            }

            short value = 600 + label_instruction_location;
            write_out_short(value, out, &counter);
        }
    }

    printf("\n\n");

    return 0;
}