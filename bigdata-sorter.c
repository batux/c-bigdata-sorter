/*
    Batuhan Düzgün - 03.01.2021
    Question-3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

const int RANDOM_NUMBER_SIZE = 1000000; //1000000; //100;
const int CHUNK_SIZE = 100000; //100000; //10;
const int DEBUG_FLAG = 0;

#define DATA_SOURCE_FILE                            "random.dat"
#define RESULT_FILE                                 "result.dat"
#define RANDOM_NUMBER_WRITE_OPERATION_OK            0;
#define RANDOM_NUMBER_WRITE_OPERATION_FAIL          100;
#define RANDOM_NUMBER_WRITE_OPERATION_LIMIT_ZERO    101;


FILE *createDataFileIfNotExist(const char *filePath, int appendMode)
{
    FILE *fBinaryData;

    if( appendMode == 1 && (fBinaryData = fopen(filePath, "r")) != NULL )
    {
        fBinaryData = fopen(filePath, "ab+");
        return fBinaryData;
    }

    if( (fBinaryData = fopen(filePath, "wb+")) == NULL )
    {
        fprintf(stderr, "cannot open file!...\n");
        //exit(EXIT_FAILURE);
        return NULL;
    }

    return fBinaryData;
}

void closeFile(FILE *fBinaryData)
{
    fclose(fBinaryData);
}

int writeRandomNumbersToFile(FILE *fBinaryData, int limit)
{
    srand(time(0));

    if(limit == 0)
    {
        return RANDOM_NUMBER_WRITE_OPERATION_LIMIT_ZERO;
    }

    for(int i = 0; i < limit; i++)
    {
        int randomNumber = rand() / 100000;
        if (fwrite(&randomNumber, sizeof(int), 1, fBinaryData) != 1) 
        {
            if (ferror(fBinaryData)) 
            {
                fprintf(stderr, "cannot write file!..\n");
			    return RANDOM_NUMBER_WRITE_OPERATION_FAIL;
            }
		}
    }

    return RANDOM_NUMBER_WRITE_OPERATION_OK;
}

FILE *createTmpFile()
{
    FILE *tmpFile = NULL;

	if ((tmpFile = tmpfile()) == NULL) 
    {
		fprintf(stderr, "cannot create temporary file!..\n");
		exit(EXIT_FAILURE);
	}

    return tmpFile;
}

int calculateChunkSize()
{
    int chunkPartCount = RANDOM_NUMBER_SIZE / CHUNK_SIZE;
    if( (RANDOM_NUMBER_SIZE % CHUNK_SIZE) > 0)
    {
        chunkPartCount++;
    }
    return chunkPartCount;
}

int *createChunkLimitArray(int limit)
{
    int *chunkLimitParts = (int*) malloc(limit * sizeof(int));
    int chunkPartCount = RANDOM_NUMBER_SIZE / CHUNK_SIZE;

    for(int i = 0; i < chunkPartCount; i++)
    {
        chunkLimitParts[i] = CHUNK_SIZE;
    }

    int hasLeapItem = (limit - chunkPartCount);
    if(hasLeapItem > 0)
    {
        chunkLimitParts[limit - 1] = (RANDOM_NUMBER_SIZE % CHUNK_SIZE);
    }

    return chunkLimitParts;
}

int *readChunkFromDataFile(FILE *fBinaryData, int dataSize, int seekPoint)
{
    int val;
    int chunkItemCounter = 0;
    int *chunkNumberArr = (int*) malloc(dataSize * sizeof(int));

    fseek(fBinaryData, (seekPoint * sizeof(int)), SEEK_SET);
    //printf("FILE CURSOR: %ld\n", ftell(fBinaryData));

    while ( fread(&val, sizeof(int), 1, fBinaryData) > 0 )
    {
        if( !(chunkItemCounter < dataSize) )
        {
            break;
        }
        chunkNumberArr[chunkItemCounter] = val;
        //printf("chunkNumberArr[%d]: %d \n", chunkItemCounter, chunkNumberArr[chunkItemCounter]);
        chunkItemCounter++;
    }

    if (ferror(fBinaryData)) 
    {
		fprintf(stderr, "cannot read file!..\n");
		exit(EXIT_FAILURE);
	}

    return chunkNumberArr;
}

void printNumbers(int *arr, int chunkSize)
{   
    int i = 0;
    while(i < chunkSize)
    {
        printf("%d \n", *(arr + i));
        i++;
    }
    printf("------------------------------------------------------\n");
}

void writeToTmpFile(FILE *tmpF, int *nbrArr, int limit)
{
    for (int i = 0; i < limit; i++)
    {
        int val = nbrArr[i];
        if (fwrite(&val, sizeof(int), 1, tmpF) != 1) 
        {
			fprintf(stderr, "cannot write file!..\n");
			exit(EXIT_FAILURE);
		}
    }
}

int readFromTmpFile(FILE *f, int seekPoint)
{
    fseek(f, (seekPoint * sizeof(int)), SEEK_SET);

    int val;
    if (fread(&val, sizeof(int), 1, f) > 0)
    {
        return val;
    }

    return INT_MAX;
}

int findMinValueIndex(int array[], int limit) 
{
    int minIndex = 0;
    int min = array[minIndex];

    for (int i = 1; i < limit; i++)
    {
        if (array[i] < min)
        {
            minIndex = i;
            min = array[minIndex];
        }
    }

    return minIndex;
}

int isReadCompleted(int array[], int limit)
{
    int flag = 1;
    for (int i = 0; i < limit; i++)
    {
        if(array[i] != INT_MAX)
        {
            flag = -1;
            break;
        }
    }
    return flag;
}

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
}

void bubbleSort(int *array, int limit) 
{ 
   int i, j; 
   for (i = 0; i < limit-1; i++)
   {
       for (j = 0; j < limit-i-1; j++) 
       {
           if (array[j] > array[j+1]) 
           {
               swap(&array[j], &array[j+1]); 
           }
       }
   }
} 

void closeFiles(FILE *tmpFiles[], int limit)
{
    int fileCounter = 0;
    while(fileCounter < limit)
    {
        FILE *tmpF;
        tmpF = tmpFiles[fileCounter];
        closeFile(tmpF);
        fileCounter++;
    }
}

void printResult(FILE *fBinaryResult, int debug)
{
    printf("FULL RESULT\n");
    fseek(fBinaryResult, 0, SEEK_SET);

    int valr;
    int previousVal = 0;
    int validFlag = 1;
    while ( fread(&valr, sizeof(int), 1, fBinaryResult) > 0 )
    {   
        if(debug)
        {
            printf("%d \n", valr);
        }
        
        if(previousVal != 0 && previousVal > valr)
        {
            printf("INVALID STATE => pre: %d, curr: %d \n", previousVal, valr);
            validFlag = 0;
            break;
        }

        previousVal = valr;
    }
    
    if(validFlag == 1)
    {
        printf("ORDER SUCCESS!\n");
    }
}

int *createTmpFileSeekPoints(int limit)
{
    int *seekPointsOfTmpFiles = (int*) malloc( limit * sizeof(int) );
    for(int i = 0; i < limit; i++)
    {
        seekPointsOfTmpFiles[i] = 0;
    }
    return seekPointsOfTmpFiles;
}

int *readNumbersFromTmpFiles(FILE *tmpFiles[], int *seekPointsOfTmpFiles, int limit, int debug)
{
    int *tmpValues = (int*) malloc( limit * sizeof(int) );

    for(int i = 0; i < limit; i++)
    {
        FILE *tmpF;
        tmpF = tmpFiles[i];
        int valueFromTmpFile = readFromTmpFile(tmpF, seekPointsOfTmpFiles[i]);
        tmpValues[i] = valueFromTmpFile;

        if(debug)
        {
            printf("TMP-%d: %d  ", i, tmpValues[i]);
        }
    }
    return tmpValues;
}

void createTmpFiles(FILE *fBinaryData, FILE *tmpFiles[], int *chunkLimitParts, int limit, int debug)
{
    int chunkPartCounter = 0;
    int seekPoint = 0;

    while(chunkPartCounter < limit)
    {
        int chunkSize = chunkLimitParts[chunkPartCounter];

        int *numberArr = readChunkFromDataFile(fBinaryData, chunkSize, seekPoint);
        printf("STEP-%d \n", chunkPartCounter);
        
        if(debug)
        {
            printNumbers(numberArr, chunkSize);
            printf("ORDER-%d \n", chunkPartCounter);
        }
        
        bubbleSort(numberArr, chunkSize);

        if(debug)
        {
            printNumbers(numberArr, chunkSize);
        }

        FILE *tmpFile = createTmpFile();
        writeToTmpFile(tmpFile, numberArr, chunkSize);
        tmpFiles[chunkPartCounter] = tmpFile;

        free(numberArr);

        seekPoint = seekPoint + chunkSize;
        chunkPartCounter++;
    }
}

void printRawData(FILE *fBinaryData, int debug)
{   
    if(debug)
    {
        printf("FULL DATA\n");
        fseek(fBinaryData, 0, SEEK_SET);

        int valu;
        while ( fread(&valu, sizeof(int), 1, fBinaryData) > 0 )
        {
            printf("%d \n", valu);
        }
        printf("------------------------------------------------------\n");
    }
}

int main()
{
    FILE *fBinaryData;
    if( (fBinaryData = createDataFileIfNotExist(DATA_SOURCE_FILE, 0)) == NULL )
    {
        exit(EXIT_FAILURE);
    }

    if( writeRandomNumbersToFile(fBinaryData, RANDOM_NUMBER_SIZE) > 0 )
    {
        exit(EXIT_FAILURE);
    }

    printRawData(fBinaryData, DEBUG_FLAG);

    int chunkPartLimit = calculateChunkSize();
    int *chunkLimitParts = createChunkLimitArray(chunkPartLimit);

    FILE *tmpFiles[chunkPartLimit];
    createTmpFiles(fBinaryData, tmpFiles, chunkLimitParts, chunkPartLimit, DEBUG_FLAG);
    
    int *seekPointsOfTmpFiles = createTmpFileSeekPoints(chunkPartLimit);

    FILE *fBinaryResult;
    if( (fBinaryResult = createDataFileIfNotExist(RESULT_FILE, 0)) == NULL )
    {
        exit(EXIT_FAILURE);
    }

    int step = 0;
    while(1)
    {   
        printf("ITERATION-%d\n", step);
        int *tmpValues = readNumbersFromTmpFiles(tmpFiles, seekPointsOfTmpFiles, chunkPartLimit, DEBUG_FLAG);

        if(isReadCompleted(tmpValues, chunkPartLimit) == 1)
        {
            if(DEBUG_FLAG)
            {
                printf("BREAK OUT!!\n");
            }
            break;
        }

        int minValIndex = findMinValueIndex(tmpValues, chunkPartLimit);
        int minVal = tmpValues[minValIndex];

        if(DEBUG_FLAG)
        {
            printf("MIN TMP INDEX: %d\n", minValIndex);
            printf("MIN VALUE: %d\n", minVal);
        }

        if (fwrite(&minVal, sizeof(int), 1, fBinaryResult) != 1) 
        {
            fprintf(stderr, "cannot write file!..\n");
            exit(EXIT_FAILURE);
        }

        seekPointsOfTmpFiles[minValIndex] = seekPointsOfTmpFiles[minValIndex] + 1;
        step++;

        if(DEBUG_FLAG)
        {
            printf("----------------------------------------------------\n");
        }
    }
    
    printResult(fBinaryResult, DEBUG_FLAG);
    
    free(chunkLimitParts);
    closeFile(fBinaryData);
    closeFile(fBinaryResult);
    closeFiles(tmpFiles, chunkPartLimit);

    return 0;
}