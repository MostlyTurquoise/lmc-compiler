#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0

typedef enum
{
    MODE_TEXT,
    MODE_BINARY,
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

void writeOutChars(char c1, char c2, FILE *out, int *counter)
{
    fputc(c1, out);
    fputc(c2 >> 8, out);
    *counter++;
}

void writeOutShort(short val, FILE *out, int *counter)
{
    fputc(val, out);
    fputc(val >> 8, out);
    *counter++;
}

int lookupVariable(long *variableTable, int variableCount, long variable)
{
    for (int i = 0; i < variableCount; i += 2)
    {
        long varHash = variableTable[i];
        long varLoc = variableTable[i + 1];
        if (varHash == variable)
        {
            return varLoc;
        }
    }
    return -1;
}

const int MAX_LINE_LENGTH = 100;
const int MAX_VARIABLES = 100;

int main(int argc, char **argv)
{
    Mode assemblyMode = MODE_BINARY;
    char *filename = 0;
    char *outputFile = "out.lmb";
    if (argc > 1)
    {
        filename = argv[1];
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
            assemblyMode = MODE_TEXT;
        }
        else if (strcmp(flag, "-o") == 0)
        {
            outputFile = argv[++i];
        }
        else
        {
            printf("Unknown extra arguments\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE *in;
    if (!(in = fopen(filename, "r")))
    {
        printf("Error opening input file!\n");
        exit(EXIT_FAILURE);
    }

    FILE *out;
    if (!(out = fopen(outputFile, "wb")))
    {
        printf("Error opening output file\n");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    int counter = 0;
    long variables[MAX_VARIABLES * 2];
    int next_variable = 0;

    printf("Variable scan:\n");

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

        printf("Number: %i, First: %s, Second: %s, Third: %s\n", number, first, second, third);

        if (strcmp(first, "DAT") == 0)
        {
            if (*second)
            {
                char *end;
                unsigned short val = strtol(second, &end, 10);
                writeOutShort(val, out, &counter);
            }
            else
            {
                writeOutShort(0, out, &counter);
            }
        }
        else if (strcmp(second, "DAT") == 0)
        {
            printf("Dat with label\n");
            variables[next_variable++] = hash(first);
            variables[next_variable++] = (long)counter;
            if (*third)
            {
                char *end;
                unsigned short val = strtol(third, &end, 10);
                printf("%i", val);
                writeOutShort(val, out, &counter);
            }
            else
            {
                writeOutShort(0, out, &counter);
            }
        }
    }

    fseek(in, 0, SEEK_SET);

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

        if (strcmp(first, "DAT") == 0)
        {
            if (*second)
            {
                char *end;
                unsigned short val = strtol(second, &end, 10);
                writeOutShort(val, out, &counter);
            }
            else
            {
                writeOutShort(0, out, &counter);
            }
        }
        else if (strcmp(second, "DAT") == 0)
        {
            if (*third)
            {
                char *end;
                unsigned short val = strtol(third, &end, 10);
                printf("%i", val);
                writeOutShort(val, out, &counter);
            }
            else
            {
                writeOutShort(0, out, &counter);
            }
        }
        else if (strcmp(first, "INP") == 0 || strcmp(second, "INP"))
        {
        }
        else if (strcmp(first, "STA") == 0 || stcmp(second, "STA"))
        {
        }

        // fputs(line, out);
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