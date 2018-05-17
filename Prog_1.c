/*
 * Author: Corey Stidston
 * Compilation method:
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define NUM_PROCESSES 7
#define FIFO_SIZE 2

typedef struct {
    int pId;
    int arriveTime;
    int burstTime;
    int remainingBurstTime;
} process_data_t;

typedef struct {
    char const * const filename;
} writing_args_t;

//sem_t simulateCpuSchedulerSem, writeToFileSem; // Semaphores
//pthread_t tidA, tidB; // Thread IDs
//pthread_attr_t attr; // Thread attributes

void initializeData(process_data_t *processData);
void *simulateCpuScheduler(void *param);
void *writeToFile(void *param);

int main(int argc, char*argv[])
{
    unsigned int time = 0;
    
    process_data_t *processData;
    processData = malloc(sizeof(process_data_t)*NUM_PROCESSES);// Allocate memory for processData
    initializeData(processData);

    
    /************START Thread 1: Simulate CPU Scheduling */
    
    // Initialize Average Waiting Time and Average Turn-around time
    double waitingTime = 0;
    double turnaroundTime = 0;
    double averageWaitingTime;
    double averageTurnaroundTime;
    
    // Have some kind of loop here
    int indexOfSmallestCpuBurstTime;
    int numProcessesComplete = 0;
    
    while(numProcessesComplete != NUM_PROCESSES)
    {
        // Find The Process with the Smallest Burst Time
        indexOfSmallestCpuBurstTime = -1;
        bool first = true;
        for (int i = 0; i < NUM_PROCESSES; i++)
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
        
        //Simulate CPU Clock
        ++time;
        
        if(indexOfSmallestCpuBurstTime != -1) // Check For A Valid Index
        {
            //printf("executing process: %i\n", indexOfSmallestCpuBurstTime + 1);
            processData[indexOfSmallestCpuBurstTime].remainingBurstTime -= 1; // Simulate CPU Burst
            
            if(processData[indexOfSmallestCpuBurstTime].remainingBurstTime == 0) // Check For Completed Process
            {
                //printf("finished process: %i at time: %i\n", indexOfSmallestCpuBurstTime+1, time);
                
                // Summate Wait Time (wait time = end time - arrival time - burst time)
                waitingTime += time - processData[indexOfSmallestCpuBurstTime].arriveTime - processData[indexOfSmallestCpuBurstTime].burstTime;
                
                // Summate Turn-around Time (turn-around time = end time - arrive time)
                turnaroundTime += time - processData[indexOfSmallestCpuBurstTime].arriveTime;
                
                // Increment The Number of Processes Complete
                ++numProcessesComplete;
            }
        }
    }
    
    averageWaitingTime = waitingTime / NUM_PROCESSES; // Calculate Average Waiting Time
    averageTurnaroundTime = turnaroundTime / NUM_PROCESSES; // Calculate Average Turn-around Time
    
    printf("Average waiting time = %f \n", averageWaitingTime);
    printf("Average turn-around time = %f \n", averageTurnaroundTime);
    
    
    // Calculate average waiting time and average turn-around time
    
    // Push to FIFO
    
    // Signal Thread 2
    
    /************ END Thread 1 */
    
    //writing_args_t writingArgs = {argv[1]};
    
    
//    pthread_attr_init(&attr); // Get the default attributes
//
//    pthread_create(&tidA, &attr, simulateCPUScheduler, processData); // Creates thread A
//    pthread_create(&tidB, &attr, writeToFile, &writingArgs); // Creates thread B
//
//    if(pthread_join(tidA, NULL) != 0)
//        printf("Issue joining Thread A\n"); exit(1);
//    if(pthread_join(tidB, NULL) != 0)
//        printf("Issue joining Thread B\n"); exit(1);
    
    
    // Free Allocated Memory
    free(processData);

    return(0);
}

void *simulateCpuScheduler(void *param)
{
    return 0;
}

void *writeToFile(void *param)
{
//    writing_args_t *parameters = param;
//    FILE* writeFile = fopen(parameters->filename, "w");
//    if(!writeFile)
//    {
//        perror("Invalid File");
//        exit(1);
//    }
//
//    char line[BUFFER_SIZE];
//    int eoh_flag = 0; // End of header flag
//
//    while(!sem_wait(&writeToFileSem))
//    {
//        if(eoh_flag)
//        {
//            /* Place the contents of the shared buffer into file */
//            fputs(parameters->shared_buffer->buffer, writeFile);
//        }
//        else if(strstr(parameters->shared_buffer->buffer, END_OF_HEADER)) // Check for the end of header
//        {
//            eoh_flag = 1;
//        }
//
//        sem_post(&readSem);
//    }
//
//    fclose(writeFile); // Close FILE*
    
//    pthread_cancel(tidA);
//    pthread_cancel(tidB);

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
