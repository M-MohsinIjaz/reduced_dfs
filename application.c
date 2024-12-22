#include <stdio.h>
#include "clientlib.h"
#include <stdlib.h>
#include <string.h>
extern file_metadata files[MAX_FILES];
int main() {
    FILE *fp;
    char input[1000];
    fp = fopen("newfile.txt", "w");

    if (fp == NULL) {
        printf("Error: Could not create file\n");
        return 1;
    }

    // Read user input
    printf("Enter some text: ");
    fgets(input, 1000, stdin);

    // Write user input to file
    fprintf(fp, "%s", input);

    // Close file
    fclose(fp);
    memset(files,-1,(MAX_FILES * sizeof(file_metadata)));
    printf("Successfully wrote to file.\n");
    printf("File created successfully\n");
    int file_id = create_file("newfile.txt");
    int file_offset = 1;
    //printf("File_ID : %d \n",create_file("newfile.txt"));
    //printf("File_ID : %d \n",create_file("newfile.txt"));
    //printf("File_ID : %d \n",create_file("newfile.txt"));
    //printf("File_ID : %d \n",create_file("newfile.txt"));
    chunk **chunks = create_chunks(2,&file_offset);
    /*
    for (int i=0;i<2;i++)
    {
    	printf("Chunk ID : %d \n",chunks[i]->chunk_id);
    	printf("LLL : %d \n",chunks[i]->len);
    	printf("Received Data : %.*s\n",chunks[i]->len,(char *)chunks[i]->data);
    }*/
    append_file(file_id,chunks,2);

    int chunk_ids[1]={2};
    read_chunk(file_id,chunk_ids,1);

    return 0;
}

