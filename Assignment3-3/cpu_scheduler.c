#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 1000
#define CONTEXT_SWITCH_TIME 0.1

typedef struct
{
    int pid;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int startTime;
    int endTime;
    int waitingTime;
    int turnaroundTime;
    int responseTime;
    int isCompleted;
} Task;

Task tasks[MAX_TASKS];
int taskCount = 0;
int contextSwitchCount = 0;
int timeQuantum = 0;

void readTasks(const char *filename);
void printResult(double totalTime);
void FCFS();
void RR();
void SJF();
void SRTF();

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        perror("Invalid arguments: cpu_simulator [input_file] [FCFS|RR|SJF|SRTF] [time_quantum]\n");
        return -1;
    }

    // read tasks from input file
    readTasks(argv[1]);

    // perform cpu scheduler
    if (strcmp(argv[2], "FCFS") == 0)
    {
        FCFS();
    }
    else if (strcmp(argv[2], "RR") == 0)
    {
        if (argc < 4)
        {
            perror("RR requires time quantum.\n");
            return -1;
        }
        timeQuantum = atoi(argv[3]);
        RR();
    }
    else if (strcmp(argv[2], "SJF") == 0)
    {
        SJF();
    }
    else if (strcmp(argv[2], "SRTF") == 0)
    {
        SRTF();
    }
    else
    {
        perror("Unknown cpu scheduling argv[2].\n");
        return -1;
    }

    return 0;
}

void readTasks(const char *filename)
{
    // open input file
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open file.\n");
        exit(1);
    }

    // read task info - pid, arrivalTime, burstTime
    while (fscanf(file, "%d %d %d", &tasks[taskCount].pid, &tasks[taskCount].arrivalTime,
                  &tasks[taskCount].burstTime) == 3)
    {
        tasks[taskCount].remainingTime = tasks[taskCount].burstTime;
        tasks[taskCount].isCompleted = 0;
        taskCount++;
    }

    // close file
    fclose(file);
}

void printResult(double totalTime)
{
    double totalWatingTime = 0;
    double totalTurnaroundTime = 0;
    double totalResponseTime = 0;

    // add tasks' time info
    for (int i = 0; i < taskCount; i++)
    {
        totalWatingTime += tasks[i].waitingTime;
        totalTurnaroundTime += tasks[i].turnaroundTime;
        totalResponseTime += tasks[i].responseTime;
    }

    // print simulation result
    // printf("Total Time = %.2f\n", totalTime);
    // printf("Context Switching Count: %d\n", contextSwitchCount);
    printf("Average Waiting Time = %.2f\n", totalWatingTime / taskCount);
    printf("Average Turnaround Time = %.2f\n", totalTurnaroundTime / taskCount);
    printf("Average Response Time = %.2f\n", totalResponseTime / taskCount);
    printf("CPU Utilization = %.2f%%\n", (totalTime / (totalTime + CONTEXT_SWITCH_TIME * contextSwitchCount)) * 100);
}

void FCFS()
{
    double totalTime = 0;

    printf("Gantt Chart:\n");
    for (int i = 0; i < taskCount; i++)
    {
        if (totalTime < tasks[i].startTime - tasks[i].arrivalTime)
            totalTime = tasks[i].arrivalTime;

        // perform tasks
        tasks[i].startTime = totalTime;
        tasks[i].waitingTime = tasks[i].startTime - tasks[i].arrivalTime;
        tasks[i].responseTime = tasks[i].waitingTime;
        tasks[i].endTime = tasks[i].startTime + tasks[i].burstTime;
        tasks[i].turnaroundTime = tasks[i].endTime - tasks[i].arrivalTime;

        for (int j = 0; j < tasks[i].burstTime; j++)
            printf("| P%d ", tasks[i].pid);
        totalTime = tasks[i].endTime + CONTEXT_SWITCH_TIME;
        contextSwitchCount++;
    }
    printf("|\n");

    printResult(totalTime);
}

void RR()
{
    double totalTime = 0;
    int completedTasks = 0;
    contextSwitchCount++;

    printf("Gantt Chart:\n");
    while (completedTasks < taskCount)
    {
        for (int i = 0; i < taskCount; i++)
        {
            if (tasks[i].arrivalTime <= totalTime && tasks[i].remainingTime > 0)
            {
                // Record the start time and response time for the first execution
                if (tasks[i].remainingTime == tasks[i].burstTime)
                {
                    tasks[i].startTime = totalTime;
                    tasks[i].responseTime = tasks[i].startTime - tasks[i].arrivalTime;
                }

                // Execute task for each millisecond within time quantum or remaining time
                int execTime = (tasks[i].remainingTime < timeQuantum) ? tasks[i].remainingTime : timeQuantum;
                for (int j = 0; j < execTime; j++)
                {
                    printf("| P%d ", tasks[i].pid);
                    tasks[i].remainingTime--;
                    totalTime++;

                    // Check if the task completes
                    if (tasks[i].remainingTime == 0)
                    {
                        tasks[i].endTime = totalTime;
                        tasks[i].turnaroundTime = tasks[i].endTime - tasks[i].arrivalTime;
                        tasks[i].waitingTime = tasks[i].turnaroundTime - tasks[i].burstTime;
                        completedTasks++;
                        break;
                    }
                }

                // Insert context switch time after each time quantum, unless task is completed
                if (tasks[i].remainingTime > 0)
                {
                    totalTime += CONTEXT_SWITCH_TIME;
                    contextSwitchCount++;
                }
            }
        }
    }
    printf("|\n");

    printResult(totalTime);
}

