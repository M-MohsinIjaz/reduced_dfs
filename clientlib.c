#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clientlib.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>



//chunk *chunk_table[MAX_NUM_CHUNKS];
master master_server = {
    .ip_address = "127.0.0.1",
    .port_num = 6600
};

file_metadata files[MAX_FILES];

int global_file_index=0;
chunk ** create_chunks(int num_of_chunks,int *chunk_id_offset)
{
    chunk **chunk_table = (chunk **)malloc(num_of_chunks * sizeof(chunk*));
    char *str = (char *)malloc(CHUNK_SIZE);
    int chunk_count = 0;
    memset(str,0,CHUNK_SIZE);
    while (chunk_count < num_of_chunks) 
    {
        chunk_table[chunk_count] =(chunk *) malloc(sizeof(chunk));
	// assign chunk an id
	chunk_table[chunk_count]->chunk_id = *chunk_id_offset;
	*chunk_id_offset = (*chunk_id_offset)+1;
	printf("\r\n Enter a Chunk data \n\r");
    	fgets(str, CHUNK_SIZE, stdin);
	chunk_table[chunk_count]->data = (void *)malloc(strlen(str)-1);
	chunk_table[chunk_count]->len = strlen(str)-1;
	memcpy(chunk_table[chunk_count]->data,(void *)str,strlen(str)-1);
	memset(str,0,CHUNK_SIZE);
	chunk_count++;
    }
    return chunk_table;
}

node * append_file(int file_id,chunk **chunks,int num_chunks)
{
	int file_index=check_cache(file_id);
	
	node *node_list;
	printf("\n\r Append File With ID %d \n\r",file_id);
	
	if (file_index !=-1)
	{
		node_list = files[file_index].nodes;
	}
	else
	{
		printf("\r\n File Info is not in cache getting info from server \n\r");
		node_list = send_read_request_to_server(file_id);
	}

	send_chunks_to_nodes(node_list,chunks,num_chunks,file_id);
		return node_list;
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
		global_file_index++;
		files[global_file_index-1].file_id = file_id;
		
		return global_file_index-1;
	}
	else
	{
		return file_index;
	}
}

chunk_metadata * divide_file_into_chunks(FILE *fp,int file_size) {
    int remaining_size, chunk_size;
    int chunk_id = 0;
    int NUM_CHUNKS = file_size/CHUNK_SIZE +1;
    chunk **chunk_table = malloc(NUM_CHUNKS * sizeof(chunk*));
    // Divide the file into chunks and write each chunk to a separate file
    remaining_size = file_size;
    printf("File Size %d \n",file_size);
    int chunk_count = 0;
    while (remaining_size > 0) {
        // Determine the size of the next chunk
        if (remaining_size >= CHUNK_SIZE) {
            chunk_size = CHUNK_SIZE;
        } else {
            chunk_size = remaining_size;
        }
        chunk_table[chunk_count] =(chunk *) malloc(sizeof(chunk));
	// assign chunk an id
	chunk_table[chunk_count]->chunk_id = chunk_id;
	chunk_table[chunk_count]->len = chunk_size;
	chunk_id++;
	chunk_table[chunk_count]->data = (void *)malloc(chunk_size);
	char *buffer = (char *) malloc(chunk_size);
        // Read the chunk into the buffer
        if (fread(buffer, chunk_size, 1, fp) != 1) {
            printf("Error: Could not read chunk from file\n");
            free(chunk_table[chunk_count]->data);
            return NULL;
        }
        

        //((char *)chunk_table[chunk_count]->data)[chunk_size] = 'X';
	memcpy(chunk_table[chunk_count]->data,(void *)buffer,chunk_size);	
	remaining_size -= chunk_size;
        chunk_count++;

    }

    printf("\r\n File divided into %d chunks successfully \n\r",chunk_count);
    chunk_metadata *chunk_info = (chunk_metadata *)malloc(sizeof(chunk_metadata));
    chunk_info->chunk_table = chunk_table;
    chunk_info->num_of_chunks = chunk_count;
    return chunk_info;
}

