#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 128

int main(void)
{
    FILE *file = fopen("temp.txt", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
        return 1;
    }

    // write numbers
    for (int i = 0; i < MAX_PROCESSES * 2; i++)
    {
        fprintf(file, "%d\n", i + 1);
    }

    // close file
    fclose(file);

    return 0;
}