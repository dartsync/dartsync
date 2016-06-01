/*
 * peer.c
 *
 *  Created on: May 23, 2016
 *      Author: Vishal Gaurav
 */
#define TRACKER_ADDRESS_LENGTH 100

char tracker_host_name[100];

#include "peer.h"
#include "../utils/seg.h"
#include "../utils/utils.c"
#include "../peer/p2p.h"
#include "../network/network_utils.h"
#include "../utils/constants.h"
#include "downloadtable.h"

//#include "filetable.h"

char dirname[128];
int heartbeat_interval;
int piece_len;
int network_conn=-1;
peer_file_table *filetable;
//download_table *downloadtable;

//peer_peer_table *peertable;

pthread_mutex_t *file_tb_mutex;
pthread_mutex_t *download_tb_mutex;

void lock_file_table(){
	pthread_mutex_lock(file_tb_mutex);
}

void unlock_file_table(){
	pthread_mutex_unlock(file_tb_mutex);
}

void lock_download_table(){
	pthread_mutex_lock(download_tb_mutex);
}

void unlock_download_table(){
	pthread_mutex_unlock(download_tb_mutex);
}


int connectToTracker(){
  int out_conn;
  struct sockaddr_in servaddr;
  servaddr.sin_family =AF_INET;
  //servaddr.sin_addr.s_addr= inet_addr("129.170.214.100");
  servaddr.sin_addr.s_addr= inet_addr("129.170.212.139");
  servaddr.sin_port = htons(TRACKER_PORT);
  out_conn = socket(AF_INET,SOCK_STREAM,6);

  if(out_conn<0) {
	printf("Create socket error\n");
	return -1;
  }
  if(connect(out_conn, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
  	perror("connect to trackerfail: ");
	printf("\n");
	return -1;
  }
  printf("Send register pkg \n");
  if(send_registion(out_conn)<0){
  	printf("Send Register seg fail\n");
  }
  ttp_seg_t recvseg;
  if(peer_recvseg(out_conn,&recvseg)<0){
  	printf("Receive filetable error\n");
  }

  heartbeat_interval=recvseg.interval;
  piece_len=recvseg.piece_len;
  printf("Receive intercal: %d piece_len: %d \n",heartbeat_interval,piece_len);

  return out_conn;
}

int send_registion(int out_conn){
  int ret;
  ptt_seg_t* sendseg=(ptt_seg_t*)malloc(sizeof(ptt_seg_t));
  memset(sendseg,0,sizeof(ptt_seg_t));
  sendseg->protocol_len=sizeof(ptt_seg_t);
//  sendseg->protocol_name=;
  sendseg->type=REGISTER;
  sendseg->peer_ip=getmyip();
//  sendseg->port=;
//  sendseg->
  sendseg->file_table_size=0;
//  sendseg->file_table=NULL;
  ret=peer_sendseg(out_conn, sendseg);
  if(ret<0){
  	return -1;
  }
  return 1;
}

int send_filetable(){
	if(network_conn<0){
		return -1;
	}
	ptt_seg_t* sendseg=(ptt_seg_t*)malloc(sizeof(ptt_seg_t)); 
	sendseg->protocol_len=sizeof(ptt_seg_t);
//	sendseg->protocol_name="PI_NET";
	sendseg->type=FILE_UPDATE;
	sendseg->peer_ip=getmyip();
//	sendseg->port=;
	
	//peer_file_table *sendftable;
	//sendftable=filetable_init(dirname);

	//sendftable=filetable;

	sendseg->file_table_size=filetable->filenum;
	int fnum=filetable->filenum;
	Node* sendfnode=filetable->file;
	printf("Sending filetable: filenum: %d\n",fnum);
	int i=0;
	for(i=0;i<fnum;i++){
		memcpy(sendseg->file_table+i,sendfnode,sizeof(Node));
		sendfnode=sendfnode->pNext;
	}
	int retnum;
	retnum=peer_sendseg(network_conn, sendseg);
	//filetable_destroy(sendftable);
	return retnum;
}

int peer_update_filetable(Node* recv,int recvnum){

	//peer_file_table *curftable;
	//curftable=filetable_init(dirname);
	lock_file_table();
	int curnum=filetable->filenum;
	int num=0;
	Node* recvfpt=recv;
	Node* curfpt=filetable->file	;
	printf("in peer_update_filetable: block update\n");
	//blockUpdate();
	Node* tmp=recv;
	int n;
	for(n=0;n<recvnum;n++){
		printf("name: %s\n",tmp->name);
		tmp++;
	}
	int i=0;
	// file mutex
	while(num<recvnum&&curfpt!=NULL){
		char name[256];
		memset(name,0,256);
		Node* recvnode=recv+num;
		//printf("recv: %s,cur: %s",recvnode->name,curfpt->name);
		if(strcmp(recvnode->name,curfpt->name)<0){
			printf("%d\n",strlen(recvnode->name));
			if(recvnode->name[strlen(recvnode->name)-1]=='/'){
				char newdirname[128];
				printf("Find new DIR: %s\n",recvnode->name);
				get_dir_frompath(name,recvnode->name);
				printf("Dir name: %s\n",name);
				struct stat st = {0};
				sprintf(newdirname,"%s/%s",dirname,recvnode->name);
				if (stat(newdirname, &st) == -1) {
				    mkdir(newdirname, 0700);
				}
				num++;
			}
			else{
				printf("Find new file: %s\n",recvnode->name);
				int i;
				printf("IPnum: %s\n",recvnode->name);
				download_file(recvnode);
				num++;
			}
		}
		else if(strcmp(recvnode->name,curfpt->name)>0){
			char delFilename[128];
			memset(delFilename,0,128);
			if(curfpt->name[strlen(curfpt->name)-1]!='/'){
				printf("Delete file: %s\n",curfpt->name);
				sprintf(delFilename,"%s/%s",dirname,curfpt->name);
				remove(delFilename);
				curfpt=curfpt->pNext;
			}
			else{
				char tmppath[256];
				char tmppath2[256];
				memset(tmppath,0,256);
				memset(tmppath2,0,256);
				int i=1;
				int j;
				Node* tmp=curfpt->pNext;
				for(j=0;j<strlen(curfpt->name);j++){
					if(curfpt->name[j]=='/'){
						tmppath2[j]='/';
						tmppath2[j+1]='\0';
						break;
					}
					tmppath2[j]=curfpt->name[j];
				}
				for(j=0;j<strlen(tmp->name);j++){
					if(tmp->name[j]=='/'){
						tmppath[j]='/';
						tmppath[j+1]='\0';
						break;
					}
					tmppath[j]=tmp->name[j];
				}
				while(tmp!=NULL&&strcmp(tmppath2,tmppath)==0){
					memset(tmppath,0,256);
					for(j=0;j<strlen(tmp->name);j++){
						if(tmp->name[j]=='/'){
							tmppath[j]='/';
							tmppath[j+1]='\0';
							break;
						}
						tmppath[j]=tmp->name[j];
					}
					tmp=tmp->pNext;
					i++;
				}
				int k=i;
				printf("na: %s",tmppath);
				printf("i:%d",i);
				while(i>0){
					tmp=curfpt;
					int n;
					for(n=1;n<i;n++){
					    tmp=tmp->pNext;
					}
					memset(delFilename,0,128);
					printf("Delete dir: %s\n",tmp->name);		
					sprintf(delFilename,"%s/%s",dirname,tmp->name);
					remove(delFilename);
					i--;
				}
				int m;
				for(m=0;m<k;m++){
					curfpt=curfpt->pNext;
				}
			}
		}
		else{
			printf("%s\n",recvnode->name);
				if(recvnode->size!=curfpt->size||recvnode->timestamp!=curfpt->timestamp){
					// tracker and peer both have this file, but peer side file need to be updated
					if(recvnode->name[strlen(recvnode->name)-1]!='/'){
						printf("Find motified : %s\n",recvnode->name);
						char delFilename[128];
						sprintf(delFilename,"%s/%s",dirname,recvnode->name);
						remove(delFilename);
						download_file(recvnode);
						num++;
						curfpt=curfpt->pNext;
					}
						num++;
						curfpt=curfpt->pNext;
				}
		else{
				printf("In here\n");
				num++;
				curfpt=curfpt->pNext;
			}
		}

	}
	printf("here\n");
	while(num<recvnum){
		    char name[256];
			memset(name,0,256);
			Node* recvnode=recv+num;
			if(recvnode->name[strlen(recvnode->name)-1]=='/'){
				char newdirname[128];
				memset(newdirname,0,128);
				printf("Find new DIR: %s\n",recvnode->name);
				get_dir_frompath(name,recvnode->name);
				printf("Dir name: %s\n",name);
				struct stat st = {0};
				sprintf(newdirname,"%s/%s",dirname,recvnode->name);
				if (stat(newdirname, &st) == -1) {
				    mkdir(newdirname, 0700);
				}
				num++;
			}
			else{
				printf("Find new file: %s\n",recvnode->name);
				printf("Target file IP : %u\n",recvnode->peerip[0]);
				download_file(recvnode);
				num++;
			}
	}
	while(curfpt!=NULL){
		char delFilename[128];
		memset(delFilename,0,128);
			if(curfpt->name[strlen(curfpt->name)-1]!='/'){
				printf("Delete file: %s\n",curfpt->name);
				sprintf(delFilename,"%s/%s",dirname,curfpt->name);
				remove(delFilename);
				curfpt=curfpt->pNext;
			}
			else{
				char tmppath[256];
				char tmppath2[256];
				memset(tmppath,0,256);
				memset(tmppath2,0,256);
				int i=1;
				int j;
				Node* tmp=curfpt->pNext;
				for(j=0;j<strlen(curfpt->name);j++){
					if(curfpt->name[j]=='/'){
						tmppath2[j]='/';
						tmppath2[j+1]='\0';
						break;
					}
					tmppath2[j]=curfpt->name[j];
				}
				for(j=0;j<strlen(tmp->name);j++){
					if(tmp->name[j]=='/'){
						tmppath[j]='/';
						tmppath[j+1]='\0';
						break;
					}
					tmppath[j]=tmp->name[j];
				}
				while(tmp!=NULL&&strcmp(tmppath2,tmppath)==0){
					memset(tmppath,0,256);
					for(j=0;j<strlen(tmp->name);j++){
						if(tmp->name[j]=='/'){
							tmppath[j]='/';
							tmppath[j+1]='\0';
							break;
						}
						tmppath[j]=tmp->name[j];
					}
					tmp=tmp->pNext;
					i++;
				}
				int k=i;
				printf("na: %s",tmppath);
				printf("i:%d",i);
				while(i>0){
					tmp=curfpt;
					int n;
					for(n=1;n<i;n++){
					    tmp=tmp->pNext;
					}
					memset(delFilename,0,128);
					printf("Delete dir: %s\n",tmp->name);		
					sprintf(delFilename,"%s/%s",dirname,tmp->name);
					remove(delFilename);
					i--;
				}
				int m;
				for(m=0;m<k;m++){
					curfpt=curfpt->pNext;
				}
			}
		//fileDeleted(filetable, curfpt->name);
	}
	printf("unblock update\n");
	unlock_file_table();
	//unblockUpdate();
	//filetable_destroy(curftable);
}

int get_dir_frompath(char* name,char* path){
	memcpy(name,path,strlen(path));
	name[strlen(name)-1]='\0';
	printf("path after remove /%s\n",name);
	int l = 0;
	char* tmp=strstr(name,"/");
	while(tmp!=NULL){
		l=strlen(tmp)+1;
    	name=&name[strlen(name)-l+2];
		tmp=strstr(name,"/");
	}
	return 1;
}

void peerlistening(){
  printf("Now in peerlistening thread \n");
  int transport_conn;
  int listenfd,connfd;
  struct sockaddr_in servaddr;
  struct sockaddr_in clitaddr;
  servaddr.sin_family=AF_INET;
  servaddr.sin_addr.s_addr= htonl(INADDR_ANY);
  servaddr.sin_port = htons(COMMUNICATION_PORT);

  listenfd = socket(AF_INET,SOCK_STREAM,0);  
  if(listenfd<0) {
	printf("Create socket error\n");
	return;
  }
  if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1){
  	printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
  	close(listenfd);
  	return;
  }
  if(listen(listenfd,1) == -1){
  	printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
  	return;
  }
  while(1){
	  int len=sizeof(clitaddr);
  	  printf("waiting for request from other peers \n");
	  if( (transport_conn=accept(listenfd, (struct sockaddr*)&clitaddr, &len)) == -1){
		printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
				// then?
		break;
	  }  
//	  pthread_t peer_upload_thread;
//	  pthread_create(&peer_upload_thread,NULL,peerupload,(void*)&transport_conn);
  }
  printf("Peer listening thread exit!\n");
  pthread_exit(NULL);
}
/**
 * typedef struct p2p_seg{
	char file_name[MAX_FILE_NAME_LEN];
	int piece_len;
	int start_idx;
	int end_idx;
} peer2peer_seg;

typedef struct download_seg{
	int socket;
	peer2peer_seg seg;
} peerdownload_seg;
 */
