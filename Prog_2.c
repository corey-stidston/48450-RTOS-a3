/*
 * Author: Corey Stidston
 * Compilation method: For compiling this source code, you won't need any flags.
 * e.g:     gcc Prog_2.c
 * When executing Prog_2, provide the input file name and the output file name
 * i.e:     ./a.out Topic2_Prog_2.txt output.txt
 * The program will use Topic2_Prog_2.txt as the input file and output.txt as the output file
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define MAX_NUM_PROCESSES 20

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

void readFromFile(char * filename); // Reads From The Input File
void extractData(unsigned int linePos, int * processId, char * found); // Extracts Data From The File
bool detectCpuDeadlock(int * processSequence, int * deadlockedProcesses, int * numFinishedProcesses); // Detects CPU Deadlock
void writeToFile(int numFinishedProcesses, int * processSequence, int * deadlockedProcesses, char * filename); // Writes To File
void writeToFileHelper(char * buffer, FILE * writeFile); // Helper Method For Writing To File (includes error handling)
void printSequence(FILE * writeFile, int * sequence, int upperBound); // Writes Sequence Of Processes To File

/*
 *  int signum - Is The Signal Id
 */
void signalHandler(int signum)
{
    if (signum == SIGUSR1)
    {
        printf("Writing to output file has finished!\n");
    }
}

int main(int argc, char*argv[])
{
    if(argc != 3)
    {
        printf("Please supply two arguments, an input filename e.g. 'input.txt' and an output filename e.g. 'output.txt'.\n");
        exit(0);
    }
    
    int processSequence[MAX_NUM_PROCESSES];
    int deadlockedProcesses[MAX_NUM_PROCESSES];
    int numFinishedProcesses;
    
    signal(SIGUSR1, signalHandler); // Assign Handler Method to SIGUSR1
    
    readFromFile(argv[1]); // Read From File
    detectCpuDeadlock(processSequence, deadlockedProcesses, &numFinishedProcesses); // Detect CPU Deadlock
    writeToFile(numFinishedProcesses, processSequence, deadlockedProcesses, argv[2]); // Write Data To File
    
    return(0);
}

/*
 *  char * filename - pointer to a filename
 */
void readFromFile(char * filename)
{
    char lineFromFile[BUFFER_SIZE];
    
    FILE * readFile = fopen(filename, "r");
    if(!readFile)
    {
        perror("Invalid File."); exit(0);
    }
    
    const char delimiters[] = " \t";
    char * truncatedLine;
    char * lineSegment;
    int lineNo = 1;
    
    while(fgets(lineFromFile, BUFFER_SIZE, readFile) != NULL)
    {
        if(lineNo > 2) // Ignore Column Names
        {
            truncatedLine = strdup(lineFromFile); // Return Null Terminated Byte String
            
            int linePos = 0;
            int processId;
            while((lineSegment = strsep(&truncatedLine, delimiters)) != NULL ) // Slice String On Spaces and Tabs
            {
                if(strlen(lineSegment) >= 1) // Filter Out Unuseful Information
                {
                    extractData(linePos, &processId, lineSegment); // Extract and Store Data
                    ++linePos;
                }
            }
            ++numProcesses;
        }
        ++lineNo;
    }
    
    free(truncatedLine); // Free Dynamically Allocated Memory
}

/*
 * unsigned int linePos - integer representing the line position in the input file
 * int * processId - pointer to an integer representing the current process id
 * char * lineSegment - pointer to a char array representing a segment of the line (critical value)
 */
void extractData(unsigned int linePos, int * processId, char * lineSegment)
{
    /* Store Data Based On Line Position */
    switch(linePos)
    {
        case 0: {
                *processId = atoi(&lineSegment[1]);
                if(*processId > MAX_NUM_PROCESSES)
                {
                    printf("Error: this program does not accept more than %i number of processes.\n", MAX_NUM_PROCESSES); exit(1);
                }
            }
            break;
        case 1: allocationMatrix[*processId][RESOURCE_A] = atoi(lineSegment);
            break;
        case 2: allocationMatrix[*processId][RESOURCE_B] = atoi(lineSegment);
            break;
        case 3: allocationMatrix[*processId][RESOURCE_C] = atoi(lineSegment);
            break;
        case 4: requestMatrix[*processId][RESOURCE_A] = atoi(lineSegment);
            break;
        case 5: requestMatrix[*processId][RESOURCE_B] = atoi(lineSegment);
            break;
        case 6: requestMatrix[*processId][RESOURCE_C] = atoi(lineSegment);
            break;
        case 7: availabilityVector[RESOURCE_A] = atoi(lineSegment);
            break;
        case 8: availabilityVector[RESOURCE_B] = atoi(lineSegment);
            break;
        case 9: availabilityVector[RESOURCE_C] = atoi(lineSegment);
            break;
    }
}

