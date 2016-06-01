#include "filetable.h"

peer_file_table* file_table_create(){
	peer_file_table* newfiletable=(peer_file_table*)malloc(sizeof(peer_file_table));
	newfiletable->filenum=0;
	newfiletable->file=NULL;
	return newfiletable;
}

peer_file_table* filetable_init(char* directory){
	// dir is the base directory for data sync
	peer_file_table* newfiletable=(peer_file_table*)malloc(sizeof(peer_file_table));
	newfiletable->filenum=0;
	newfiletable->file=NULL;

    struct stat attrib;
    struct dirent *pdirent;
    DIR *pdir;
    int filenum=0;

    pdir = opendir (directory);
    if (pdir == NULL) {
        perror ("Cannot open sync dir: ");
        return NULL;
    }
    while ((pdirent = readdir(pdir)) != NULL) {
		if(pdirent->d_name[0]=='.')
			continue;
		filenum++;
	    if (pdirent->d_type == FILE_TYPE) {
			char path[100];
		        sprintf(path,"%s/%s",directory,pdirent->d_name);
		        stat(path, &attrib);
			//strftime(date, 128, "%y-%m-%d %H:%M", localtime(&(attrib.st_ctime)));
			filetable_addnode(newfiletable, attrib.st_size, pdirent->d_name, attrib.st_mtime);
		}
		else if(pdirent->d_type == FOLDER_TYPE){
			// need to add support for folder

		}
		else{
			printf("Unknown file type\n");
		}
    }
    closedir(pdir);
    return newfiletable;
}
int filetable_addnode(peer_file_table* ptable, int size, char* name, unsigned long int timestamp){
	unsigned int ip=(unsigned int)getmyip();
	ptable->filenum++;
	Node* tmp=ptable->file;

	if(tmp==NULL){
		Node* fnode=(Node*)malloc(sizeof(Node));
		memset(fnode,0,sizeof(Node));
		fnode->size=size;
		memcpy(fnode->name,name,strlen(name));
		//printf("Init node name: %s",fnode->name);
		fnode->timestamp=timestamp;
		fnode->pNext=NULL;
		fnode->peernum=1;
		fnode->peerip[fnode->peernum-1]=ip;
		ptable->file=fnode;
	}
	else{
		Node* tmpnode=(Node*)malloc(sizeof(Node));
		memset(tmpnode,0,sizeof(Node));
		tmpnode->pNext=tmp;
		Node* del=tmpnode;

		while((strcmp(name,tmp->name)>0)&&(tmp->pNext!=NULL)){
			tmpnode=tmpnode->pNext;
			tmp=tmp->pNext;
		}

		Node* newnode=(Node*)malloc(sizeof(Node));
		memset(newnode,0,sizeof(Node));
		newnode->size=size;
		memcpy(newnode->name,name,strlen(name));
		newnode->timestamp=timestamp;
		newnode->peernum=1;
		newnode->peerip[newnode->peernum-1]=ip;
		if(strcmp(name,tmp->name)>0 && tmp->pNext==NULL){
			newnode->pNext=NULL;
			tmp->pNext=newnode;
		}
		else if(tmp==ptable->file){
			newnode->pNext=tmp;
			ptable->file=newnode;
		}
		else{
			newnode->pNext=tmp;
			tmpnode->pNext=newnode;
		}
		free(del);
	}
	return 1;
}

int filetable_modifynode(peer_file_table* ptable, int size, char* name, unsigned long int timestamp){
	Node* tmp=ptable->file;
	if(tmp==NULL){
		return -1;
	}
	else{
		while(tmp!=NULL){
			if(strcmp(tmp->name,name)==0){
				tmp->size=size;
				tmp->timestamp=timestamp;
				return 1;
			}
			tmp=tmp->pNext;
		}
	}
	return -1;
}

int file_table_deleteip(peer_file_table* ptable, unsigned int ip){
	Node* tmp=ptable->file;
	if(tmp==NULL){
		printf("Delete ip error \n");
		return -1;
	}
	while(tmp!=NULL){
		if(tmp->peernum==1){
			if(tmp->peerip[0]==ip){
				// if there exits a node which only this ip has, then delete the node
				filetable_delnode(ptable,tmp->size,tmp->name,tmp->timestamp);
			}
			else{
				printf("Delete ip error \n");
				return -1;
			}
		}
		else{
			int i;
			for(i=0;i<tmp->peernum;i++){
				if(tmp->peerip[i]==ip){
					tmp->peernum--;
					tmp->peerip[i]=tmp->peerip[tmp->peernum];
				}
			}
		}

		tmp=tmp->pNext;
	}
	return 1;
}

