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
#define RANDOM_NUMBER_WRITE_OPERATION_OK            0
#define RANDOM_NUMBER_WRITE_OPERATION_FAIL          100
#define RANDOM_NUMBER_WRITE_OPERATION_LIMIT_ZERO    101

// Data file funcs...
FILE *create_data_file_if_notexist(const char *file_path, int append_mode);
int write_random_mumbers_to_file(FILE *f_binary_data, int limit);
int *read_chunk_from_datafile(FILE *f_binary_data, int data_size, int seek_point);
void print_result(FILE *f_binary_result, int debug);
void print_rawdata(FILE *f_binary_data, int debug);
void close_file(FILE *f_binary_data);
void close_files(FILE *tmpfiles[], int limit);

// Temp file funcs...
FILE *create_tmpfile();
void create_tmpfiles(FILE *f_binary_data, FILE *tmpfiles[], int *chunk_limit_parts, int limit, int debug);
void write_to_tmpfile(FILE *tmpf, int *nbr_arr, int limit);
int read_from_tmpfile(FILE *f, int seek_point);
int *read_numbers_from_tmpfiles(FILE *tmpfiles[], int *seekpoints_of_tmpfiles, int limit, int debug);
int *create_tmpfile_seekpoints(int limit);

// Common funcs...
int calculate_chunk_size();
int *create_chunk_limit_array(int limit);
void print_numbers(int *arr, int chunk_size);
int find_minvalue_index(int array[], int limit);
int is_read_op_completed(int array[], int limit);

// Sorting funcs...
void swap(int *val1, int *val2);
void bubblesort(int *array, int limit);

int qsort_compare_func(const void * val1, const void * val2) 
{
   return ( *(int*)val1 - *(int*)val2 );
}

int main()
{
    FILE *f_binarydata;
    if( (f_binarydata = create_data_file_if_notexist(DATA_SOURCE_FILE, 0)) == NULL )
    {
        exit(EXIT_FAILURE);
    }

    if( write_random_mumbers_to_file(f_binarydata, RANDOM_NUMBER_SIZE) > 0 )
    {
        exit(EXIT_FAILURE);
    }

    print_rawdata(f_binarydata, DEBUG_FLAG);

    int chunkpart_limit = calculate_chunk_size();
    int *chunklimit_parts = create_chunk_limit_array(chunkpart_limit);

    FILE *tmpfiles[chunkpart_limit];
    create_tmpfiles(f_binarydata, tmpfiles, chunklimit_parts, chunkpart_limit, DEBUG_FLAG);
    
    int *seekpoints_of_tmpfiles = create_tmpfile_seekpoints(chunkpart_limit);

    FILE *f_binaryresult;
    if( (f_binaryresult = create_data_file_if_notexist(RESULT_FILE, 0)) == NULL )
    {
        exit(EXIT_FAILURE);
    }

    int step = 0;
    while(1)
    {   
        if(DEBUG_FLAG)
        {
            printf("ITERATION-%d\n", step);
        }
        
        int *tmpvalues = read_numbers_from_tmpfiles(tmpfiles, seekpoints_of_tmpfiles, chunkpart_limit, DEBUG_FLAG);

        if(is_read_op_completed(tmpvalues, chunkpart_limit) == 1)
        {
            if(DEBUG_FLAG)
            {
                printf("BREAK OUT!!\n");
            }
            break;
        }

        int minval_index = find_minvalue_index(tmpvalues, chunkpart_limit);
        int minval = tmpvalues[minval_index];

        if(DEBUG_FLAG)
        {
            printf("MIN TMP INDEX: %d\n", minval_index);
            printf("MIN VALUE: %d\n", minval);
        }

        if (fwrite(&minval, sizeof(int), 1, f_binaryresult) != 1) 
        {
            fprintf(stderr, "cannot write file!..\n");
            exit(EXIT_FAILURE);
        }

        seekpoints_of_tmpfiles[minval_index] = seekpoints_of_tmpfiles[minval_index] + 1;
        step++;

        if(DEBUG_FLAG)
        {
            printf("----------------------------------------------------\n");
        }
    }
    
    print_result(f_binaryresult, DEBUG_FLAG);
    
    free(chunklimit_parts);
    close_file(f_binarydata);
    close_file(f_binaryresult);
    close_files(tmpfiles, chunkpart_limit);

    return 0;
}

