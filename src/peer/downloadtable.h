#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <arpa/inet.h>
#include "../utils/constants.h"

typedef struct dnode{
//the size of the file
int size;
//the name of the file
char name[256];
//the timestamp when the file is modified or created
unsigned long int timestamp;
//pointer to build the linked list
struct dnode *pNext;
}dNode,*pdNode;

typedef struct dfiletable{
	int dfilenum;
	dNode* file;
}download_table;

download_table* downloadtable_create();
int is_exist(dNode* node);
int downloadtable_addnode(dNode* node);
int downloadtable_delnode(dNode* node);
void downloadtable_destroy();
void downloadtable_print();
int dtable_empty();
int getdnodebyname(dNode* node);
