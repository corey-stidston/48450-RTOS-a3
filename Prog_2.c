/*
 * Author: Corey Stidston
 * Compilation method:
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define MAX_NUM_PROCESSES 9

typedef enum
{
    RESOURCE_A, // 0
    RESOURCE_B, // 1
    RESOURCE_C, // 2
    NUM_RESOURCES // 3
} RESOURCE;

unsigned int availabilityVector[NUM_RESOURCES];
unsigned int allocationMatrix[MAX_NUM_PROCESSES][NUM_RESOURCES];
unsigned int requestMatrix[MAX_NUM_PROCESSES][NUM_RESOURCES];
unsigned int numProcesses;

void readFromFile(char * arg);
bool detectDeadlock(unsigned int * processSequence);
void extractData(unsigned int linePos, int * processId, char * found);
void printExtractedData();

int main(int argc, char*argv[])
{
    if(argc != 2)
    {
        printf("Please supply one argument, an input file name e.g. 'input.txt'.\n");
        exit(0);
    }
    
    readFromFile(argv[1]);
    printExtractedData();
    
    unsigned int * processSequence;
    if(detectDeadlock(processSequence))
    {
        // Deadlock Detected
    }
    else
    {
        // No Deadlock Detected
    }
    
    return(0);
}

bool detectDeadlock(unsigned int * processSequence)
{
    return false;
}

void readFromFile(char * argv)
{
    char lineFromFile[BUFFER_SIZE];
    
    FILE * readFile = fopen(argv, "r");
    if(!readFile)
    {
        perror("Invalid File"); exit(0);
    }
    
    const char delimiters[] = " \t";
    char * truncatedLine;
    char * lineSegment;
    int lineNo = 1;
    
    while(fgets(lineFromFile, BUFFER_SIZE, readFile) != NULL)
    {
        if(lineNo > 2) // Ignore Column Names
        {
            truncatedLine = strdup(lineFromFile);
            
            int linePos = 0;
            int processId;
            while((lineSegment = strsep(&truncatedLine, delimiters)) != NULL )
            {
                if(strlen(lineSegment) >= 1) // Filter Out Unuseful Information
                {
                    extractData(linePos, &processId, lineSegment);
                    ++linePos;
                }
            }
            ++numProcesses;
        }
        ++lineNo;
    }
    
    free(truncatedLine);
}

void extractData(unsigned int linePos, int * processId, char * found)
{
    switch(linePos)
    {
        case 0: {
            *processId = atoi(&found[1]);
            if(*processId > MAX_NUM_PROCESSES)
            {
                printf("Error, program does not accept more than %i number of processes.\n", MAX_NUM_PROCESSES); exit(1);
            }
        }
            break;
        case 1: allocationMatrix[*processId][RESOURCE_A] = atoi(found);
            break;
        case 2: allocationMatrix[*processId][RESOURCE_B] = atoi(found);
            break;
        case 3: allocationMatrix[*processId][RESOURCE_C] = atoi(found);
            break;
        case 4: requestMatrix[*processId][RESOURCE_A] = atoi(found);
            break;
        case 5: requestMatrix[*processId][RESOURCE_B] = atoi(found);
            break;
        case 6: requestMatrix[*processId][RESOURCE_C] = atoi(found);
            break;
        case 7: availabilityVector[RESOURCE_A] = atoi(found);
            break;
        case 8: availabilityVector[RESOURCE_B] = atoi(found);
            break;
        case 9: availabilityVector[RESOURCE_C] = atoi(found);
            break;
    }
}

void printExtractedData()
{
    printf("Number of processes %i.\n", numProcesses);
    
    int i;
    for(i = 0; i < numProcesses; i++)
    {
        printf("Process %i:\n", i);
        printf("\tAll Matrix A: %i, B: %i, C: %i.\n", allocationMatrix[i][0],allocationMatrix[i][1], allocationMatrix[i][2]);
        printf("\tReq Matrix A: %i, B: %i, C: %i.\n", requestMatrix[i][0],requestMatrix[i][1], requestMatrix[i][2]);
    }

    printf("Availability Vector: A: %i, B: %i, C: %i.\n", availabilityVector[0], availabilityVector[1], availabilityVector[2]);
}