void append_down_header(peerdownload_seg *node, char *body){
	sprintf(body,"%s,",node->seg.file_name);
	sprintf(body,"%s%d,",body,node->seg.start_idx);
	sprintf(body,"%s%d\n",body,node->seg.piece_len);
}
void *download_chunk_diff(void *download_info){
	printf("downloading chunk server port = %d\n",PEER_DOWNLOAD_PORT_DIFFERENT);
	fflush(stdout);
	char header[256];
	bzero(header,256);
	peerdownload_seg *download_seg = (peerdownload_seg *) download_info;
	download_seg->socket = get_client_socket_fd_ip_diff(download_seg->peer_ip,PEER_DOWNLOAD_PORT_DIFFERENT);
	printf("connection socket = %d \n", download_seg->socket);
	if(download_seg->socket < 0){
		printf("socket error for chunk download from java ip = %d & port = %d \n", download_seg->peer_ip, PEER_DOWNLOAD_PORT_DIFFERENT );
		pthread_exit(NULL);
		return NULL;
	}
	append_down_header(download_seg,header);
	printf("java download header = %s \n",header);
	if(send(download_seg->socket,header,sizeof(header),0)<0){
		printf("unable to send download header to java socket %d \n",download_seg->socket);
		close(download_seg->socket);
		pthread_exit(NULL);
		return NULL;
	}
	int received_size = 0 ;
	int total = 0 ;
	char buffer[FILE_BUFFER_SIZE];
	printf("header sent wating for download :- %s \n", download_seg->seg.file_name);
	while((received_size = recv(download_seg->socket,buffer,FILE_BUFFER_SIZE,0)) > 0){
		total += received_size;
		printf("writting to temp file  = %d bytes buffer = %s \n", received_size, buffer);
		if(fwrite(buffer,received_size,1,download_seg->tempFile) < 0){
			printf("file write error in temp file \n");
			printf("unable to send segment to socket %d \n",download_seg->socket);
			close(download_seg->socket);
			pthread_exit(NULL);
			return NULL;
		}
	}
	printf("chunk download for for %s total = %d\n" , download_seg->seg.file_name, total);
	download_seg->isSuccess = TRUE;
	close(download_seg->socket);
	fflush(stdout);
	pthread_exit(NULL);
	return NULL;
}
void *download_chunk(void *download_info){
	printf("downloading chunk server port = %d\n",PEER_DOWNLOAD_PORT);
	fflush(stdout);
	peerdownload_seg *download_seg = (peerdownload_seg *) download_info;
	download_seg->socket = get_client_socket_fd_ip(download_seg->peer_ip,PEER_DOWNLOAD_PORT);
	printf("connection socket = %d \n", download_seg->socket);
	if(download_seg->socket < 0){
		printf("unable to get socket for chunk download \n");
		pthread_exit(NULL);
		return NULL;
	}
	if(send_p2p_seg(download_seg->socket,&download_seg->seg) < 0){
		printf("unable to send segment to socket %d \n",download_seg->socket);
		close(download_seg->socket);
		pthread_exit(NULL);
		return NULL;
	}
	int received_size = 0 ;
	char buffer[FILE_BUFFER_SIZE];
	while((received_size = recv(download_seg->socket,buffer,FILE_BUFFER_SIZE,0)) > 0){
		printf("writting to temp file  = %d bytes buffer = %s \n", received_size, buffer);
		if(fwrite(buffer,received_size,1,download_seg->tempFile) < 0){
			printf("file write error in temp file \n");
			printf("unable to send segment to socket %d \n",download_seg->socket);
			close(download_seg->socket);
			pthread_exit(NULL);
			return NULL;
		}
	}
	printf("chunk download success \n");
	download_seg->isSuccess = TRUE;
	close(download_seg->socket);
	fflush(stdout);
	pthread_exit(NULL);
	return NULL;
}
void merge_temp_file(FILE *main_file, FILE *temp_file){
	printf("merging temp file \n");
	rewind(temp_file);
	char buffer[FILE_BUFFER_SIZE];
	while(!feof(temp_file)){
		int received_size = fread(buffer, sizeof(char), FILE_BUFFER_SIZE, temp_file);
		//printf("read %d bytes from temp file \n", received_size);
		if(received_size > 0){
			if(fwrite(buffer,received_size,1,main_file) < 0 ){
				printf("file write error in main file \n");
			}
		}
	}
	printf("merging temp file finished\n");
}
void *file_download_handler(void *file_info){
	//blockUpdate();
	char filename[256];
	Node* file_node = (Node *) file_info;
	sprintf(filename,"%s/%s",dirname,file_node->name);

	if(file_node){
		int chunks = (file_node->size > file_node->peernum) ? file_node->peernum : file_node->size ;
		//chunks = 1;
		if(chunks > 0){
			printf("chunks available = %d \n",chunks);
			temp_download_t multi_threads[chunks] ;
			int chunk_size = file_node->size / chunks;
			for(int i = 0 ; i < chunks ; i++){
				// start downloading chunks
				peerdownload_seg *download_seg = (peerdownload_seg *)malloc(sizeof(peerdownload_seg));
				bzero(download_seg,sizeof(peerdownload_seg));
				download_seg->seg.start_idx = i * chunk_size;
				download_seg->seg.piece_len =  (i < chunks -1) ? chunk_size :  (file_node->size - (i * chunk_size));
				memcpy(download_seg->seg.file_name,file_node->name,MAX_FILE_NAME_LEN);
				download_seg->peer_ip = file_node->peerip[i];
				download_seg->tempFile = tmpfile(); // assuming tempfile is unique and will not stored in the file monitor directory
				download_seg->isSuccess = FALSE;
				multi_threads[i].download_seg = download_seg;
				if(file_node->peer_type[i] == PEER_TYPE_DIFFERENT){
					printf("downloading chunk from java = %d \n",i);
					pthread_create(&(multi_threads[i].thread), NULL, download_chunk_diff,(void *)download_seg);
				}else{
					printf("downloading chunk from C = %d \n",i);
					pthread_create(&(multi_threads[i].thread), NULL, download_chunk,(void *)download_seg);
				}
			}
			for(int i = 0 ; i < chunks ; i++){
				pthread_join((multi_threads[i].thread),NULL);
				fflush(stdout);
				printf("chunk %d finished \n", i);
				FILE *main_file = fopen(filename,"a");
					// merge the downloaded chunks
				merge_temp_file(main_file,multi_threads[i].download_seg->tempFile);
				fclose(multi_threads[i].download_seg->tempFile);
				fclose(main_file);
			}
			printf("merge file success \n");
		}else if(file_node->size == 0 && file_node->peernum != 0){
			//download it from a peer
		}else{
			//tell tracker that none have the file
			// will not use I think
		}
	}
	/*printf("remove node from download table,add to filetable\n");
	dNode dfile;
	memset(&dfile,0,sizeof(dNode));
	memcpy(dfile.name,file_node->name,strlen(file_node->name));
	dfile.size=file_node->size;
	dfile.timestamp=file_node->timestamp;
	downloadtable_delnode(&dfile);
	filetable_addnode(filetable, dfile.size, dfile.name, dfile.timestamp);*/
	//unblockUpdate();
	//send_filetable();
	pthread_exit(NULL);
	return NULL ;
}