FILE *create_data_file_if_notexist(const char *file_path, int append_mode)
{
    FILE *f_binary_data;

    if( append_mode == 1 && (f_binary_data = fopen(file_path, "r")) != NULL )
    {
        f_binary_data = fopen(file_path, "ab+");
        return f_binary_data;
    }

    if( (f_binary_data = fopen(file_path, "wb+")) == NULL )
    {
        fprintf(stderr, "cannot open file!...\n");
        return NULL;
    }

    return f_binary_data;
}

void close_file(FILE *f_binary_data)
{
    fclose(f_binary_data);
}

int write_random_mumbers_to_file(FILE *f_binary_data, int limit)
{
    srand(time(0));

    if(limit == 0)
    {
        return RANDOM_NUMBER_WRITE_OPERATION_LIMIT_ZERO;
    }

    for(int i = 0; i < limit; i++)
    {
        int random_number = rand() / 100000;
        if (fwrite(&random_number, sizeof(int), 1, f_binary_data) != 1) 
        {
            if (ferror(f_binary_data)) 
            {
                fprintf(stderr, "cannot write file!..\n");
			    return RANDOM_NUMBER_WRITE_OPERATION_FAIL;
            }
		}
    }

    return RANDOM_NUMBER_WRITE_OPERATION_OK;
}

FILE *create_tmpfile()
{
    FILE *tmpf = NULL;

	if ((tmpf = tmpfile()) == NULL) 
    {
		fprintf(stderr, "cannot create temporary file!..\n");
		exit(EXIT_FAILURE);
	}

    return tmpf;
}

int calculate_chunk_size()
{
    int chunk_part_count = RANDOM_NUMBER_SIZE / CHUNK_SIZE;
    if( (RANDOM_NUMBER_SIZE % CHUNK_SIZE) > 0)
    {
        chunk_part_count++;
    }
    return chunk_part_count;
}

int *create_chunk_limit_array(int limit)
{
    int *chunk_limit_parts = (int*) malloc(limit * sizeof(int));
    int chunk_part_count = RANDOM_NUMBER_SIZE / CHUNK_SIZE;

    for(int i = 0; i < chunk_part_count; i++)
    {
        chunk_limit_parts[i] = CHUNK_SIZE;
    }

    int has_leap_item = (limit - chunk_part_count);
    if(has_leap_item > 0)
    {
        chunk_limit_parts[limit - 1] = (RANDOM_NUMBER_SIZE % CHUNK_SIZE);
    }

    return chunk_limit_parts;
}

int *read_chunk_from_datafile(FILE *f_binary_data, int data_size, int seek_point)
{
    int *chunk_number_arr;
    if ((chunk_number_arr = (int *)malloc(data_size * sizeof(int))) == NULL) 
    {
		fprintf(stderr, "cannot allocate memory!..\n");
		exit(EXIT_FAILURE);
	}

    fseek(f_binary_data, (seek_point * sizeof(int)), SEEK_SET);
    //printf("FILE CURSOR: %ld\n", ftell(f_binary_data));

    if(fread(chunk_number_arr, sizeof(int), data_size, f_binary_data) != data_size)
    {
        fprintf(stderr, "cannot read file!..\n");
		exit(EXIT_FAILURE);
    }

    return chunk_number_arr;
}

void print_numbers(int *arr, int chunk_size)
{   
    int i = 0;
    while(i < chunk_size)
    {
        printf("%d \n", *(arr + i));
        i++;
    }
    printf("------------------------------------------------------\n");
}

void write_to_tmpfile(FILE *tmpf, int *nbr_arr, int limit)
{
    if (fwrite(nbr_arr, sizeof(int), limit, tmpf) != limit) 
    {
        fprintf(stderr, "cannot write file!..\n");
        exit(EXIT_FAILURE);
    }
}

int read_from_tmpfile(FILE *f, int seek_point)
{
    fseek(f, (seek_point * sizeof(int)), SEEK_SET);

    int val;
    if (fread(&val, sizeof(int), 1, f) > 0)
    {
        return val;
    }

    return INT_MAX;
}

