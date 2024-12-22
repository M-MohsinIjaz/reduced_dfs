
#ifndef MYHEADER_H
#define MYHEADER_H
#define MAX_NUM_CHUNKS 100
#define CHUNK_SIZE 30 // Specify the chunk size in bytes here
#define CREATE_FILE "CF"
#define APPEND_FILE "AF"
#define NODE_FAILURE "NF"
#define MAX_FILES 1000


typedef struct {
    int chunk_id;
    int len;
    void *data;
} chunk;
typedef struct
{
	char ip_address[16];
	int port_num;
}master;
typedef struct 
{
	int num_of_chunks;
	chunk **chunk_table;
}chunk_metadata;
typedef struct
{
	char ip_address[16];
	int port;
}node;
typedef struct
{
	int file_id;
	char file_name[100];
	node *nodes;
}file_metadata;
/* Function declaration */
node * decode_cf_rsp_payload(char *payload);
void print_node(node *node_list);
char *generate_cf_request(const char *file_name);
node * request_master_create_file(char *payload);

int create_payload(chunk **chunk_table,int num_chunks);
int send_chunks_to_server(int sock,char *payload);

void print_chunks(chunk **chunk_table,int num_chunks);
int send_tcp_data(int sock, const char* data, int data_length);
int initialize_server_connection();
int close_server_connection(int sock);
int check_cache(int file_id);
node * send_read_request_to_server(int file_id);

int send_chunks_to_nodes(node *node_list,chunk **chunk_table,int num_chunks,int file_id);
int connect_with_node(char *node_ip,int node_port);
char ** read_chunk(int file_id,int *chunk_ids,int number_of_chunks);
//int append_file(int file_id,chunk **chunks,int num_chunks);
node * append_file(int file_id,chunk **chunks,int num_chunks);

chunk ** create_chunks(int num_of_chunks,int *chunk_id_offset);
/***********************************************************************
		Client Library APIs
************************************************************************/
chunk_metadata * divide_file_into_chunks(FILE *fp,int file_size);
int create_file(const char *filename);
#endif /* MYHEADER_H */