int download_file(Node* file_node){
	printf(" in download file \n");
	downloadtable_print();
	dNode dfile;
	memset(&dfile,0,sizeof(dNode));
	memcpy(dfile.name,file_node->name,strlen(file_node->name));
	dfile.size=file_node->size;
	dfile.timestamp=file_node->timestamp;
	downloadtable_addnode(&dfile);
	downloadtable_print();
	if(file_node){
		pthread_t peer_download_thread;
		pthread_create(&peer_download_thread,NULL,file_download_handler,(void*)file_node);
	}
	return 0;
}

void* filemonitor(void* arg){
	char* dir=(char*)arg;
	watchDirectory(filetable,dir);
	return NULL;
}

void* heartbeat(){
	printf("Now in heartbeat function\n");
	while(1){
		//sleep(HEARTBEAT_INTERVAL);
		select(0,0,0,0,&(struct timeval){.tv_usec = heartbeat_interval * 1000000 * 0.9});		
		ptt_seg_t* sendseg=(ptt_seg_t*)malloc(sizeof(ptt_seg_t));
		memset(sendseg,0,sizeof(ptt_seg_t));
//		sendseg->protocol_len=;
//		sendseg->protocol_name=;
		sendseg->type=KEEP_ALIVE;
		sendseg->peer_ip=getmyip();
//		sendseg->port=;
//		sendseg->file_table_size=;
//		sendseg->file_table=;
//		segsize=;
		peer_sendseg(network_conn, sendseg);
	}

}

