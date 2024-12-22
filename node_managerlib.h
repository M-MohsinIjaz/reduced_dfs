// Define the structure for each node in the linked list
#define MAX_NODE_FILES 20
#define MAX_NODES_ASSIGNED 3
#define MAX_NODES 5
typedef struct storage_node {
    char ip_address[16];
    int port;
    char status;
    int num_files;
    int node_id;
    struct storage_node *next;
} storage_node;

//void print_node(node *node_list);
void decode_cf_rsp_payload(char *payload);

int populate_nodes();
int ping_test(char *ip_address);
void insertNode(storage_node **head, char *newData,int position);
void removeNode(storage_node **head, int key);
void printList(storage_node **node,int number_of_nodes);
int check_node_availability(storage_node *node);
int update_nodes_status(storage_node **node,int number_of_nodes);
void add_node(storage_node **head, storage_node *node,int position);
storage_node ** get_node_list(storage_node **node_list,int num_of_nodes);
char *generate_cf_response(storage_node **node_list, int file_id);


void print_node_list_per_file(storage_node ***files_nodes);
