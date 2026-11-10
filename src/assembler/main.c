#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0

const char *FILENAME = "test.txt";

typedef enum
{
    MODE_TEXT,
    MODE_BINARY,
} Mode;

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

    char line[100];
    while (fgets(line, sizeof(line), in))
    {
        char first[100];
        char second[100];
        char third[100];

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
                short val = strtol(second, &end, 10);
                fputc(val, out);
            }
            else
            {
                fputc(0, out);
            }
        }
        else if (strcmp(second, "DAT") == 0)
        {
            printf("Dat with label\n");
            if (*third)
            {
                char *end;
                short val = strtol(third, &end, 10);
                fputc(val, out);
            }
            else
            {
                fputc(0, out);
            }
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

    return 0;
}