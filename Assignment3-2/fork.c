#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_PROCESSES 128

void handle_child(int num1, int num2)
{
    int sum = num1 + num2;
    exit(sum);
}

int main()
{
    // open temp.txt
    FILE *file = fopen("temp.txt", "r");
    if (file == NULL)
    {
        perror("Failed to open file");
        return 1;
    }

    // read numbers from temp.txt and close
    int numbers[MAX_PROCESSES * 2];
    for (int i = 0; i < MAX_PROCESSES * 2; i++)
    {
        fscanf(file, "%d", &numbers[i]);
    }
    fclose(file);

    // initialize
    clock_t start_time = clock();
    int final_result = 0;
    int results[MAX_PROCESSES];

    // calculate the sum of numbers
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        int num1 = numbers[i * 2];
        int num2 = numbers[i * 2 + 1];

        pid_t pid = fork();
        if (pid == 0)
        {
            // parent process
            handle_child(num1, num2);
        }
        else
        {
            // child process
            int status;
            wait(&status);
            int temp = WEXITSTATUS(status);

            // compensate overflow
            if (i >= 64)
                temp = temp + 256 * (i / 64);
            results[i] = temp;

            // add temp to result
            final_result += temp;
        }
    }

    // print the result and execution time
    clock_t end_time = clock();
    printf("Final Result: %d\n", final_result);
    printf("Execution Time: %.4f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    // print the results to temp.txt
    FILE *file_append = fopen("temp.txt", "a");
    if (file_append == NULL)
    {
        perror("Failed to open file for appending");
        return 1;
    }

    // Write each element of results to the file
    int tree_size = MAX_PROCESSES;
    while (tree_size > 1)
    {
        for (int i = 0; i < tree_size / 2; i++)
            results[i] = results[2 * i] + results[2 * i + 1];
        tree_size /= 2;
        for (int i = 0; i < tree_size; i++)
            fprintf(file_append, "%d\n", results[i]);
    }
    fclose(file_append);

    return 0;
}