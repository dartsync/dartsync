#include "downloadtable.h"
download_table *dtable;

download_table* downloadtable_create(){
	dtable=(download_table*)malloc(sizeof(download_table));
	memset(dtable,0,sizeof(download_table));
	dtable->dfilenum=0;
	dtable->file=NULL;
	return dtable;
}

int is_exist(dNode* node){
	dNode* tmp=dtable->file;	
	int i;
	if(tmp==NULL){
		return -1;	
	}
	for(i=0;i<dtable->dfilenum;i++){
//		if((strcmp(tmp->name,node->name)==0)&&(tmp->size==node->size)&&(tmp->timestamp==node->timestamp)){
		if(strcmp(tmp->name,node->name)==0){
			return 1;
		}
		tmp=tmp->pNext;
	}
	return -1;
}

int downloadtable_addnode(dNode* node){
	if(is_exist(node)>0){
		printf("Download table addnode fail: node already exist\n");
		return -1;
	}
	dNode* tmp=dtable->file;
	int i;
	for(i=0;i<dtable->dfilenum-1;i++){
		tmp=tmp->pNext;
	}
	dNode* tmpnode=(dNode*)malloc(sizeof(dNode));
	memset(tmpnode,0,sizeof(dNode));
	memcpy(tmpnode->name,node->name,strlen(node->name));
	tmpnode->size=node->size;
	tmpnode->timestamp=node->timestamp;
	tmpnode->pNext=NULL;
	if(tmp!=NULL){
		tmp->pNext=tmpnode;
	}
	else{
		dtable->file=tmpnode;
	}
	dtable->dfilenum++;
	return 1;
}

int downloadtable_delnode(dNode* node){
	dNode* tmp=dtable->file;
	if(tmp==NULL){
		return -1;
	}
	dNode* next=tmp->pNext;
	if(next==NULL){
		if(strcmp(tmp->name,node->name)!=0){
			return -1;
		}
		else{
			if(tmp->size==node->size&&tmp->timestamp==node->timestamp){
				dtable->dfilenum=0;
				dtable->file=NULL;
				return 1;
			}
			return -1;
		}
	}
	else if(strcmp(tmp->name,node->name)==0&&tmp->size==node->size&&tmp->timestamp==node->timestamp){
		dtable->file=tmp->pNext;
		dtable->dfilenum--;
		return 1;
		free(tmp);
	}
	else{
		while(next!=NULL){
			if(strcmp(next->name,node->name)==0&&next->size==node->size&&next->timestamp==node->timestamp){
				tmp->pNext=next->pNext;
				free(next);
				dtable->dfilenum--;
				return 1;
			}
			tmp=tmp->pNext;
			next=next->pNext;
		}
	}
	return -1;
}

void downloadtable_destroy(){
	dNode* tmp=dtable->file;
	while(tmp!=NULL){
		dNode* del=tmp;
		tmp=tmp->pNext;
		free(del);
	}
	free(dtable);
}

void downloadtable_print(){
	printf("\n----------------In downloadtable_print function---------------------\n");
	dNode* tmp=dtable->file;
	while(tmp!=NULL){
		printf("Filename:%s size:%d timestamp: %u \n",tmp->name,tmp->size,tmp->timestamp);
		int i;
		tmp=tmp->pNext;
	}
	printf("Return\n");

}

int getdnodebyname(dNode* node){
	dNode* tmp=dtable->file;
	int i;
printf("this node's name is %s\n", node->name);	
for(i=0;i<dtable->dfilenum;i++){
		if(strcmp(node->name,tmp->name)==0){
			node->size=tmp->size;
			node->timestamp=tmp->timestamp;
			return 1;
		}		
		tmp=tmp->pNext;
	}
printf("do not have this node in download table\n");
	return -1;
}

int dtable_empty(){
	dNode* tmp=dtable->file;
	if(tmp==NULL){
		return 1;
	}
	return 0;
}