char *generate_cf_request(const char *file_name)
{
    // $(1 byte)+(4bytes)(length of payload)+(2bytes)(instruction code)+(length of filename)(file_name)
    int payload_length = 1 + 4 + 2 + strlen(file_name);

    char *payload = (char *)malloc(payload_length);
    char start[] = "$";
    char cf[2]="CF";
    // Copy the string into the payload buffer
    char *temp = payload+1;
    memcpy(payload, &start[0], sizeof(char));
    // Convert the integer to network byte order
    //int payload_length_nbo = htonl(payload_length);
    
    // Copy the payload length in network byte order into the payload buffer
    memcpy(temp, &payload_length, sizeof(payload_length));
    
    // Copy the instruction code into the payload buffer
    memcpy(payload + sizeof(char) + sizeof(payload_length), &cf, 2);
    
    // Copy the filename into the payload buffer
    memcpy(payload + sizeof(char) + sizeof(payload_length) + sizeof(cf), file_name, strlen(file_name));

    
    return payload;
}
node * decode_rf_rsp_payload(char *payload) {
	node *nodes = (node *)malloc(sizeof(node)*3);
    int payload_length;
    int file_id;
    
    memcpy(&payload_length, payload + 1, sizeof(payload_length));
    memcpy(&file_id, payload + 5, sizeof(payload_length));
  
    // Copy the instruction code into the payload buffer
    memcpy(&nodes[0].ip_address,payload + 9, 16);
    memcpy(&nodes[0].port,payload + 25, 4);
    
    memcpy(&nodes[1].ip_address,payload + 29, 16);
    memcpy(&nodes[1].port,payload + 45, 4);
    
    memcpy(&nodes[2].ip_address,payload + 49, 16);
    memcpy(&nodes[2].port,payload + 65, 4);
 
  return nodes;
}
node * decode_cf_rsp_payload(char *payload) {
    
    node *nodes = (node *)malloc(sizeof(node)*3);
    int payload_length;
    int file_id;
    
    memcpy(&payload_length, payload + 1, sizeof(payload_length));
    memcpy(&file_id, payload + 5, sizeof(payload_length));
    
  
    // Copy the instruction code into the payload buffer
    memcpy(&nodes[0].ip_address,payload + 9, 16);
    memcpy(&nodes[0].port,payload + 25, 4);
    
    memcpy(&nodes[1].ip_address,payload + 29, 16);
    memcpy(&nodes[1].port,payload + 45, 4);
    
    memcpy(&nodes[2].ip_address,payload + 49, 16);
    memcpy(&nodes[2].port,payload + 65, 4);
    files[global_file_index].file_id = file_id;
    return nodes;
}

void print_node(node *node_list)
{
	for (int i=0;i<3;i++)
	{
		printf("Node %d IP: %s\n", i,node_list[i].ip_address);
    		printf("Node %d PORT: %d\n", i,node_list[i].port);
	}
}



int create_payload(chunk **chunk_table,int num_chunks)
{
	char header[1]="$";
	int data_len = 0;
	int sock = initialize_server_connection();
	for (int i=0;i<num_chunks;i++)
	{
		data_len = strlen((char *)chunk_table[i]->data);
		char *payload=(char *)malloc(data_len+(2*sizeof(int))+1);
		//*payload=header;
				
		sprintf(payload, "%s%d%d%s", header, chunk_table[i]->chunk_id,data_len,(char *) chunk_table[i]->data);
		send_chunks_to_server(sock,payload);
		free(payload);
	}
	close_server_connection(sock);
	return 0;
}


int send_chunks_to_server(int sock,char *payload)
{

	printf("Sending data");
	send_tcp_data(sock, payload, 18);
 
 	return 0;
}
int send_tcp_data(int sock, const char* data, int data_length) {


    if (send(sock, data, data_length, 0) < 0) {
        perror("send");
        return -1;
    }

   
    return 0;
}

node * request_master_create_file(char *payload)
{
	int sock = initialize_server_connection();
	char buffer[1024] = {0};
	send_chunks_to_server(sock,payload);
	printf("\r\n CF Response Received From Server : Payload Length %ld \n\r",recv(sock, buffer, sizeof(buffer), 0));
	close_server_connection(sock);
    	
	return decode_cf_rsp_payload(buffer);
	
}
char *generate_rf_request(int file_id)
{
    // $(1 byte)+(4bytes)(length of payload)+(2bytes)(instruction code)+(length of filename)(file_name)
    int payload_length = 1 + 2 + 4 + 4;

    char *payload = (char *)malloc(payload_length);
    char start[] = "$";
    char cf[2]="RF";
    memcpy(payload, &start[0], sizeof(char));

    // Convert the integer to network byte order
    //int payload_length_nbo = htonl(payload_length);
    
      memcpy(payload+1, &payload_length, sizeof(int));

    // Copy the instruction code into the payload buffer
    memcpy(payload + 5, &cf, 2);
    
    memcpy(payload+7, &file_id, sizeof(int));

    return payload;
}
node * send_read_request_to_server(int file_id)
{
	char *payload= generate_rf_request(file_id);
	int sock = initialize_server_connection();
	char buffer[1024] = {0};
	send_chunks_to_server(sock,payload);
	printf("\r\n RF Response Received From Server : Payload Length %ld  \r\n" ,recv(sock, buffer, sizeof(buffer), 0));
	close_server_connection(sock);
    
	return decode_rf_rsp_payload(buffer);
	
}

int create_file(const char *filename)
{
    FILE *fp;
    int file_size;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: Could not open file for reading\n");
        return 0;
    }

    // Get the size of the file
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    file_size--;
    fseek(fp, 0L, SEEK_SET);
    chunk_metadata *chunk_info = divide_file_into_chunks(fp,file_size);
    print_chunks(chunk_info->chunk_table,chunk_info->num_of_chunks);
    char *payload = generate_cf_request(filename);
    files[global_file_index].nodes = request_master_create_file(payload);
    
    memcpy(files[global_file_index].file_name,filename,strlen(filename));
    send_chunks_to_nodes(files[global_file_index].nodes,chunk_info->chunk_table,chunk_info->num_of_chunks,files[global_file_index].file_id); 

    sleep(1);
    global_file_index++;
    fclose(fp);
    return files[global_file_index-1].file_id;
}

