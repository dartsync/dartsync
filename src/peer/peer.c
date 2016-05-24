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
//#include "filetable.h"

int heartbeat_interval;
int piece_len;
int network_conn=-1;
peer_file_table *filetable;
//peer_peer_table *peertable;


int connectToTracker(){
  int out_conn;
  struct sockaddr_in servaddr;
  servaddr.sin_family =AF_INET;
  servaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
  servaddr.sin_port = htons(TRACKER_PORT);
  out_conn = socket(AF_INET,SOCK_STREAM,0);

  if(out_conn<0) {
	printf("Create socket error\n");
	return -1;
  }
  if(connect(out_conn, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
	printf("connect to tracker: error cnnecting to trcker \n");
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
	return retnum;
}

int peer_update_filetable(Node* recv,int recvnum){
	int curnum=filetable->filenum;
	int num=0;
	Node* recvfpt=recv;
	Node* curfpt=filetable;
	int i=0;
	// file mutex
	while(num<recvnum&&curfpt!=NULL){
		Node* recvnode=recv+num;
		if(strcmp(recvnode->name,curfpt->name)<0){
			//pthread_t peer_download_thread;
			//pthread_create(&peer_download_thread,NULL,peerdownload,(void*)recvfpt->name);
			printf("Find new file: %s\n",recvnode->name);
//			download_file(recvfpt);
			num++;
		}
		else if(strcmp(recvnode->name,curfpt->name)>0){
			printf("Delete file: %s\n",curfpt->name);
			remove(curfpt->name);
			fileDeleted(filetable, curfpt->name);
			curfpt=curfpt->pNext;
		}
		else{
			if(recvnode->size!=curfpt->size||recvnode->timestamp!=curfpt->timestamp){
				// tracker and peer both have this file, but peer side file need to be updated
//				download_file(recvfpt);
				num++;
				curfpt=curfpt->pNext;
			}
			else{
				num++;
				curfpt=curfpt->pNext;
			}
		}
	}

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

/*int download_file(Node* fnode){
	printf("In download_file: downloading file: %s \n", fnode->name);
	int filesize=fnode->size;
	int peernum=fnode->peernum;
	//creat a download thread for each peer that own the requested file
	int i;
	for(i=0;i<peernum;i++){
		// find if there exist a temp file, if not exit, try to download from peer
		if(){
			int download_sock;
			// creat a new socket for connection
		    if( (download_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		    printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
		    exit(0);
		    }
		    memset(&servaddr, 0, sizeof(servaddr));
		    servaddr.sin_family = AF_INET;
		    servaddr.sin_port = htons();
		    if(inet_pton(AF_INET, serveraddr, &servaddr.sin_addr) <= 0){
		   		printf("inet_pton error for %s\n",argv[1]);
		    	exit(0);
		    }
		    if(connect(download_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
		    	printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
		    	exit(0);
		    }
		    peerdownload_seg download_file;

		    download_file->socket=download_sock;
		    download_file->seg.piece_len=
			download_file->seg.start_idx=
			download_file->seg.end_idx=


			pthread_t peer_download_thread;
			pthread_create(&peer_download_thread,NULL,peerdownload,(void*)&download_file);
		}
	}
	//wait until all the tmp file have been downloaded
	while(){

	}

}*/

void* filemonitor(void* arg){
	char* dir=(char*)arg;
	watchDirectory(filetable,dir);

}

void* heartbeat(){
	printf("Now in heartbeat function\n");
	while(1){
		sleep(HEARTBEAT_INTERVAL);
		ptt_seg_t* sendseg=(ptt_seg_t*)malloc(sizeof(ptt_seg_t)); 
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
	printf("Peer stop!\n");
	exit(0);
}

void start_peer(char *argv[]){
	//TODO fetch ip_address and root dir from argv.
	//input_string("Enter the IP address of tracker (max 100 chars ) \n",tracker_host_name,TRACKER_ADDRESS_LENGTH);



	//char filename[MAX_FILE_NAME];
	char filename[1024];
	readConfigFile(&filename);
	printf("Get sync dir: %s\n",filename);

	filetable=filetable_init(filename);
	filetable_print(filetable);
	//peertable=peertable_init();
	
	printf(" Connecting to tracker \n");
	network_conn=connectToTracker();
	if(network_conn<0){
		printf("Connect to Tracker fail \n");
		exit(0);
	}

    pthread_t alive_thread;
    pthread_create(&alive_thread,NULL,heartbeat,(void*)0);

    signal(SIGINT, peer_stop);

	pthread_t file_monitor_thread;
	pthread_create(&file_monitor_thread,NULL,filemonitor,(void*)filename);

	//pthread_t peer_listening_thread;
	//pthread_create(&peer_listening_thread,NULL,peerlistening,(void*)0);

	while(1){
		// keep receving message from tracker
		ttp_seg_t recvseg;
		if(peer_recvseg(network_conn,&recvseg)<0){
  			printf("Receive filetable error\n");
  		}
  		printf("Received segment from tracker\n");
  		printf("Received filetable size: %d \n",recvseg.file_table_size);
  		peer_update_filetable(recvseg.file_table,recvseg.file_table_size);


	}

}

