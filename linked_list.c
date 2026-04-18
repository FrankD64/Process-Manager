#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

Node * add_newNode(Node* head, pid_t new_pid, char * new_path){
	//malloc(size of memory) allocates that memory dynamically
	Node* new_node = (Node*) malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for node");
        return head;
    }

    new_node->pid  = new_pid;
    new_node->path = new_path;
    new_node->next = head;

    return new_node;
}


Node * deleteNode(Node* head, pid_t pid){
	// your code here
	Node * temp = head;
    Node * prev = NULL;

    if (temp == NULL){
        return head;
    }

    // If head node itself holds the PID to be deleted
    if (temp->pid == pid) {
        head = temp->next; 
		//deallocate the memory allocated by malloc
        free(temp->path);
        free(temp); 
        return head;
    }

	while (temp != NULL && temp->pid != pid) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == NULL){
		return head;
	};
	
    prev->next = temp->next;

    free(temp->path);
    free(temp); 
    return head;
}

void printList(Node *node){
	// your code here
	int counter = 0;
    while (node != NULL){
        printf("%d: %s\n", node->pid, node->path);
        node = node->next;
        counter++;
    }
    printf("Total background jobs: %d\n", counter);
}


int PifExist(Node *node, pid_t pid){
	while (node != NULL){
        if (node->pid == pid){
          return 1;
        }
        node = node->next;
    }
    return 0;
}

