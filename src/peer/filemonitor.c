#include "filemonitor.h"
#include "downloadtable.h"

char DIR_PATH[128];
int block_updated;

int watchDirectory(peer_file_table* ptable,char* directory){
	block_updated=1;
	struct inotify_event *event;
	memcpy(DIR_PATH,directory,strlen(directory));
	ssize_t numRead;
	char* ept;
	int fd=inotify_init ();
	char buf[EVENT_BUF_LEN]={0};
	if(fd<0){
		printf("filemonitor: Fail to initialize inotify\n");
	}
	int wd=inotify_add_watch(fd, directory, IN_ALL_EVENTS);
	if(wd<0){
		printf("Error when add watch for inotify\n");
	}
	while(1){
		int updated=0;
		numRead=read(fd, buf, EVENT_BUF_LEN);
		if(numRead<=0){
			printf("Eror reading inotify event\n");
		//	exit
		}
		for (ept=buf; ept<buf+numRead;) {
		    event = (struct inotify_event *) ept;
		    // get file name and inotify event type and related event handler
		    char* filename=event->name;
		    if(event->len){
			if(block_updated){
			    	if(event->mask & IN_CREATE){
			    		printf("Inotify event:File %s creat\n",filename);
			    		//updated=fileAdded(ptable,filename);
			    		fileAdded(ptable,filename);
					//updated=1;
			    	}
			    	else if(event->mask & IN_MODIFY){
						printf("Inotify event:File %s modify\n",filename);
						updated=fileModified(ptable,filename);
					//updated=1;
			    	}
			    	else if(event->mask & IN_DELETE){
						printf("Inotify event:File %s delete\n",filename);
						fileDeleted(ptable,filename);
						updated=1;
			    	}
			    	else if(event->mask & IN_MOVED_FROM){
						printf("Inotify event:File %s mode to another directory\n",filename);
						fileDeleted(ptable,filename);
						updated=1;
			    	}
				       	else if(event->mask & IN_MOVED_TO){
						printf("Inotify event:File %s moved in\n",filename);
						updated=fileAdded(ptable,filename);
						//updated=1;
			    	}
			}
	 				// haven't handle delete self mask
		    }		
		    ept += sizeof(struct inotify_event) + event->len;
	    }
	    if(updated){
		send_filetable();
	    }
        //send_filetable();
	}
}
//read configuration file and find the dir to be monitored;
int readConfigFile(char* filename){
    char lineBuf[100];
    char *word1, *word2;
    FILE *fp;
    if ((fp = fopen("./config.dat", "r")) == NULL) {
        printf("Fail to open configuration file\n");
    }
	while (fgets(lineBuf, 100, fp)) {
        	word1 = strtok(lineBuf, " ");
        	word2 = strtok(NULL, " ");
        	if(strcmp(word1,"PATH")==0){
        		strcpy(filename,word2);
        		return 1;
        	}
	}
	return -1;
}

int fileAdded(peer_file_table* ptable,char* filename){
	printf("In fileAdded function\n");
	FileInfo* finfo=getFileInfo(filename);
	printf("File size: %u",finfo->size);
	dNode dfile;
	memcpy(dfile.name,filename,strlen(filename));
	//dfile->size=finfo->size;
	//dfile->timestamp=finfo->timestamp;
	if(is_exist(&dfile)<0){
		downloadtable_print();
		printf("fileadd: file not exist in downloadtable\n");
		if(filetable_is_exist(ptable,finfo->size, filename)<0){
			printf("fileadd: node already have\n");
			filetable_addnode(ptable, finfo->size, filename, finfo->lastModifyTime);
		}
		free(finfo);
		filetable_print(ptable);
		printf("Sending filetable\n");
		//send_filetable();
		return 1;
	}
	free(finfo);
	return -1;
}

int fileModified(peer_file_table* ptable,char* filename ){
	printf("In fileModifieded function\n");
	FileInfo* finfo=getFileInfo(filename);
	dNode dfile;
	memcpy(dfile.name,filename,strlen(filename));
	printf("Timestamp: %u\n",finfo->lastModifyTime);
	if(is_exist(&dfile)<0){
		filetable_modifynode(ptable, finfo->size, filename, finfo->lastModifyTime);
		free(finfo);
		filetable_print(ptable);
		//send_filetable();
		return 1;
	}
	else{
		printf("remove node from download table,add to filetable\n");
		dNode dfile;
		memset(&dfile,0,sizeof(dNode));
		memcpy(dfile.name,filename,strlen(filename));
		getdnodebyname(&dfile);
		if(dfile.size!=finfo->size){
			return -1;		
		}
		downloadtable_delnode(&dfile);
		filetable_addnode(ptable, dfile.size, dfile.name, dfile.timestamp);

	}
	free(finfo);
	return -1;
}
int fileDeleted(peer_file_table* ptable,char* filename){
	printf("In fileDeleted function\n");
	FileInfo* finfo=getFileInfo(filename);
	filetable_delnode(ptable, finfo->size, filename, finfo->lastModifyTime);
	free(finfo);
	filetable_print(ptable);
	//send_filetable();
	return 1;
}
int getAllFilesInfo(){

}
FileInfo* getFileInfo(char* filename){
	char path[100];
	sprintf(path,"%s/%s",DIR_PATH,filename);
	//sleep(1);
	FileInfo* file=(FileInfo*)malloc(sizeof(FileInfo));
	struct stat attrib;
	stat(path, &attrib);
	file->size=attrib.st_size;
	file->lastModifyTime=attrib.st_mtime;
	return file;
}
void blockUpdate(){
	block_updated=0;
}

void unblockUpdate(){
	block_updated=1;
}

int freeAll(){

}
int blockFileAddListenning(){


}
int unblockFileAddListenning(){

}
int blockFileWriteListenning(){

}
int unblockFileWriteListenning(){

}
int blockFileDeleteListenning(){

}
int unblockFileDeleteListenning(){

}