void peer_stop(){
	close(network_conn);
	filetable_destroy(filetable);
	downloadtable_destroy();
	printf("Peer stop!\n");
	exit(0);
}

void *file_upload_request_handler(){
	int server_sock_fd = get_server_socket_fd(PEER_DOWNLOAD_PORT,MAX_PEERS_NUM);
	if(server_sock_fd < 0 ){
		printf("unable to create listening socket for upload handler\n");
		pthread_exit(NULL);
		return NULL;
	}
	struct sockaddr_in client_address ;
	int addr_length = sizeof(struct sockaddr);
	printf("waiting for an upload request on port %d \n",PEER_DOWNLOAD_PORT);
	while(1){
		int new_connection = accept(server_sock_fd, (struct sockaddr_in *)&client_address, (socklen_t *) &addr_length);
		printf("received a new upload request %d \n",new_connection);
		if(new_connection > 0){
			// create a new thread for the upload handler and
			pthread_t upload_handler_thread;
            int* connew = (int*) malloc(sizeof(int));
            *connew = new_connection;
            
			pthread_create(&upload_handler_thread,NULL,p2p_upload,(void*)connew);
		}else{
			printf("Error in accepting new upload request\n");
		}
	}
	return NULL;
}

void *file_upload_request_handler_diff(){
	int server_sock_fd = get_server_socket_fd(PEER_DOWNLOAD_PORT_DIFFERENT,MAX_PEERS_NUM);
	if(server_sock_fd < 0 ){
		printf("unable to create listening socket for upload handler for diff platforms\n");
		pthread_exit(NULL);
		return NULL;
	}
	struct sockaddr_in client_address ;
	int addr_length = sizeof(struct sockaddr);
	printf("waiting for an upload request on port %d \n",PEER_DOWNLOAD_PORT_DIFFERENT);
	while(1){
		int new_connection = accept(server_sock_fd, (struct sockaddr_in *)&client_address, (socklen_t *) &addr_length);
		printf("received a new upload request %d \n",new_connection);
		if(new_connection > 0){
			// create a new thread for the upload handler and
			pthread_t upload_handler_thread_diff;
			pthread_create(&upload_handler_thread_diff,NULL,p2p_upload_diff,(void*)&new_connection);
		}else{
			printf("Error in accepting new upload request for diff platform\n");
		}
	}
	return NULL;
}

