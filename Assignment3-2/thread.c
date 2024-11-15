#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSES 128

typedef struct
{
    int num1;
    int num2;
    int *result;
} ThreadData;

void *thread_func(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    *data->result = data->num1 + data->num2;
    pthread_exit(NULL);
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
    int results[MAX_PROCESSES];
    pthread_t threads[MAX_PROCESSES];
    ThreadData thread_data[MAX_PROCESSES];

    // calculate the sum of numbers
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        thread_data[i].num1 = numbers[i * 2];
        thread_data[i].num2 = numbers[i * 2 + 1];
        thread_data[i].result = &results[i];
        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }

    // compute the final result
    int final_result = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        pthread_join(threads[i], NULL);
        final_result += results[i];
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