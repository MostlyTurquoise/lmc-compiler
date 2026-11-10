#include <stdio.h>

#define true 1

const char *FILENAME = "test.txt";

int main(int argc, char **argv)
{
    char *filename = 0;
    if (argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        printf("Error: no filename provided\n");
        return 1;
    }

    FILE *in;
    if (!(in = fopen(filename, "rt")))
    {
        printf("Error opening file!\n");
        return 1;
    }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), in))
    {
        printf("Error reading file\n");
        return 1;
    }

    printf("%s\n", buffer);

    char *currentlyAt = buffer;

    while (true)
    {
        char line[1024];

        sscanf(currentlyAt, "%[^\n]", line);
        printf("Line: %s\n", buffer);
    }

    return 0;
}