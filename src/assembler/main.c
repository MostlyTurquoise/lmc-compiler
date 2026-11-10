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
    printf("Searching for %ld\n", labelHash);
    for (int i = 0; i < label_count; i += 2)
    {
        long currentHash = labelTable[i];
        long currentLoc = labelTable[i + 1];
        printf("Got %ld at %ld\n", currentHash, currentLoc);
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

    printf("Label scan:\n");

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

        // printf("Label Pass: Number: %i, First: %s, Second: %s\n", number, first, second);

        if (is_instruction(second))
        {
            printf("Found label %s at %i (instruction %s %s)\n", first, counter, second, third);
            labels[label_count++] = hash(first);
            labels[label_count++] = (long)counter;
        }

        counter++;
    }

    fseek(in, 0, SEEK_SET);
    counter = 0;

    printf("Output scan:\n");

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

        printf("Number: %i, First: %s, Second: %s, Third: %s\n", number, first, second, third);

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
                unsigned short val = strtol(second, &end, 10);
                write_out_short(val, out, &counter);
            }
            else
            {
                write_out_short(0, out, &counter);
            }
        }
        else if (strcmp(opcode, "INP") == 0)
        {
        }
        else if (strcmp(opcode, "STA") == 0)
        {
            printf("Got STA\n");
            char *end;
            long label_instruction_location = strtol(operand, &end, 10);
            if (operand == end) // If there was no number
            {
                printf("Must be a label\n");
                // it must be a label
                label_instruction_location = lookup_label(labels, label_count, hash(operand));
            }
            // label_instruction_location is now either a valid number, or 0 if it couldnt be parsed
            if (label_instruction_location == -1)
            {
                printf("Error: Invalid STA operand, '%s' is not a label or mailbox literal\n", operand);
                exit(EXIT_FAILURE);
            }
            printf("Target: %i, %i =? %i\n", label_instruction_location, operand, end);
        }

        counter++;
    }

    // const int BUFFER_SIZE = 1024;
    // char buffer[BUFFER_SIZE];
    // if (!fgets(buffer, sizeof(buffer), in))
    // {
    //     printf("Error reading file\n");
    //     return 1;
    // }

    // printf("%s\n", buffer);

    // char *currentlyAt = buffer;

    // while ((buffer + BUFFER_SIZE) - currentlyAt > 1)
    // {
    //     char line[1024];
    //     int amountRead;

    //     sscanf(currentlyAt, "%[^\n]%n", line, &amountRead);
    //     printf("Line %i: %s\n", amountRead, buffer);
    //     currentlyAt += amountRead;
    // }

    printf("\n\n");

    return 0;
}