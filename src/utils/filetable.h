#include <arpa/inet.h>          // inet_ntoa
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include "../utils/constants.h"

#define MAX_PEER_NUM 32
#define FILE_TYPE 8
#define FOLDER_TYPE 4

//each file can be represented as a node in file table
typedef struct node{
//the size of the file
int size;
//the name of the file
char name[256];
//the timestamp when the file is modified or created
unsigned long int timestamp;
//pointer to build the linked list
struct node *pNext;
//the number of peers that have this file
int peernum;
//for the file table on peers, it is the ip address of the peer
//for the file table on tracker, it records the ip of all peers which has the
//newest edition of the file
unsigned int peerip[MAX_PEER_NUM];
// which type of peer is attached to client c/java
unsigned int peer_type[MAX_PEER_NUM];

}Node,*pNode;

typedef struct filetable{
	int filenum;
	Node* file;
}peer_file_table;

peer_file_table* file_table_create();
peer_file_table* filetable_init(char* directory);
int filetable_addnode(peer_file_table* ptable, int size, char* name, unsigned long int timestamp);
int filetable_modifynode(peer_file_table* ptable, int size, char* name, unsigned long int timestamp);
int filetable_delnode(peer_file_table* ptable, int size, char* name, unsigned long int timestamp);
void filetable_destroy(peer_file_table* table);
void filetable_print(peer_file_table* ptable);
int file_table_deleteip(peer_file_table* ptable, unsigned int ip);
int file_table_addip(peer_file_table* ptable,char* name, unsigned int ip);
unsigned long getmyip();
void append_node(Node *node, char *body);
