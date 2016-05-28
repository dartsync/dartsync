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
	unsigned int ip=getmyip();
	ptable->filenum++;
	Node* tmp=ptable->file;

	if(tmp==NULL){
		Node* fnode=(Node*)malloc(sizeof(Node));
		fnode->size=size;
		memcpy(fnode->name,name,strlen(name));
		fnode->timestamp=timestamp;
		fnode->pNext=NULL;
		fnode->peernum=1;
		fnode->peerip[fnode->peernum-1]=ip;
		ptable->file=fnode;
	}
	else{
		Node* tmpnode=(Node*)malloc(sizeof(Node));
		tmpnode->pNext=tmp;
		Node* del=tmpnode;

		while((strcmp(name,tmp->name)>0)&&(tmp->pNext!=NULL)){
			tmpnode=tmpnode->pNext;
			tmp=tmp->pNext;
		}

		Node* newnode=(Node*)malloc(sizeof(Node));
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
			if(strcmp(tmp->name,name)){
				tmp->size=size;
				tmp->timestamp=timestamp;
				return 1;
			}
			tmp=tmp->pNext;
		}
	}
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
			ptable->file=NULL;
			return 1;
		}
	}
	else if(strcmp(tmp->name,name)==0){
		ptable->file=tmp->pNext;
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
	printf("In filetable_print function\n");
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

unsigned int getmyip(){
  char myhostName[50];
  struct hostent *hostInfo;
  if (gethostname(myhostName, sizeof(myhostName)) == 0) {
  	hostInfo = gethostbyname(myhostName);
  }
  else{
  	printf("In function getip: gethostname error\n");
  }
  struct in_addr* addr=(struct in_addr *)hostInfo->h_addr_list[0];
  char* ip = inet_ntoa (*addr);
  return inet_addr(ip);
}