int check_cache(int file_id)
{	
	int file_index = -1;

	for (int i=0;i<MAX_FILES;i++)
	{		
		if (files[i].file_id == file_id)
		{
			file_index = i;
			break;
		}
	}
	return file_index;
}

char ** read_chunk(int file_id,int *chunk_ids,int number_of_chunks)
{	
	
	
	int file_index=check_cache(file_id);
	
	node *node_list;
	char buffer[40];
	char **chunk_list = (char **)malloc(number_of_chunks * sizeof(char *));
;
	printf("\r\n Read File With ID %d \n\r",file_id);
	
	if (file_index !=-1)
	{
		node_list = files[file_index].nodes;
	}
	else
	{
		printf("\r\n Get Available Nodes List From Master \n\r");
		node_list = send_read_request_to_server(file_id);
	}
	
	int sock=-1;
	int i=0;
	while (i<3)
	{
		sock = connect_with_node(node_list[i].ip_address,node_list[i].port);
		if (sock>-1)
		{
			printf("\r\n Connected Node IP Address is %s \n\r",node_list[i].ip_address);
			break;
		}
		else
		{
			i++;
		}
	}
	
	int payload_length = 2 + 4 + 4;
	
	for (i=0;i<number_of_chunks;i++)
	{
		char *payload = (char *)malloc(payload_length);
		memset(payload, 0, sizeof(payload_length));
		char start[] = "$R";
		memcpy(payload, start, (2*sizeof(char)));
		memcpy(payload+2, &file_id, sizeof(int));
		memcpy(payload+6, &chunk_ids[i], sizeof(int));
		
		if (send(sock, payload, payload_length, 0) < 0) 
		{
			perror("send");
			return NULL;
		}
		free(payload);

		printf("\r\n********************************************************************* \n\r");
		printf("\r\nNumber of bytes received from storage Node: %ld \n\r",recv(sock, buffer, sizeof(buffer), 0));
		int rl;
		memcpy(&rl, buffer+1, sizeof(int));
		chunk_list[i] = (char *)malloc(rl);
		memcpy(chunk_list[i], buffer+9, rl);
		printf("\r\n Received Data : %.*s \n\r", rl, chunk_list[i]);

		printf("\r\n********************************************************************* \n\r");
		memset(buffer, 0, 40);
	}
	close_server_connection(sock);
	return chunk_list;
}	

int send_chunks_to_nodes(node *node_list,chunk **chunk_table,int num_chunks,int file_id)
{	
	int sock;
	for (int i=0;i<3;i++)
	{
		sock = connect_with_node(node_list[i].ip_address,node_list[i].port);
		if (sock>-1)
		{
			for (int j=0;j<num_chunks;j++)
			{
				int payload_length = 2 + 4 + 4 + 4 + chunk_table[j]->len;
				char *payload = (char *)malloc(payload_length);
				memset(payload, 0, sizeof(payload_length));
				char start[] = "$W";
				memcpy(payload, start, (2*sizeof(char)));
				memcpy(payload+2, &payload_length, sizeof(int));

				memcpy(payload+6, &file_id, sizeof(int));
				memcpy(payload+10, &chunk_table[j]->chunk_id, sizeof(int));
				memcpy(payload+14, chunk_table[j]->data, payload_length-14);			

				if (send(sock, payload, payload_length, 0) < 0) {
				perror("send");
				return -1;
				}

				printf("\r\n Chunk with ID %d Sent  \r\n",chunk_table[j]->chunk_id);
				sleep(1);
				free(payload);
			}
		close(sock);
		}
		else
		{
			printf("\r\n Node with IP: %s and Port:%d is OFFLINE \n\r",node_list[i].ip_address,node_list[i].port);
		}
	}
	return 0;
}
int connect_with_node(char *node_ip,int node_port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(node_ip);
    server_addr.sin_port = htons(node_port);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("can not connect to node");
        return -1;
    }
  return sock;
	
}
int initialize_server_connection()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(master_server.port_num);
    if (inet_pton(AF_INET, master_server.ip_address, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("can not connect to master");
        return -1;
    }
  return sock;
}
int close_server_connection(int sock)
{
	close(sock);
	return 0;
}
void print_chunks(chunk **chunk_table,int num_chunks)
{
	printf("\r\n********************************************************************* \n\r");
	for (int i=0;i<num_chunks;i++)
	{
		printf("\r\n CHUNK ID: %d \n\r", chunk_table[i]->chunk_id);
		// If data is required:
		printf("\r\n CHUNK DATA: %s \n\r", (char *)chunk_table[i]->data);
	}
	printf("\r\n********************************************************************* \n\r");
}

