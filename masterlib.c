#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "node_managerlib.h"
#define PORT 6600
#define MAX_FILES 1000
/*
typedef struct storage_node {
    char *ip_address;
    int port;
    char status;
    int  file_id[20];
    struct storage_node *next;
} storage_node;
*/
storage_node **head;
storage_node **files_nodes[MAX_FILES];
extern char nodes_ips[5][16];
int file_counter=0;
typedef struct {

    storage_node node[3];
} file_metadata;

char * function_CF() {
    storage_node **available_nodes = NULL;
    available_nodes = get_node_list(head,MAX_NODES);
    files_nodes[file_counter] = available_nodes;
    file_counter++;
    print_node_list_per_file(files_nodes);
    char *payload = generate_cf_response(available_nodes,file_counter-1);
    //print_node(node_list);
    printf("Function for CF called.\n");
    return payload;
}
void print_node_list_per_file(storage_node ***files_nodes)
{
	for (int i=0;i<file_counter;i++)
	{
		for (int j=0;j<3;j++)
		{
			printf("File_id: %d  NodeIP: %s \n",i,files_nodes[i][j]->ip_address);
			printf("File_id: %d  NodePort: %d \n",i,files_nodes[i][j]->port);
		}
	}
}
void function_AF() {
    printf("Function for AF called.\n");
}

void function_NF() {
    printf("Function for NF called.\n");
}

void function_DF() {
    printf("Function for DF called.\n");
}

char * function_RF(char *payload) {
    printf("Function for RF called.\n");
    int file_id;
    memcpy(&file_id, payload+7,sizeof(int));
    
    //print_node_list_per_file(files_nodes);
    printf("Read Requested File ID : %d \n",file_id);
    printf("Read Requested IP : %s \n",files_nodes[file_id][0]->ip_address);
    char *payload_rsp = generate_cf_response(files_nodes[file_id],file_id);
    return payload_rsp;
}

char *call_function_based_on_string(char *str,char *payload) {
    if (strcmp(str, "CF") == 0) {
        // call function for "CF" case
        return function_CF();
    } else if (strcmp(str, "AF") == 0) {
        // call function for "AF" case
        function_AF();
    } else if (strcmp(str, "NF") == 0) {
        // call function for "NF" case
        function_NF();
    } else if (strcmp(str, "DF") == 0) {
        // call function for "DF" case
        function_DF();
    } 
    else if (strcmp(str, "RF") == 0) {
        // call function for "RF" case
        return function_RF(payload);
    } else {
        printf("Invalid string: %s\n", str);
        return NULL;
    }
}

char * decode_instruction(char *payload)
{
	    // Extract the instruction code (2 bytes)
    char instruction_code[2];
    memcpy(instruction_code, payload + 5, sizeof(instruction_code));
    printf("Instruction: %.*s\n", 2,instruction_code);
    //decode_payload(payload);
    char *response = call_function_based_on_string(instruction_code,payload);
    //decode_cf_rsp_payload(response);
    return response;
}
void decode_payload(char *payload) {
    // Extract the payload length (4 bytes)
    
    int payload_length;
    memcpy(&payload_length, payload + 1, sizeof(payload_length));
    //payload_length = ntohl(payload_length);

    // Extract the instruction code (2 bytes)
    char instruction_code[2];
    memcpy(instruction_code, payload + 5, sizeof(instruction_code));

    // Extract the file name (variable length)
    char file_name[payload_length-7];
    memcpy(file_name, payload + 7, payload_length - 7);

    // Null-terminate the file name string
    //file_name[payload_length - 8] = '\0';

    // Print the decoded values
    printf("Payload length: %d\n", payload_length);
    printf("Instruction: %.*s\n", 2,instruction_code);
    printf("file name: %.*s\n", payload_length-7,file_name);
    //call_function_based_on_string(char *str)
}
// This is the function that will be executed in the new thread to receive data from the client
void *receive_data(void *arg) {
    int sock = *(int *)arg;
    char buffer[1024] = {0};
    ssize_t num_bytes;

    // Receive data from the client until the connection is closed
    while ((num_bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        printf("Received %ld bytes: %s\n", num_bytes, buffer);

	char *response = decode_instruction(buffer);
	//printf("Instruction: %.*s\n", 2,instruction);
    	    if (send(sock, response, 69, 0) < 0) {
        perror("send");
        return -1;
    }

	//call_function_based_on_string(instruction);
        //decode_payload(buffer);
	
        memset(buffer, 0, sizeof(buffer));
    }

    // If recv returns 0, the client has closed the connection
    if (num_bytes == 0) {
        printf("Client disconnected.\n");
    } else {
        perror("recv failed");
    }

    close(sock);
    return NULL;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    head = (storage_node **)malloc(MAX_NODES * sizeof(storage_node));
       populate_nodes();
    // Insert nodes into the linked list
    insertNode(head, nodes_ips[0],0);
    insertNode(head, nodes_ips[1],1);
    insertNode(head, nodes_ips[2],2);
    insertNode(head, nodes_ips[3],3);
    insertNode(head, nodes_ips[4],4);

    printf("Original linked list: ");
    printList(head,MAX_NODES);
    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Initialize address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Create a new thread to receive data from the client
        pthread_t tid;
        if (pthread_create(&tid, NULL, receive_data, &new_socket) != 0) {
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

    return 0;
}

