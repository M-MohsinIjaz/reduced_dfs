#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_CHUNKS 1000
#define CHUNK_SIZE 30
#define BUFFER_SIZE 2 + 4 + 4 + 4 + CHUNK_SIZE
#define MAX_FILES 20
typedef struct 
{
  char *data;

  int id;
}chunk;

typedef struct
{
	int file_id;
	int num_chunks;
	chunk file_chunks[MAX_CHUNKS];
}file;
//chunk recv_chunks[MAX_CHUNCKS];
file files[MAX_FILES];
int file_indexs[MAX_FILES][1];
int chunk_counter=0,global_file_index=0;
int send_payload(int sock ,char *payload,int len)
{	
	int payload_length = len;
	if (send(sock, payload, payload_length, 0) < 0) 
	{
		perror("send error");
		return -1;
	}
	return 0;
}
int check_cache(int file_id)
{	
	int file_index = -1;
	for (int i=0;i<MAX_FILES;i++)
	{	
	
			printf("id %d \n",files[i].file_id);
			if (files[i].file_id == file_id)
			{
				file_index = i;
				break;
			}
		
	}
	return file_index;
}
int print_list_of_files()
{
for (int i=0;i<MAX_FILES;i++)
	{
		if (files[i].file_id == NULL)
		{
			break;
		}
		printf("File ID %d \n",files[i].file_id);
	}
}
int get_file_index(int file_id)
{
	int file_index=-1;
	for (int i=0;i<MAX_FILES;i++)
	{
		if (files[i].file_id == file_id)
		{
			file_index = i;
			break;
		}
	}

	if (file_index == -1)
	{
		
		files[global_file_index].file_id = file_id;
		files[global_file_index].num_chunks = 0;
		
		file_index =  global_file_index;
		global_file_index++;
			//printf("New file index %d \n",file_index);
	//printf("New global index %d \n",global_file_index);
	}
	return file_index;
}

typedef struct
{
	int len;
	char *data;
}r_data;
r_data *create_payload(char *data,int file_id,int data_len)
{
	int payload_length = 1+4+4 + strlen(data);
	r_data *pload = (r_data *)malloc(4+payload_length);
	printf("Payload Length %d \n",payload_length);
	char *payload = (char *)malloc(payload_length);
	memset(payload, 0, sizeof(payload_length));
	char start = '$';
	memcpy(payload, &start, sizeof(char));
	memcpy(payload+1, &data_len, sizeof(int));
	memcpy(payload+5, &file_id, sizeof(int));
	memcpy(payload+9, data, payload_length-9);
	printf("Payload Data %s \n",payload+9);
	//send_payload(int sock ,char *payload);			
	pload->data = payload;
	pload->len = payload_length;
	return pload;
}
r_data *read_chunk(char *buffer)
{	int chunk_id;	
	int file_id,file_index;
	r_data *data;
	memcpy(&file_id,buffer+2,4);
	memcpy(&chunk_id,buffer+6,4);
	file_index = check_cache(file_id);
	printf("Requested File ID %d Chunk_ID :%d File_index %d\n",file_id,chunk_id,file_index);
	//for (int i=0;i<MAX_CHUNCKS;i++)
	//{
		//if (files[file_index]->file_chunks[chunk_id].id == chunk_id)
		//{
			data=  create_payload(files[file_index].file_chunks[chunk_id].data,file_id,strlen(files[file_index].file_chunks[chunk_id].data));
		//}
	//}
	return data;
}




int write_chunk(char *buffer)
{	
	int file_id,file_index,payload_length;
	memcpy(&payload_length,buffer+2,4);
	memcpy(&file_id,buffer+6,4);
	file_index=get_file_index(file_id);
	printf("*********************************************************************\n");
		printf("Write file ID %d \n",file_id);
	
	printf("Write file index %d \n",file_index);
	printf("Write global index %d \n",global_file_index);
	
	chunk_counter = files[file_index].num_chunks;
	printf("Write chunk_counter %d \n",chunk_counter);
	//chunk recv_chunks = files[file_index].file_chunks[chunk_counter];
	files[file_index].file_chunks[chunk_counter].data = (char *)malloc(payload_length-14);
	
	
	memcpy(&files[file_index].file_chunks[chunk_counter].id,buffer+10,4);
	memcpy(files[file_index].file_chunks[chunk_counter].data,buffer+14,payload_length-14);
        
        printf("\nCHUNK ID: %d \n", files[file_index].file_chunks[chunk_counter].id);
    	printf("Received Data Length: %ld\n", strlen(files[file_index].file_chunks[chunk_counter].data));
    	printf("Received Data : %.*s\n", payload_length-14, files[file_index].file_chunks[files[file_index].file_chunks[chunk_counter].id].data);
    	//printf("Received Data : %s\n", files[file_index].file_chunks[files[file_index].file_chunks[chunk_counter].id].data);
    	files[file_index].num_chunks++;
    	printf("\n*********************************************************************\n");
    	return 0;
}
void *client_handler(void *arg) {
    int clientfd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int i=0;
    
    while ((bytes_received = recv(clientfd, buffer, BUFFER_SIZE, 0)) > 0) {
	
    	if (buffer[1] == 'W')
    	{
    		write_chunk(buffer);
    	}
    	if (buffer[1] == 'R')
    	{	//sleep(1);
    		r_data *d = read_chunk(buffer);
    	        send_payload(clientfd,d->data,d->len);
    	}
        
    	//printf("Received: %d\n", *((int *)(buffer+1)));
    	
        //printf("Received: %s", buffer);
        memset(buffer,0,BUFFER_SIZE);
        i++;
    }
    close(clientfd);
}
int main() {
    // Create a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int new_socket;
    if (sockfd == -1) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }
    
    // Bind the socket to IP address 127.0.0.4 and port 5500
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.4");
    server_addr.sin_port = htons(7000);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections
    if (listen(sockfd, 1) == -1) {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }
    /*
    // Accept a client connection
    int clientfd = accept(sockfd, NULL, NULL);
    if (clientfd == -1) {
        perror("accept() error");
        exit(EXIT_FAILURE);
    }*/
    memset(files,-1,(MAX_FILES*sizeof(file))); 
    print_list_of_files();   
    // Receive data from the client and print it
    while (1) {
        // Accept a new connection
        if ((new_socket = accept(sockfd, NULL,NULL)) < 0) 
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Create a new thread to receive data from the client
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, &new_socket) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread so that it can run independently
        if (pthread_detach(tid) != 0) {
            perror("pthread_detach failed");
            exit(EXIT_FAILURE);
        }

        // Start listening for new connections again
    }
    
    // Close the client and server sockets
    //close(clientfd);
    close(sockfd);
    
    return 0;
}


