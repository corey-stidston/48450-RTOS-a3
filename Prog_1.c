/*
 * Author: Corey Stidston
 * Compilation method:
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define NUM_PROCESSES 7
#define FIFO_PATH "/tmp/prog1fifo"

typedef struct {
    int pId;
    int arriveTime;
    int burstTime;
    int remainingBurstTime;
} process_data_t;

typedef struct {
    char const * const filename;
} writing_args_t;

typedef struct {
    char key[3];
    float value;
} key_value_pair_t;

sem_t writeToFileSem; // Semaphores
pthread_t tidA, tidB; // Thread IDs
pthread_attr_t attr; // Thread attributes

void initializeData(process_data_t *processData);
void initializeSemaphore();
void *simulateCpuScheduler(void *param);
void *writeToFile(void *param);

int main(int argc, char*argv[])
{
    if(argc != 2)
    {
        printf("Please supply 1 argument: the output file name.\n"); exit(1);
    }
    
    writing_args_t writingArgs = {argv[1]};
    
    process_data_t *processData;
    processData = malloc(sizeof(process_data_t)*NUM_PROCESSES);// Allocate Memory For Process Data
    
    initializeData(processData);
    initializeSemaphore();
    
    //if(mkfifo(FIFO_PATH, 0666) == -1)
    //{
    //    printf("Issue creating mkfifo.\n"); exit(1);
    //}
    
    pthread_attr_init(&attr); // Get the default attributes

    pthread_create(&tidA, &attr, simulateCpuScheduler, processData); // Creates Thread A
    pthread_create(&tidB, &attr, writeToFile, &writingArgs); // Creates Thread B

    // Wait For Thread B To Finish
    if(pthread_join(tidB, NULL) != 0)
    {
        printf("Issue joining Thread B\n"); exit(1);
    }

    // Free Allocated Memory
    free(processData);

    return(0);
}

void *simulateCpuScheduler(void *param)
{
    process_data_t *processData = param;
    
    unsigned int time = 0; // Represents CPU ticks
    
    //int fd = open(FIFO_PATH, O_WRONLY); // Open mkfifo Pipe With Write Only Permission
    
    // Initialize Variables To Calculate Average Waiting Time and Average Turn-around Time 
    double waitingTime = 0;
    double turnaroundTime = 0;
    double averageWaitingTime = 0;
    double averageTurnaroundTime = 0;
    
    int indexOfSmallestCpuBurstTime;
    int numProcessesComplete = 0;
    
    while(numProcessesComplete != NUM_PROCESSES)
    {
        // Find The Process with the Smallest Burst Time
        indexOfSmallestCpuBurstTime = -1;
        bool first = true;
        int i;
        for (i = 0; i < NUM_PROCESSES; i++)
        {
            if(time >= processData[i].arriveTime && processData[i].remainingBurstTime > 0)
            {
                if(first || processData[i].remainingBurstTime < processData[indexOfSmallestCpuBurstTime].remainingBurstTime)
                {
                    indexOfSmallestCpuBurstTime = i;
                    first = false;
                }
            }
        }
        
        // Simulate CPU Clock
        ++time;
        
        if(indexOfSmallestCpuBurstTime != -1) // Check For A Valid Index
        {
            processData[indexOfSmallestCpuBurstTime].remainingBurstTime -= 1; // Simulate CPU Burst
            
            if(processData[indexOfSmallestCpuBurstTime].remainingBurstTime == 0) // Check For Completed Process
            {
                // Summate Wait Time (wait time = end time - arrival time - burst time)
                waitingTime += time - processData[indexOfSmallestCpuBurstTime].arriveTime - processData[indexOfSmallestCpuBurstTime].burstTime;
                
                // Summate Turn-around Time (turn-around time = end time - arrive time)
                turnaroundTime += time - processData[indexOfSmallestCpuBurstTime].arriveTime;
                
                // Increment The Number of Processes CompletesimulateCpuSchedulerSem
                ++numProcessesComplete;
            }
        }
    }
    
    averageWaitingTime = waitingTime / NUM_PROCESSES; // Calculate Average Waiting Time
    averageTurnaroundTime = turnaroundTime / NUM_PROCESSES; // Calculate Average Turn-around Time
    
    printf("Average waiting time = %f \n", averageWaitingTime);
    printf("Average turn-around time = %f \n", averageTurnaroundTime);
    
    // Push to FIFO
    //key_value_pair_t averageWaitingTime_t = {"AWT", averageWaitingTime};
    //key_value_pair_t *test;

    //write(fd, &averageWaitingTime, sizeof(double));
    //close(fd);
    
    sem_post(&writeToFileSem);
    
    return 0;
}

void *writeToFile(void *param)
{
    writing_args_t *parameters = param;
    FILE* writeFile = fopen(parameters->filename, "w");
    if(!writeFile)
    {
       perror("Invalid File");
       exit(1);
    }
    
    //int fd = open(FIFO_PATH, O_RDONLY); // Open mkfifo Pipe With Read Only Permission

    sem_wait(&writeToFileSem);
    
    //double *averageWaitingTime;
    //read(fd, averageWaitingTime, sizeof(double));
    //printf("hi -> %f <-\n", *averageWaitingTime);

    fputs("hello12345", writeFile);

    fclose(writeFile); // Close FILE*
    //close(fd);
    
    pthread_cancel(tidA);
    pthread_cancel(tidB);

    return 0;
}

void initializeData(process_data_t *processData)
{
    processData[0].pId = 1; processData[0].arriveTime = 8; processData[0].burstTime = 10; processData[0].remainingBurstTime = 10;
    processData[1].pId = 2; processData[1].arriveTime = 10; processData[1].burstTime = 3; processData[1].remainingBurstTime = 3;
    processData[2].pId = 3; processData[2].arriveTime = 14; processData[2].burstTime = 7; processData[2].remainingBurstTime = 7;
    processData[3].pId = 4; processData[3].arriveTime = 9; processData[3].burstTime = 5; processData[3].remainingBurstTime = 5;
    processData[4].pId = 5; processData[4].arriveTime = 16; processData[4].burstTime = 4; processData[4].remainingBurstTime = 4;
    processData[5].pId = 6; processData[5].arriveTime = 21; processData[5].burstTime = 6; processData[5].remainingBurstTime = 6;
    processData[6].pId = 7; processData[6].arriveTime = 26; processData[6].burstTime = 2; processData[6].remainingBurstTime = 2;
}

void initializeSemaphore()
{   
    if(sem_init(&writeToFileSem, 0, 0))
    {
        perror("Error Initializing Semaphore.\n");
        exit(1);
    }
    
    pthread_attr_init(&attr);
}