int file_table_addip(peer_file_table* ptable,char* name, unsigned int ip){
	Node* tmp=ptable->file;
	if(tmp==NULL){
		printf("Add ip error \n");
		return -1;
	}
	while(tmp!=NULL){
		if(strcmp(tmp->name,name)!=0){
			continue;
		}
		int i;
		for(i=0;i<tmp->peernum;i++){
			if(tmp->peerip[i]==ip){
				printf("file_table_addip: ip already exist\n");
				return -1;
			}
		}
		tmp->peerip[tmp->peernum]=ip;
		return 1;
	}
	printf("Not find requested file \n");
	return -1;
}

int filetable_delnode(peer_file_table* ptable, int size, char* name, unsigned long int timestamp){
	Node* tmp=ptable->file;
	if(tmp==NULL){
		return -1;
	}
	Node* next=tmp->pNext;
	if(next==NULL){
		if(strcmp(tmp->name,name)!=0){
			return -1;
		}
		else{
			ptable->filenum=0;
			ptable->file=NULL;
			return 1;
		}
	}
	else if(strcmp(tmp->name,name)==0){
		ptable->file=tmp->pNext;
		ptable->filenum--;
		free(tmp);
	}
	else{
		while(next!=NULL){
			if(strcmp(next->name,name)==0){
				tmp->pNext=next->pNext;
				free(next);
				ptable->filenum--;
				return 1;
			}
			tmp=tmp->pNext;
			next=next->pNext;
		}
	}
	return -1;	
}

void filetable_destroy(peer_file_table* table){
	printf("In filetable_destroy function\n");
	Node* tmp=table->file;
	while(tmp!=NULL){
		Node* del=tmp;
		tmp=tmp->pNext;
		free(del);
	}
	free(table);
}

void filetable_print(peer_file_table* ptable){
	printf("\n----------------In filetable_print function---------------------\n");
	printf("Dir file number: %d\n",ptable->filenum);
	Node* tmp=ptable->file;
	while(tmp!=NULL){
		printf("Filename:%s size:%d timestamp: %u peernum: %d \n",tmp->name,tmp->size,tmp->timestamp,tmp->peernum);
		int i;
		for(i=0;i<tmp->peernum;i++){
			printf("IP: %u   ",tmp->peerip[i]);
		}
		printf("\n");
		tmp=tmp->pNext;
	}
	printf("Return\n");
}

unsigned long getmyip(){
	//char hostname[100];
//	hostname[99]="\0";
	char* hostname="129.170.212.204";
	//gethostname(hostname,sizeof(hostname)); // get the ip address of local machine
	struct hostent *hostInfo;
  	hostInfo = gethostbyname(hostname);
  	if(!hostInfo) {
  		printf("error in getting host name from string for %s \n", hostname);
  		return -1;
  	}
  	struct sockaddr_in servaddr;
  	memcpy((char *) &servaddr.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
  	return servaddr.sin_addr.s_addr;
//	return inet_addr(ip);
}

void print_ip(int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);        
}
void append_node(Node *node, char *body){
	sprintf(body,"%s%s;",body,node->name);
	sprintf(body,"%s%d;",body,node->size);
	sprintf(body,"%s%d;",body,node->timestamp);
	sprintf(body,"%s%d;",body,node->peernum);
	for(int i = 0 ; i < node->peernum; i++){
		sprintf(body,"%s%d",body,node->peerip[i]);
		if(i < node->peernum - 1){
			sprintf(body,"%s%s",body,"-");
		}
	}
}

int filetable_is_exist(peer_file_table* table,int size, char* name){
	Node* tmp=table->file;	
	int i;
	for(i=0;i<table->filenum;i++){
//		if((strcmp(tmp->name,node->name)==0)&&(tmp->size==node->size)&&(tmp->timestamp==node->timestamp)){
		if(strcmp(tmp->name,name)==0&&tmp->size==size){
			return 1;
		}
		tmp=tmp->pNext;
	}
	return -1;
}
