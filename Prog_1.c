/*
 * Author: Corey Stidston
 * Compilation method: For compiling this source code, you should use two flags, -pthread and -lrt
 * e.g:     gcc Prog_1.c -pthread -lrt
 * When executing Prog_1, provide the output txt file name
 * i.e:     ./a.out output.txt
 * The program will use output.txt as the output
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
    int * fd;
    int * numElements;
    int elementSizeInBytes;
} fifo_t;

typedef struct {
    char const * const filename;
    fifo_t * fifo;
} writing_args_t;

typedef struct {
    process_data_t * processData;
    fifo_t * fifo;
} cpu_scheduler_args_t;

sem_t writeToFileSem; // Semaphores
pthread_t tidA, tidB; // Thread IDs
pthread_attr_t attr; // Thread attributes

void initializeData(process_data_t *processData);
void initializeSemaphore();
void *simulateCpuScheduler(void *param);
void *writeToFile(void *param);
void writeToFIFO(fifo_t * fifo, char * description, float value);

int main(int argc, char*argv[])
{
    if(argc != 2)
    {
        printf("Please supply 1 argument: the output file name.\n"); exit(1);
    }
    
    initializeSemaphore();
    
    if(mkfifo(FIFO_PATH, 0666) == -1)
    {
        printf("Issue creating mkfifo.\n"); exit(1);
    }
    
    int fd = open(FIFO_PATH, O_RDWR);
    if(fd == -1)
    {
        perror("Error opening FIFO\n"); exit(1);
    }
    
    int numElements = 0;
    
    fifo_t fifo = {&fd, &numElements, 100};
    
    writing_args_t writingArgs = {argv[1], &fifo};
    
    process_data_t *processData;
    processData = malloc(sizeof(process_data_t)*NUM_PROCESSES);// Allocate Memory For Process Data
    initializeData(processData);
    
    cpu_scheduler_args_t cpuArgs = {processData, &fifo};
    
    pthread_attr_init(&attr); // Get the default attributes

    pthread_create(&tidA, &attr, simulateCpuScheduler, &cpuArgs); // Creates Thread A
    pthread_create(&tidB, &attr, writeToFile, &writingArgs); // Creates Thread B

    // Wait For Thread B To Finish
    if(pthread_join(tidB, NULL) != 0)
    {
        perror("Issue joining Thread B\n"); exit(1);
    }
    
    // Close FIFO
    if(close(fd) == -1)
    {
        perror("Issue closing FIFO.\n"); exit(1);
    }
    
    // Delete FIFO name and file from filesystem
    if(unlink(FIFO_PATH) == -1)
    {
        perror("Issue deleting FIFO name and file.\n"); exit(1);
    }
    
    // Free Allocated Memory
    free(processData);

    return(0);
}

void *simulateCpuScheduler(void *param)
{
    cpu_scheduler_args_t *parameters = param;
    process_data_t *processData = parameters->processData;
    
    unsigned int time = 0; // Represents CPU ticks
    
    // CPU Calculation Descriptors
    char averageWaitingTimeDescription[] = "Average Waiting Time";
    char averageTurnaroundTimeDescription[] = "Average Turn-around Time";
    
    // Initialize Variables To Calculate Average Waiting Time and Average Turn-around Time 
    float waitingTime = 0;
    float turnaroundTime = 0;
    float averageWaitingTime = 0;
    float averageTurnaroundTime = 0;
    
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
    
    // Push Average Waiting Time To FIFO
    writeToFIFO(parameters->fifo, averageWaitingTimeDescription, averageWaitingTime);
    writeToFIFO(parameters->fifo, averageTurnaroundTimeDescription, averageTurnaroundTime);
    
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
    
    sem_wait(&writeToFileSem);
    
    // Read From FIFO
    char buffer[parameters->fifo->elementSizeInBytes];
    int i;
    for(i = 0; i < *parameters->fifo->numElements; ++i)
    {
        if(read(*parameters->fifo->fd, buffer, sizeof(buffer)) == -1) // Read Into Buffer
        {
            perror("Failed To Read From FIFO."); exit(1);
        }
        fputs(buffer, writeFile); // Write To Output File
        if(ferror(writeFile))
        {
            printf("Error Writing to File.\n");
        }
    }

    if(fclose(writeFile) == EOF) // Close The File
    {
        perror("Failed To Close File."); exit(1);
    }
    
    // Cancel Threads On Completion
    pthread_cancel(tidA);
    pthread_cancel(tidB);

    return 0;
}

void writeToFIFO(fifo_t * fifo, char * description, float value)
{
    char buffer[fifo->elementSizeInBytes];
    if(snprintf(buffer, sizeof(buffer), "%s = %f.\n", description, value) < 0)
    {
        perror("Error writing to buffer."); exit(1);
    }
    if(write(*fifo->fd, buffer, sizeof(buffer)) == -1)
    {
        perror("Failed to write to FIFO."); exit(1);
    }
    *fifo->numElements += 1;
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
