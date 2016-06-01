#include "filemonitor.h"
#include "downloadtable.h"
#include <pthread.h>
#include <sys/time.h>

char DIR_PATH[128];
int block_updated;
dirlist* dlist;
int timeflag;
int updated;
struct timeval currentTime;
unsigned long int mtime;
pthread_mutex_t *update_mutex;

void* update_filetable(){
    while(1){
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        unsigned long int curtime = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
        //printf("mtime: %u updated: %d \n",mtime,updated);
        pthread_mutex_lock(update_mutex);
        if(curtime - mtime > 5 * 1000000 && updated>0 && dtable_empty()){
            send_filetable();
            updated=0;
        }
        pthread_mutex_unlock(update_mutex);
        sleep(1);
    }
}

int watchDirectory(peer_file_table* ptable,char* directory){
    dlist=NULL;
    updated=0;
    timeflag=0;
    block_updated=1;
    update_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(update_mutex, NULL);

    struct inotify_event *event;
    memcpy(DIR_PATH,directory,strlen(directory));
    ssize_t numRead;
    char* ept;
    int fd=inotify_init ();
    char buf[EVENT_BUF_LEN]={0};
    int wd=inotify_add_watch(fd, directory, IN_ALL_EVENTS);
        if(wd<0){
        printf("Error when add watch for inotify\n");
        }
    char tmpname[256];
    char tmppath[256];
    if(fd<0){
        printf("filemonitor: Fail to initialize inotify\n");
    }
    addwatch(fd,NULL);

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    mtime = currentTime.tv_sec * 1000000 + currentTime.tv_usec;

    pthread_t ftable_update_thread;
    pthread_create(&ftable_update_thread,NULL,update_filetable,NULL);
    
     while(1){
         numRead=read(fd, buf, EVENT_BUF_LEN);
         struct timeval cTime;
         gettimeofday(&cTime, NULL);
         mtime = cTime.tv_sec * 1000000 + cTime.tv_usec;
         pthread_mutex_lock(update_mutex);

         if(numRead<=0){
             printf("Eror reading inotify event\n");
         //  exit
         }
         for (ept=buf; ept<buf+numRead;) {
             memset(tmpname,0,256);
             memset(tmppath,0,256);
             int isdir;
             event = (struct inotify_event *) ept;
             // get file name and inotify event type and related event handler
             char* filename=event->name;
             if(getnodefromwd(tmppath,event->wd)>0){
                 sprintf(tmpname,"%s%s",tmppath,filename);
             }
             else{
                 memcpy(tmpname,filename,strlen(filename));
             }
             if(event->mask & IN_ISDIR){
                isdir=1;
                sprintf(tmpname,"%s/",tmpname);
             }
             else{
                isdir=0;
             }
             if(event->len){
             if(block_updated){
                     if(event->mask & IN_CREATE){
                                     
                         //updated=fileAdded(ptable,filename);
                     if(isdir){
                         printf("Inotify event:dir %s creat\n",tmpname);
			 lock_file_table();
                         updated=fileAdded(ptable,tmpname);
			 unlock_file_table();
                         addwatch(fd,tmpname);
                     }
                     else{
                         if(!ignore_tmp(filename)){
                             printf("Inotify event:File %s creat\n",tmpname);
			     lock_file_table();
                             updated=fileAdded(ptable,tmpname);
			     unlock_file_table();
                             }else{
                             printf("tmp file, ignore it\n");
                         }
                         //updated=1;
                     }
                     }
                     else if(event->mask & IN_MODIFY){
                     //updated=1;
                     if(isdir){
                         printf("Inotify event:dir %s modify\n",tmpname);    
                         //addwatch(fd,dlist,tmpname);                   
                     }
                     else{
                         if(!ignore_tmp(filename)){
                             printf("Inotify event:File %s modify\n",tmpname);
			     lock_file_table();
                             updated=fileModified(ptable,tmpname);
			     unlock_file_table();
                             //updated=1;
                             }else{
                             printf("tmp file, ignore it\n");
                         }
                     }


                     }
                     else if(event->mask & IN_DELETE){
                     if(isdir){
                         printf("Inotify event:dir %s delete\n",tmpname);
                         //addwatch(fd,dlist,tmpname);
			 lock_file_table();
                         fileDeleted(ptable,tmpname);
			 unlock_file_table();
                         updated=1;
                         dlist_delnode(event->wd);
                                             
                     }
                     else{
                         if(!ignore_tmp(filename)){
                             printf("Inotify event:File %s delete\n",tmpname);
			     lock_file_table();
                             fileDeleted(ptable,tmpname);
			     unlock_file_table();
                             updated=1;
                             //updated=1;
                             }else{
                             printf("tmp file, ignore it\n");
                         }
                     }

                     }
                     else if(event->mask & IN_MOVED_FROM){
                     if(isdir){
                         printf("Inotify event:DIR %s move to another directory\n",tmpname);
                         //addwatch(fd,dlist,tmpname);
                         lock_file_table();
                         fileDeleted(ptable,tmpname);
			 unlock_file_table();
                         updated=1;
                         dlist_delnode(event->wd);                   
                     }
                     else{
                         if(!ignore_tmp(filename)){
                             printf("Inotify event:File %s move to another directory\n",tmpname);
                             lock_file_table();
                             fileDeleted(ptable,tmpname);
			     unlock_file_table();
                             updated=1;
                             //updated=1;
                             }else{
                             printf("tmp file, ignore it\n");
                         }
                     }
                     }
                 else if(event->mask & IN_MOVED_TO){
                     if(isdir){
                         printf("Inotify event:DIR %s move moved in\n",tmpname);
			 lock_file_table();
                         updated=fileAdded(ptable,tmpname);
			 unlock_file_table();
                         addwatch(fd,tmpname);                   
                     }
                     else{
                         if(!ignore_tmp(filename)){
                             printf("Inotify event:File %s moved in\n",tmpname);
                             lock_file_table();
                             updated=fileAdded(ptable,tmpname);
			     unlock_file_table();
                             //updated=1;
                             }else{
                             printf("tmp file, ignore it\n");
                         }
                     }
                     
                     }
             }
                     // haven't handle delete self mask
             }
             ept += sizeof(struct inotify_event) + event->len;
         }
         pthread_mutex_unlock(update_mutex);
/*         if(updated){
             if(dtable_empty()){
                 send_filetable();  
             }   
         }*/
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
memset(&dfile, 0, sizeof(dNode));   
memcpy(dfile.name,filename,strlen(filename));
    //dfile->size=finfo->size;
    //dfile->timestamp=finfo->timestamp;
    if(is_exist(&dfile)<0){
        downloadtable_print();
        printf("fileadd: file not exist in downloadtable\n");
        if(filetable_is_exist(ptable,finfo->size, filename)<0){
            printf("fileadd: node not already have\n");
            filetable_addnode(ptable, finfo->size, filename, finfo->lastModifyTime);
        }
        free(finfo);
        filetable_print(ptable);
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
    memset(&dfile, 0, sizeof(dNode));
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
          printf("dfile size: %d, finfo size: %d add fail, hehe\n", dfile.size, finfo->size);   
          return -1;        
        }
        downloadtable_delnode(&dfile);
        filetable_addnode(ptable, dfile.size, dfile.name, dfile.timestamp);
        downloadtable_print();
        filetable_print(ptable);
    }
    free(finfo);
    return 1;
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
    memset(path,0,100);
    sprintf(path,"%s/%s",DIR_PATH,filename);
    printf("get file Info: %s",path);
    select(0,0,0,0,&(struct timeval){.tv_usec =  1000000 * 0.1});
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

int ignore_tmp(char *filename){
    int len = strlen(filename);
    printf("file name: %s", filename);
    if(filename[0] == '.' || filename[len - 1] == '~')
        return 1;
    return 0;
}

int addwatch(int fd,char* directory){
    char tmppath[256];
    char tmpdir[256];
    memset(tmpdir,0,256);
    memset(tmppath,0,256);
    if(directory!=NULL){
        printf("add dir: %s\n",directory);
        sprintf(tmpdir,"%s/%s",DIR_PATH,directory);
        int wd=inotify_add_watch(fd, tmpdir, IN_ALL_EVENTS);
            if(wd<0){
        printf("Error when add watch for inotify\n");
            }
        printf("add wd: %d\n",wd);
            dlist_addnode(directory,wd);
        //dlistprint(dlist);
    }
    struct stat attrib;
    struct dirent *pdirent;
    DIR *pdir;
    int filenum=0;
    if(directory!=NULL){
        pdir = opendir (tmpdir);
    }
    else{
    pdir = opendir (DIR_PATH);
    }
    if (pdir == NULL) {
        perror ("Cannot open sync dir: ");
        return NULL;
    }
    while ((pdirent = readdir(pdir)) != NULL) {
    if(pdirent->d_name[0]=='.')
        continue;
    if (pdirent->d_type == FILE_TYPE) {
        continue;
    }
        else if(pdirent->d_type == FOLDER_TYPE){
        memset(tmppath,0,256);
        if(directory!=NULL){
            sprintf(tmppath,"%s/%s",directory,pdirent->d_name);
            addwatch(fd,tmppath);
            continue;
        }
        addwatch(fd,pdirent->d_name);       
    }
    else{
        printf("Unknown file type\n");
    }
    }
    closedir(pdir);
}

int dlist_addnode(char* directory,int wd){
    dirlist* new=(dirlist*)malloc(sizeof(dirlist));
    memset(new,0,sizeof(dirlist));
    memcpy(new->dirpath,directory,strlen(directory));
    new->wd=wd;
    new->pNext=NULL;
    if(dlist==NULL){
        dlist=new;
        printf("first node: %s,%d",dlist->dirpath,dlist->wd);
        return 1;
    }
    dirlist* tmp=dlist;
    while(tmp->pNext!=NULL){
        tmp=tmp->pNext;
    }
    tmp->pNext=new;
    return 1;   
}

int getnodefromwd(char* path,int wd){
    dirlist* tmp=dlist;
    while(tmp!=NULL){
        if(tmp->wd==wd){
            if(strcmp(tmp->dirpath,DIR_PATH)==0){
                return -1;
            }
            memcpy(path,tmp->dirpath,strlen(tmp->dirpath));
            return 1;
        }
        tmp=tmp->pNext;
    }
    return -1;
}

int dlist_delnode(int wd){
    dirlist* tmp=dlist;
    if(tmp==NULL){
        return -1;  
    }
    if(tmp->pNext==NULL&&tmp->wd==wd){
        dlist=NULL;
        return 1;
    }
    dirlist* next=tmp->pNext;
    while(next!=NULL){
        if(next->wd==wd){
            tmp->pNext=next->pNext;
            free(next);
            return 1;
        }
        next=next->pNext;
        tmp=tmp->pNext;
    }
    return -1;
}

void dlistprint(){
    dirlist* tmp=dlist;
    while(tmp!=NULL){
        printf("dir name: %s, wd: %d \n",tmp->dirpath,tmp->wd);
        tmp=tmp->pNext;
    }
    return;
}
