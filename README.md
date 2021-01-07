# c-bigdata-sorter
Simple big data sorter prototype in C language in restricted dynamic memory conditions like micro controllers

Basically, If we need to sort big data in restricted dynamic memory conditions, we can use temporary files. So you can split big data file into some chunks.
After that, we choose a sort algorithm, which we have used Buble Sort Algorithm in this example, now we can sort chunk data and then can save to a temporary file.

After splitting şnto chunks, now we can utilize merge strategy to create one single sorted file. In this method, you should read records from temporary files,
after that, you have to select the minimum value between temporary files. And you have to write this value to the result file. 
And you have to increment the seek value of the temporary file which had the minimum value. 
Then, we have to iterate this steps until all the data have been written to result file.

# Steps

- Split big data file into some chunks
- Create temporary file from these chunk data
- Sort the chunk data in asc mode
- Write chunk data to temporary file
- Then start merging process which uses temporary files
- In iteration step find the minimum value from temporary file
- Write minimum value to result file
- Increment seek value of the temporary file which has minimum file