void SJF()
{
    double totalTime = 0;
    int completedTasks = 0;
    contextSwitchCount++;

    printf("Gantt Chart:\n");

    while (completedTasks < taskCount)
    {
        // Find the task with the shortest burst time that has arrived
        int shortestTask = -1;
        for (int i = 0; i < taskCount; i++)
        {
            if (!tasks[i].isCompleted && tasks[i].arrivalTime <= totalTime)
            {
                if (shortestTask == -1 || tasks[i].burstTime < tasks[shortestTask].burstTime)
                {
                    shortestTask = i;
                }
            }
        }

        // If a task is found, process it
        if (shortestTask != -1)
        {
            // Record start time and calculate response time for the first execution
            tasks[shortestTask].startTime = totalTime;
            tasks[shortestTask].responseTime = tasks[shortestTask].startTime - tasks[shortestTask].arrivalTime;

            // Execute task one millisecond at a time for the Gantt chart
            for (int j = 0; j < tasks[shortestTask].burstTime; j++)
            {
                printf("| P%d ", tasks[shortestTask].pid);
                totalTime++;
            }

            // Calculate turnaround and waiting times upon completion
            tasks[shortestTask].endTime = totalTime;
            tasks[shortestTask].turnaroundTime = tasks[shortestTask].endTime - tasks[shortestTask].arrivalTime;
            tasks[shortestTask].waitingTime = tasks[shortestTask].turnaroundTime - tasks[shortestTask].burstTime;

            tasks[shortestTask].isCompleted = 1;
            completedTasks++;

            // Add context switch time if there are still tasks remaining
            if (completedTasks < taskCount)
            {
                totalTime += CONTEXT_SWITCH_TIME;
                contextSwitchCount++;
            }
        }
    }
    printf("|\n");

    printResult(totalTime);
}

void SRTF()
{
    double totalTime = 0;
    int completedTasks = 0;
    contextSwitchCount++;

    printf("Gantt Chart:\n");

    while (completedTasks < taskCount)
    {
        // Find the task with the shortest remaining time that has arrived
        int shortestTask = -1;
        for (int i = 0; i < taskCount; i++)
        {
            if (!tasks[i].isCompleted && tasks[i].arrivalTime <= totalTime)
            {
                if (shortestTask == -1 || tasks[i].remainingTime < tasks[shortestTask].remainingTime)
                    shortestTask = i;
            }
        }

        // If a task is found, process it for one millisecond
        if (shortestTask != -1)
        {
            // Record start and response time for the first execution
            if (tasks[shortestTask].remainingTime == tasks[shortestTask].burstTime)
            {
                tasks[shortestTask].startTime = totalTime;
                tasks[shortestTask].responseTime = tasks[shortestTask].startTime - tasks[shortestTask].arrivalTime;
            }

            // Execute task for one millisecond in Gantt chart
            printf("| P%d ", tasks[shortestTask].pid);
            tasks[shortestTask].remainingTime--;
            totalTime++;

            // If task completed
            if (tasks[shortestTask].remainingTime == 0)
            {
                tasks[shortestTask].endTime = totalTime;
                tasks[shortestTask].turnaroundTime = tasks[shortestTask].endTime - tasks[shortestTask].arrivalTime;
                tasks[shortestTask].waitingTime = tasks[shortestTask].turnaroundTime - tasks[shortestTask].burstTime;
                tasks[shortestTask].isCompleted = 1;
                completedTasks++;
            }

            // Insert context switch if there’s another task with shorter remaining time
            int nextShortestTask = -1;
            for (int i = 0; i < taskCount; i++)
            {
                if (!tasks[i].isCompleted && tasks[i].arrivalTime <= totalTime)
                {
                    if (nextShortestTask == -1 || tasks[i].remainingTime < tasks[nextShortestTask].remainingTime)
                        nextShortestTask = i;
                }
            }
            if (nextShortestTask != shortestTask && nextShortestTask != -1)
            {
                totalTime += CONTEXT_SWITCH_TIME;
                contextSwitchCount++;
            }
        }
        else
        {
            // If no task is ready, increment time
            totalTime++;
        }
    }
    printf("|\n");

    printResult(totalTime);
}