int find_minvalue_index(int array[], int limit) 
{
    int min_index = 0;
    int min = array[min_index];

    for (int i = 1; i < limit; i++)
    {
        if (array[i] < min)
        {
            min_index = i;
            min = array[min_index];
        }
    }
    return min_index;
}

int is_read_op_completed(int array[], int limit)
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

void swap(int *val1, int *val2) 
{ 
    int temp = *val1; 
    *val1 = *val2; 
    *val2 = temp;
}

void bubblesort(int *array, int limit) 
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

void close_files(FILE *tmpfiles[], int limit)
{
    FILE *tmpf;
    int file_counter = 0;

    while(file_counter < limit)
    {
        tmpf = tmpfiles[file_counter];
        close_file(tmpf);
        file_counter++;
    }
}

void print_result(FILE *f_binary_result, int debug)
{
    printf("FULL RESULT\n");
    fseek(f_binary_result, 0, SEEK_SET);

    int valr;
    int previous_val = 0;
    int valid_flag = 1;
    while ( fread(&valr, sizeof(int), 1, f_binary_result) > 0 )
    {   
        if(debug)
        {
            printf("%d \n", valr);
        }
        
        if(previous_val != 0 && previous_val > valr)
        {
            printf("INVALID STATE => pre: %d, curr: %d \n", previous_val, valr);
            valid_flag = 0;
            break;
        }

        previous_val = valr;
    }
    
    if(valid_flag == 1)
    {
        printf("ORDER SUCCESS!\n");
    }
}

int *create_tmpfile_seekpoints(int limit)
{
    int *seek_points_of_tmpfiles = (int*) malloc( limit * sizeof(int) );
    for(int i = 0; i < limit; i++)
    {
        seek_points_of_tmpfiles[i] = 0;
    }
    return seek_points_of_tmpfiles;
}

int *read_numbers_from_tmpfiles(FILE *tmpfiles[], int *seekpoints_of_tmpfiles, int limit, int debug)
{
    FILE *tmpf;
    int *tmpvalues = (int*) malloc( limit * sizeof(int) );

    for(int i = 0; i < limit; i++)
    {
        tmpf = tmpfiles[i];
        int value_from_tmpfile = read_from_tmpfile(tmpf, seekpoints_of_tmpfiles[i]);
        tmpvalues[i] = value_from_tmpfile;

        if(debug)
        {
            printf("TMP-%d: %d  ", i, tmpvalues[i]);
        }
    }
    return tmpvalues;
}

void create_tmpfiles(FILE *f_binary_data, FILE *tmpfiles[], int *chunk_limit_parts, int limit, int debug)
{
    int chunkpart_counter = 0;
    int seek_point = 0;

    while(chunkpart_counter < limit)
    {
        int chunk_size = chunk_limit_parts[chunkpart_counter];

        int *number_arr = read_chunk_from_datafile(f_binary_data, chunk_size, seek_point);
        printf("STEP-%d \n", chunkpart_counter);
        
        if(debug)
        {
            print_numbers(number_arr, chunk_size);
            printf("ORDER-%d \n", chunkpart_counter);
        }
        
        // Bubble sort is slower than quick sort!
        // bubblesort(number_arr, chunk_size);

        qsort(number_arr, chunk_size, sizeof(int), qsort_compare_func);

        if(debug)
        {
            print_numbers(number_arr, chunk_size);
        }

        FILE *tmpfile = create_tmpfile();
        write_to_tmpfile(tmpfile, number_arr, chunk_size);
        tmpfiles[chunkpart_counter] = tmpfile;

        free(number_arr);

        seek_point = seek_point + chunk_size;
        chunkpart_counter++;
    }
}

void print_rawdata(FILE *f_binary_data, int debug)
{   
    if(debug)
    {
        printf("FULL DATA\n");
        fseek(f_binary_data, 0, SEEK_SET);

        int valu;
        while ( fread(&valu, sizeof(int), 1, f_binary_data) > 0 )
        {
            printf("%d \n", valu);
        }
        printf("------------------------------------------------------\n");
    }
}