/*
 * int * processSequence - pointer to an array of integers representing the process sequence
 * int * deadlockedProcesses - pointer to an array of integers representing the process sequence
 * int * numFinishedProcesses - pointer to a integer representing the number of finished processes
 */
bool detectCpuDeadlock(int * processSequence, int * deadlockedProcesses, int * numFinishedProcesses)
{
    bool finishVector[MAX_NUM_PROCESSES] = {false};
    *numFinishedProcesses = 0;
    int i, j;
    
    for(i = 0; i < numProcesses; i++)
    {
        for(j = 0; j < numProcesses; ++j)
        {
            // 1. Determine Whether Particular Process Has Already Finished.
            if(!finishVector[j])
            {
                // 2. Check Whether Request <= Available
                if(requestMatrix[j][RESOURCE_A] <= availabilityVector[RESOURCE_A] && requestMatrix[j][RESOURCE_B] <= availabilityVector[RESOURCE_B] && requestMatrix[j][RESOURCE_C] <= availabilityVector[RESOURCE_C])
                {
                    // 3. Availability = Availability + Allocation
                    availabilityVector[RESOURCE_A] += allocationMatrix[j][RESOURCE_A];
                    availabilityVector[RESOURCE_B] += allocationMatrix[j][RESOURCE_B];
                    availabilityVector[RESOURCE_C] += allocationMatrix[j][RESOURCE_C];
                    
                    finishVector[j] = true; // 4. Mark As Finished
                    
                    processSequence[(*numFinishedProcesses)++] = j;
                    if(*numFinishedProcesses == numProcesses)
                    {
                        return false;;
                    }
                }
            }
        }
    }
    
    int numDeadlockedProcesses = 0;
    for(i = 0; i < numProcesses; i++)
    {
        if(!finishVector[i])
        {
            deadlockedProcesses[numDeadlockedProcesses++] = i;
        }
    }
    
    return true;
}

/*
 * int numFinishedProcesses - integer representing the number of finished processes
 * int * processSequence - pointer to an array of integers representing the process sequence
 * int * deadlockedProcesses - pointer to an array of integers representing the process sequence
 * char * filename - pointer to an array of chars representing the filename
 */
void writeToFile(int numFinishedProcesses, int * processSequence, int * deadlockedProcesses, char * filename)
{
    char buffer[BUFFER_SIZE];
    FILE * writeFile = fopen(filename, "w");
    if(!writeFile)
    {
        perror("Invalid File."); exit(1);
        exit(1);
    }
    
    if(numFinishedProcesses != numProcesses)
    {
        writeToFileHelper("Deadlock Detected. Resource of the following processes were released:\n<", writeFile);
        printSequence(writeFile, processSequence, numFinishedProcesses);
        writeToFileHelper("A deadlock exists with the following processes:\n<", writeFile);
        printSequence(writeFile, deadlockedProcesses, numProcesses - numFinishedProcesses);
    }
    else
    {
        writeToFileHelper("No Deadlock Detected. Processes were completed in the following sequence:\n<", writeFile);
        printSequence(writeFile, processSequence, numProcesses);
    }

    if(ferror(writeFile))
    {
        perror("Error Writing to File.\n"); exit(1);
    }
    
    if(fclose(writeFile) == EOF)
    {
        perror("Failed To Close File."); exit(1);
    }
    
    raise(SIGUSR1); // Raise User Defined Signal 1
}

/*
 * char * buffer - pointer to an array of characters representing a char buffer
 * FILE * writeFile - file pointer
 */
void writeToFileHelper(char * buffer, FILE * writeFile)
{
    if(fputs(buffer, writeFile) == EOF)
    {
        perror("Error Writing to File.\n"); exit(1);
    }
}

/*
 * FILE * writeFile - file pointer
 * int * sequence - pointer to an array of integers representing a sequence of processes
 * int upperBound - integer representing the upper bound of a for loop
 */
void printSequence(FILE * writeFile, int * sequence, int upperBound)
{
    char buffer[BUFFER_SIZE];
    
    int i;
    for(i = 0; i < upperBound; ++i)
    {
        if(i != upperBound-1)
        {
            if(snprintf(buffer, sizeof(buffer), "%i, ", sequence[i]) < 0)
            {
                perror("Error Writing to Buffer.\n"); exit(1);
            }
        }
        else
        {
            if(snprintf(buffer, sizeof(buffer), "%i>\n", sequence[i]) < 0)
            {
                perror("Error Writing to Buffer."); exit(1);
            }
        }
        writeToFileHelper(buffer, writeFile);
    }
}
