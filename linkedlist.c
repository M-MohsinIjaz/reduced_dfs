#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node_managerlib.h"
char nodes_ips[5][16];
int ports[5];
int populate_nodes()
{
	strcpy(nodes_ips[0], "127.0.0.1");
	ports[0] = 6500;
	strcpy(nodes_ips[1], "127.0.0.2");
	ports[1] = 6800;
	strcpy(nodes_ips[2], "127.0.0.3");
	ports[2] = 6900;
	strcpy(nodes_ips[3], "127.0.0.4");
	ports[3] = 7000;
	strcpy(nodes_ips[4], "127.0.0.5");
	ports[4] = 7100;
}
int ping_test(char *ip_address)
{
    char command[50];
    sprintf(command, "ping -c 1 %s", ip_address);
    int result = system(command);
    if (result == 0) {
        printf("Ping successful\n");
    } else {
        printf("Ping failed\n");
    }
    return result;
}

// Function to insert a new node at the beginning of the linked list
void insertNode(storage_node **head, char *newData,int position) {
    // Allocate memory for the new node
    storage_node *newNode = (storage_node *)malloc(sizeof(storage_node));
    strcpy(newNode->ip_address, newData);
    newNode->port = ports[position];

    head[position] = newNode;
}

// Function to remove a node from the linked list
void removeNode(storage_node **head, int key) {
    // Create a temporary pointer to the head node and a pointer to the previous node
    storage_node *temp = *head, *prev = NULL;

    // If the head node contains the key, remove it
    if (temp != NULL && temp->status == key) {
        *head = temp->next; // Change the head pointer
        free(temp); // Free the memory of the removed node
        return;
    }

    // Traverse the linked list to find the node with the key
    while (temp != NULL && temp->status != key) {
        prev = temp;
        temp = temp->next;
    }

    // If the key was not found in the linked list, return
    if (temp == NULL) {
        return;
    }

    // Remove the node with the key
    prev->next = temp->next; // Set the next pointer of the previous node to the next node
    free(temp); // Free the memory of the removed node
}

// Function to print the contents of the linked list
void printList(storage_node **node,int number_of_nodes) {
    for (int i=0;i<number_of_nodes;i++) {
        printf("ip address %s \n\r", node[i]->ip_address);
        printf("port %d \n\r", node[i]->port);
        printf("status %d \n\r", node[i]->status);
        //node = node->next;
    }
    printf("\n");
}
int check_node_availability(storage_node *node)
{
	if (node->status == 0)
	{
	return 0;
	}
	else
	{
		if (node->num_files < MAX_NODE_FILES)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
}
int update_nodes_status(storage_node **node,int number_of_nodes)
{	for (int i=0;i< number_of_nodes;i++)
	{
		node[i]->status = !ping_test(node[i]->ip_address);
		//node=node->next;
	}
}

void add_node(storage_node **head, storage_node *node,int position) 
{
    // Allocate memory for the new node
    head[position] = node;
    node->num_files++;
}
storage_node ** get_node_list(storage_node **node_list,int num_of_nodes)
{
	int result;
	//storage_node *available_nodes[MAX_NODES_ASSIGNED]={NULL};
	storage_node **available_nodes=(storage_node **)malloc(MAX_NODES_ASSIGNED * sizeof(storage_node));
	int node_assigned = 0;
	update_nodes_status(node_list,MAX_NODES);
	for (int i=0;i<num_of_nodes;i++)
	{
		
		result = check_node_availability(node_list[i]);
		if (result==1)
		{
			add_node(available_nodes,node_list[i],node_assigned);
			node_assigned++;
		}
		if (node_assigned==MAX_NODES_ASSIGNED)
		{
			printf("MAX ACHIVED %d \n",node_assigned);
			break;
		}
		
		
	}
	if (node_assigned < MAX_NODES_ASSIGNED)
	{
		printf("Nodes are Busy");
	}
	return available_nodes;
}
char *generate_cf_response(storage_node **node_list,int file_id)
{
    // $(1 byte)+(4bytes)(length of payload)+(2bytes)(instruction code)+(length of filename)(file_name)
    
    int payload_length = 1+4+4+3*(16+4);

    char *payload = (char *)malloc(payload_length);
    char start[] = "$";
    // Copy the string into the payload buffer
    memcpy(payload, &start[0], sizeof(char));

    // Copy the payload length in network byte order into the payload buffer
    memcpy(payload+1, &payload_length, sizeof(payload_length));
    memcpy(payload+5, &file_id, sizeof(int));
    
    // Copy the instruction code into the payload buffer
    memcpy(payload + 9  , node_list[0]->ip_address, 16);
    memcpy(payload + 25  , &node_list[0]->port, 4);
    
    memcpy(payload + 29  , node_list[1]->ip_address, 16);
    memcpy(payload + 45  , &node_list[1]->port, 4);
    
    memcpy(payload + 49  , node_list[2]->ip_address, 16);
    memcpy(payload + 65  , &node_list[2]->port, 4);
    //sprintf(payload, "PayLoad: %c%d%d%s%d%s%d%s%d", a, b, c);
    printf("CF length : %d",(*(payload+1)));
    return payload;
}



/*
int main() {
    storage_node **head = (storage_node **)malloc(MAX_NODES * sizeof(storage_node));
    storage_node **available_nodes = NULL;
    populate_nodes();
    // Insert nodes into the linked list
    insertNode(head, nodes_ips[0],0);
    insertNode(head, nodes_ips[1],1);
    insertNode(head, nodes_ips[2],2);
    insertNode(head, nodes_ips[3],3);
    insertNode(head, nodes_ips[4],4);

    printf("Original linked list: ");
    printList(head,MAX_NODES);
    //update_nodes_status(head,MAX_NODES);
    // Remove a node from the linked list
    //removeNode(&head, 2);
    available_nodes = get_node_list(head,MAX_NODES);
    char *payload = generate_cf_response(available_nodes);
    decode_cf_rsp_payload(payload);
    //printf("Linked list after removing node with key 2: ");
    //printList(available_nodes,MAX_NODES_ASSIGNED);

    return 0;
}
*/
