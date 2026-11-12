#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

short flags = 0;
const short FLAGS_UNDERFLOWED = 0b0001;
const short FLAGS_OVERFLOWED = 0b0010;

short accumulator = 0;
short program_counter = 0;
short memory[100] = {};

int main(int argc, char **argv)
{
    char *input_filename = 0;
    if (argc > 1)
    {
        input_filename = argv[1];
    }
    else
    {
        printf("Error: no filename provided\n");
        exit(EXIT_FAILURE);
    }
    printf("Running %s\n", input_filename);

    for (int i = 2; i < argc; i++)
    {
        char *flag = argv[i];
        if (strcmp(flag, "-b"))
        {
        }
        else
        {
            printf("Unknown extra arguments\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE *in;
    if (!(in = fopen(input_filename, "rb")))
    {
        printf("Error opening input file!\n");
        exit(EXIT_FAILURE);
    }

    short instruction = 0;
    short location = 0;

    while (fread(&instruction, 2, 1, in))
    {
        memory[location++] = instruction;
        if (location >= 100)
        {
            break;
        }
    }

    while (true)
    {
        short instruction = memory[program_counter++];
        if ((instruction / 100) == 0) // HLT
        {
            exit(EXIT_SUCCESS);
        }
        else if ((instruction / 100) == 1) // ADD
        {
            short mailbox = instruction % 100;
            accumulator = accumulator + memory[mailbox];
            if (accumulator > 999)
            {
                accumulator -= 999;
                flags &= FLAGS_OVERFLOWED;
                flags &= ~FLAGS_UNDERFLOWED;
            }
        }
        else if ((instruction / 100) == 2) // SUB
        {
            short mailbox = instruction % 100;
            accumulator = accumulator - memory[mailbox];
            if (accumulator < 0)
            {
                accumulator += 999;
                flags &= FLAGS_UNDERFLOWED;
                flags &= ~FLAGS_OVERFLOWED;
            }
        }
        else if ((instruction / 100) == 3) // STA
        {
            short mailbox = instruction % 100;
            memory[mailbox] = accumulator;
        }
        else if ((instruction / 100) == 5) // LDA
        {
            short mailbox = instruction % 100;
            accumulator = memory[mailbox];
            flags = 0;
        }
        else if ((instruction / 100) == 6) // BRA
        {
            short mailbox = instruction % 100;
            program_counter = mailbox;
        }
        else if ((instruction / 100) == 7) // BRZ
        {
            short mailbox = instruction % 100;
            if ((accumulator == 0) && !(flags & FLAGS_UNDERFLOWED))
            {
                program_counter = mailbox;
            }
        }
        else if ((instruction / 100) == 8) // BRP
        {
            short mailbox = instruction % 100;
            if (!(flags & FLAGS_UNDERFLOWED))
            {
                program_counter = mailbox;
            }
        }
        else if (instruction == 901) // INP
        {
            printf("Input: ");
            long input = -1;
            char input_string[10];
            while (input == -1)
            {
                scanf("%lu", &input);
                if ((input > 999) || (input < 0))
                {
                    printf("Out of range [0, 999]\nInput: ");
                    input = -1;
                }
            }
            accumulator = (short)input;
            flags = 0;
        }
        else if (instruction == 902) // OUT
        {
            printf("Output: %03d\n", accumulator);
        }
        else
        {
            printf("Error: Unknown Instruction");
            exit(EXIT_FAILURE);
        }
    }
}