void start_peer_in_test() {
	pthread_t file_upload_thread;
	pthread_create(&file_upload_thread, NULL, file_upload_request_handler,NULL);
	char buffer[MAX_FILE_NAME_LEN];
	input_string("download file name\n", buffer, MAX_FILE_NAME_LEN);
	Node *fileInfo = (Node *) malloc(sizeof(Node));
	memcpy(fileInfo->name, buffer, MAX_FILE_NAME_LEN);
	fileInfo->size = 60;
	fileInfo->peernum = 1;
	fileInfo->peerip[0] = (unsigned int) get_ip_address_hostname("tahoe.cs.dartmouth.edu");
	download_file(fileInfo);
	while(1){

	}
}

char* get_dir(){
	return dirname;
}

void start_peer(char *argv[]){
	//TODO need to figure out the use of arguments

	readConfigFile(&dirname);
//	char* filename="../sync";
	printf("Get sync dir: %s\n",dirname);

	filetable=filetable_init(dirname);
	downloadtable_create();
	filetable_print(filetable);
	//peertable=peertable_init();
	
	printf(" Connecting to tracker \n");
	network_conn=connectToTracker();
	if(network_conn<0){
		printf("Connect to Tracker fail \n");
		exit(0);
	}
    	signal(SIGINT, peer_stop);

	file_tb_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(file_tb_mutex, NULL);
	download_tb_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(download_tb_mutex, NULL);

	pthread_t alive_thread;
    	pthread_create(&alive_thread,NULL,heartbeat,(void*)0);

	pthread_t file_monitor_thread;
	pthread_create(&file_monitor_thread,NULL,filemonitor,(void*)dirname);

	/**
	 * starting thread for listening to upload request
	 */
	pthread_t file_upload_thread ;
	pthread_create(&file_upload_thread,NULL,file_upload_request_handler, NULL);

	/**
		 * starting thread for listening to upload request from java clients
		 */
	pthread_t file_upload_thread_diff ;
	pthread_create(&file_upload_thread_diff,NULL,file_upload_request_handler_diff, NULL);

	while(1){
		// keep receving message from tracker
		ttp_seg_t recvseg;
		if(peer_recvseg(network_conn,&recvseg)<0){
  			printf("Receive filetable error\n");
			continue;
  		}
  		printf("Received segment from tracker\n");
  		printf("Received filetable size: %d \n",recvseg.file_table_size);
  		peer_update_filetable(recvseg.file_table,recvseg.file_table_size);
	}

}

