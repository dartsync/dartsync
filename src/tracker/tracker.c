// FILE: tracker/tracker.c
//
// Description: this file implements tracker
//
// Date: May 19,2016

#include "sys/time.h"
#include "tracker.h"
#include "../network/network_utils.h"

#define MAX_LISTEN_PEER 10

int lis_hdshake_conn;
peer_table *peer_tb;
peer_file_table *file_tb;
pthread_mutex_t *peer_tb_mutex;
pthread_mutex_t *file_tb_mutex;


void* monitor_alive(){
    char ip_addr[IP_LEN];
    ttp_seg_t sendpkg;
    
    while(lis_hdshake_conn > 0){
        
        int change = 0;
        
        pthread_mutex_lock(peer_tb_mutex);
        pthread_mutex_lock(file_tb_mutex);
        
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        unsigned long int time = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
        
        tracker_peer_t *p = peer_tb->head;
        
        while(p != NULL){
            // memset(ip_addr, 0, sizeof(char) * IP_LEN);
            //printf("time: time %lu last time: %lu \n", time, p->last_time_stamp);
            //printf("time interval: %lu\n", time - p->last_time_stamp);
            if(time - p->last_time_stamp >= HEARTBEAT_INTERVAL * 1000000){
                printf("Peer time out\n");
                unsigned int ip_addr = p->ip;
                close(p->sockfd);
                peer_table_delete(peer_tb, ip_addr);
                file_table_deleteip(file_tb, ip_addr);
                change = 1;
            }
            p = p->next;
        }
        if(change){
            // send broadcast
            if(broadcast_filetable(&sendpkg)<0){
                printf("Send broadcast pkg error\n");
            }
            
            peer_table_print(peer_tb);
            filetable_print(file_tb);
        }
        pthread_mutex_unlock(file_tb_mutex);
        pthread_mutex_unlock(peer_tb_mutex);
        
        sleep(HEARTBEAT_INTERVAL);
    }
    pthread_exit(NULL);
}

int parser_ptt_seg(char *buffer, char *delimiter,ptt_seg_t *seg){
	char *token;
	/**
	 * 1. type
	 * 2. protocol len
	 * 3. ip
	 * 4. file table size
	 */
	token = strtok(buffer,delimiter) ;
	printf("1st = %s\n",token);
	if(token == NULL) return FALSE;
	seg->type = atoi(token);
	if((token = strtok(NULL,delimiter)) == NULL) return FALSE ;
	printf("2nd = %s\n",token);
	seg->protocol_len = atoi(token);
	if((token = strtok(NULL,delimiter)) == NULL) return FALSE ;
	printf("3rd = %s\n",token);
	seg->peer_ip = atoi(token);
	if((token = strtok(NULL,delimiter)) == NULL) return FALSE ;
	printf("4th = %s\n",token);
	seg->file_table_size = atoi(token);
	if(seg->type == FILE_UPDATE){
		printf("filetable found = %s\n", token);
		for(int i = 0 ; i < seg->file_table_size ; i++){
			Node *node = (Node *) malloc(sizeof(Node));
			// file name
			if((token = strtok(NULL,delimiter)) == NULL){
				free(node);
				return FALSE ;
			}
			memcpy(node->name,token,256);
			printf("filename = %s\n",node->name);
			// file file size
			if((token = strtok(NULL,delimiter)) == NULL){
				free(node);
				return FALSE ;
			}
			node->size = atoi(token);
			printf("filesize = %d\n",node->size);
			// file timestamp
			if((token = strtok(NULL,delimiter)) == NULL){
				free(node);
				return FALSE ;
			}
			node->timestamp = atoi(token);
			printf("file time = %d\n",node->timestamp);
			seg->file_table[i] = *node;
		}

	}
	return TRUE;
}

void *listen_handshake_platform(void* arg){
    int conn = *((int*)arg);
    printf("LISTENING: DIFF PLATFORM :- %d \n",conn);
    char buffer[256];
    ptt_seg_t pkt;
    //bzero(&pkt,sizeof(ptt_seg_t));
    while(recv(conn,buffer,256,0)>0){
    	printf("%s\n",buffer);
    	parser_ptt_seg(buffer,",",&pkt);
    	switch(pkt.type){
    		case REGISTER:{
                printf("received register packet DIFF type = %d, protocol len = %d, file table size = %d, peer_ip = %d \n",pkt.type, pkt.protocol_len,pkt.file_table_size,pkt.peer_ip);
                bzero(buffer,256);
                sprintf(buffer,"%d,%d,%d%s",HEARTBEAT_INTERVAL,PIECE_LENGTH,0,"\n");
                printf("sending :- %s",buffer);
                //send(conn,"hello\n",7,0);
                if (send(conn,buffer,256,0) < 0) {
                    printf("Tracker send seg error\n");
                }
    		}
    		break;
    		case KEEP_ALIVE:{
    			printf("keep alive packet DIFF in socket: %d, t = %d, pl = %d, ip = %d, fs = %d \n",conn, pkt.type, pkt.protocol_len, pkt.peer_ip, pkt.file_table_size);
				pthread_mutex_lock(peer_tb_mutex);
				peer_table_update_timestamp(peer_tb, conn);
				pthread_mutex_unlock(peer_tb_mutex);
			}
			break;
    		case FILE_UPDATE:{
    			printf("file update packet DIFF in socket: %d \n",conn);
    			for(int i = 0 ; i < pkt.file_table_size ; i++){
        			printf("file name = %s,size =  %d, time = %d \n",pkt.file_table[i].name,pkt.file_table[i].size,pkt.file_table[i].timestamp);
    			}
				pthread_mutex_lock(peer_tb_mutex);
				pthread_mutex_lock(file_tb_mutex);
	               if(peer_table_update_timestamp(peer_tb, conn) > 0){
	            	   pkt.peer_type = PEER_TYPE_DIFFERENT;
	                    if(tracker_update_filetable(&pkt) > 0){
	                        ttp_seg_t sendpkg;
	                        printf("Filetalbe updated\n");
	                        filetable_print(file_tb);
	                        memset(&sendpkg, 0, sizeof(ttp_seg_t));
	                        sendpkg.interval = HEARTBEAT_INTERVAL;
	                        sendpkg.piece_len = PIECE_LENGTH;
	                        if(broadcast_filetable(&sendpkg)<0){
	                            printf("Send broadcast pkg error\n");
	                        }
	                    }
	                }
				pthread_mutex_unlock(file_tb_mutex);
				pthread_mutex_unlock(peer_tb_mutex);
			}
			break;
    	}
    }
    fflush(stdout);
    return NULL;
}
void* listen_handshake(void* arg){
    int conn = *((int*)arg);
    printf("LISTENING: %d \n",conn);
    ptt_seg_t pkt;
    ttp_seg_t sendpkg;
    struct in_addr ip_addr;
    while(tracker_recvseg(conn, &pkt)>0){
        switch (pkt.type) {
            case REGISTER:
                // send back ack
                printf("received register pkg\n");
                memset(&sendpkg, 0, sizeof(ttp_seg_t));
                sendpkg.interval = HEARTBEAT_INTERVAL;
                sendpkg.piece_len = PIECE_LENGTH;
                sendpkg.file_table_size = 0;
                
                pthread_mutex_lock(peer_tb_mutex);
                peer_table_add(peer_tb, pkt.peer_ip, conn);
                peer_table_print(peer_tb);
                pthread_mutex_unlock(peer_tb_mutex);
                
                if(tracker_sendseg(conn,&sendpkg)<0){
                    printf("Tracker send seg error\n");
                }
                break;
                
            case KEEP_ALIVE:
                ip_addr.s_addr = pkt.peer_ip;
                //printf("received keep alive pkg in socket %d from %s\n",conn, inet_ntoa(ip_addr));
                pthread_mutex_lock(peer_tb_mutex);
                peer_table_update_timestamp(peer_tb, conn);
                pthread_mutex_unlock(peer_tb_mutex);
                break;
                
            case FILE_UPDATE:
                printf("received file update package from: %d\n",conn);
                pthread_mutex_lock(peer_tb_mutex);
                pthread_mutex_lock(file_tb_mutex);
         	   pkt.peer_type = PEER_TYPE_DEFAULT;
                if(peer_table_update_timestamp(peer_tb, conn) > 0){
                    if(tracker_update_filetable(&pkt) > 0){
                        printf("Filetalbe updated\n");
                        filetable_print(file_tb);
                        memset(&sendpkg, 0, sizeof(ttp_seg_t));
                        sendpkg.interval = HEARTBEAT_INTERVAL;
                        sendpkg.piece_len = PIECE_LENGTH;
                        
                        if(broadcast_filetable(&sendpkg)<0){
                            printf("Send broadcast pkg error\n");
                        }
                    }
                }
                pthread_mutex_unlock(file_tb_mutex);
                pthread_mutex_unlock(peer_tb_mutex);
                break;
                
            default:
                break;
        }
    }
    
    printf("Listening thread exit");
    free(arg);
    close(conn);
    pthread_exit(NULL);
}

int broadcast_filetable(ttp_seg_t* sendpkg){
    char buffer_diff[1024];
    bzero(buffer_diff,1024);
    int fnum=file_tb->filenum;
    sendpkg->file_table_size=fnum;
    Node* sendfnode=file_tb->file;
    printf("Sending filetable: filenum: %d\n",fnum);
	sprintf(buffer_diff,"%d,",fnum);
    int i=0;
    for(i=0;i<fnum;i++){
        memcpy(sendpkg->file_table+i,sendfnode,sizeof(Node));
        printf("name: %s\n",sendfnode->name);
        printf("timestamp: %lu\n",sendfnode->timestamp);
        for(int j = 0 ; j < sendfnode->peernum; j++){
        	 printf("IP: %d , Type: %d \n",sendfnode->peerip[j], sendfnode->peer_type[j]);
        }
        append_node(sendfnode,buffer_diff);
        printf("after append buffer = %s\n",buffer_diff);
        if(i < fnum - 1)
        	sprintf(buffer_diff,"%s%s",buffer_diff,",");
        sendfnode=sendfnode->pNext;
    }
	sprintf(buffer_diff,"%s%s,",buffer_diff,"\n");
    tracker_peer_t *phead=peer_tb->head;
    while(phead!=NULL){
    	if(phead->peer_type == PEER_TYPE_DEFAULT){
			if(tracker_sendseg(phead->sockfd,sendpkg)<0){
				printf("Tracker send broadcast pkg error\n");
				return -1;
			}
    	}else{
    		printf("in broadcast DIFFERNT platform peer detected :- \n");
    		printf("sending :- %s",buffer_diff);
    		if(send(phead->sockfd,buffer_diff,sizeof(buffer_diff),0)<0){
    			printf("Tracker send broadcast pkt error DIFF\n");
    			return -1;
    		}
    	}
        phead=phead->next;
    }
    return 1;
}

int tracker_update_filetable(ptt_seg_t* recvseg){
    ptt_seg_t pkt;
    int num = recvseg->file_table_size;
    Node* head = recvseg->file_table;
    Node* curftable = file_tb->file;
    Node *prev = NULL;
    printf("Received filetable, file num: %d \n",num);
    int i;
    int change = 0;
    
    for(i = 0; i < num; i++){
        if(curftable != NULL && strcmp(head[i].name, curftable->name) == 0){
            printf("compare node in list %s", head[i].name);
            if(head[i].name[strlen(head[i].name) - 1] == '/'){
                printf("This is a dir! dir does not change\n");
            }else if(curftable->size == head[i].size && curftable->timestamp == head[i].timestamp){
                int flag = 1;
                for(int i = 0; i < curftable->peernum; i++){
                    if(curftable->peerip[i] == recvseg->peer_ip){
                        flag = 0;
                        break;
                    }
                }
                if(flag == 1 && curftable->peernum < MAX_PEER_NUM){
                    curftable->peerip[curftable->peernum] = recvseg->peer_ip;
                    curftable->peer_type[curftable->peernum] = recvseg->peer_type;
                    curftable->peernum++;
                    // change = 1;
                }
                printf(" size and time stamp didn't change\n");
            }else if(curftable->timestamp < head[i].timestamp){
                curftable->size = head[i].size;
                curftable->timestamp = head[i].timestamp;
                curftable->peerip[0] = recvseg->peer_ip;
                curftable->peer_type[0] = recvseg->peer_type;// peer_table_get_type(peer_tb,recvseg->peer_ip);
                curftable->peernum = 1;
                change = 1;
                printf(" size and time stamp change\n");
            }else{
                change = 1;
                printf("Peer %d has old version of %s\n", recvseg->peer_ip, head[i].name);
            }
                printf("The size is %d, the time stamp is: %d\n", head[i].size, head[i].timestamp);
                prev = curftable;
                curftable = curftable->pNext;
            }else if(curftable != NULL && strcmp(head[i].name, curftable->name) < 0){
                printf("add node in list %s\n", head[i].name);
                Node *t = (Node*) malloc(sizeof(Node));
                memset(t, 0, sizeof(Node));
                t->size = head[i].size;
                t->peernum = 1;
                t->timestamp = head[i].timestamp;
                memcpy(t->name, head[i].name, strlen(head[i].name));
                t->peerip[0] = recvseg->peer_ip;
            
                if(prev == NULL){
                    t->pNext = curftable;
                    file_tb->file = t;
                    prev = file_tb->file;
            }else{
                t->pNext = curftable;
                prev->pNext = t;
                prev = t;
            }
            change = 1;
            file_tb->filenum++;
            
        }else if(curftable != NULL && strcmp(head[i].name, curftable->name) > 0){
        printf("delete node in list %s\n", head[i].name);
            Node *t = curftable;
            if(prev == NULL){
                file_tb->file = file_tb->file->pNext;
                curftable = curftable->pNext;
                free(t);
            }else{
                prev->pNext = t->pNext;
                curftable = curftable->pNext;
                free(t);
            }
            change = 1;
            file_tb->filenum--;
        i--;
        }else{
        printf("add node at last position %s\n", head[i].name);
            Node *t = (Node*) malloc(sizeof(Node));
            memset(t, 0, sizeof(Node));
            t->size = head[i].size;
            t->peernum = 1;
            t->timestamp = head[i].timestamp;
            memcpy(t->name, head[i].name, strlen(head[i].name));
            t->peerip[0] = recvseg->peer_ip;
		t->peer_type[0] = recvseg->peer_type;            
if(prev == NULL){
                file_tb->file = t;
                prev = file_tb->file;
            }else{
                prev->pNext = t;
                prev = t;
            }
            change = 1;
            file_tb->filenum++;
        }
    }
    
    while(curftable != NULL){
printf("delete node last remain nodes %s\n",  curftable->name);
    if(prev == NULL){
        Node *t = curftable;
        curftable = curftable->pNext;
        file_tb->file = NULL;
        free(t);
    }else{
        prev->pNext = NULL;
            Node *t = curftable;
            curftable = curftable->pNext;
            free(t);
    }
        change = 1;
        file_tb->filenum--;
    }
    
    return change;
}

void tracker_stop(){
    printf("Tracker stop...\n");
    close(lis_hdshake_conn);
    lis_hdshake_conn = -1;
    peer_table_destroy(peer_tb);
    free(peer_tb_mutex);
    filetable_destroy(file_tb);
    free(file_tb_mutex);
    exit(0);
}



void *differnt_platform_handler(){
	fflush(stdout);
	printf("different_platform_handler called \n");
	int serverSockFD = get_server_socket_fd(TRACKER_PORT_DIFFERENT,MAX_PEERS_NUM);
	if(serverSockFD > 0){
		printf("different platform handler started ... conn id = %d\n",serverSockFD);
		while (TRUE) {
			struct sockaddr_in clt_addr;
			socklen_t cltlen = sizeof(struct sockaddr_in);
			printf("listening for diff paltforms\n");
			int peer_conn = accept(serverSockFD,(struct sockaddr*) &clt_addr,&cltlen);
			printf("Received new client in diff platform handler");
			if (peer_conn < 0) {
				perror("ERROR accepting client from different platform");
				continue;
			}
			pthread_mutex_lock(peer_tb_mutex);
			peer_table_add_d(peer_tb, &clt_addr.sin_addr, peer_conn);
			pthread_mutex_unlock(peer_tb_mutex);
			pthread_mutex_lock(peer_tb_mutex);
			peer_table_print(peer_tb);
			pthread_mutex_unlock(peer_tb_mutex);
			int *conn = (int*) malloc(sizeof(int));
			*conn = peer_conn;
			pthread_t handshake_thread;
			pthread_create(&handshake_thread, NULL, listen_handshake_platform, conn);
		}
	}
	return NULL;
}

int start_tracker() {
    printf("Start tracker...\n");
    
    lis_hdshake_conn = socket(AF_INET, SOCK_STREAM, 6);
    if(lis_hdshake_conn < 0){
        perror("ERROR opening");
        return -1;
    }
    
    struct sockaddr_in svr_addr;
    memset((char *)&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(TRACKER_PORT);
    
    if(bind(lis_hdshake_conn, (struct sockaddr*) &svr_addr, sizeof(svr_addr)) < 0){
        perror("ERROR on binding");
        return -1;
    }    
    if(listen(lis_hdshake_conn, MAX_LISTEN_PEER) < 0){
        perror("ERROR listening\n");
        return -1;
    }
    
    printf("Listening request from peer:\n\n");
    // printf("%s\n", &svr_addr.sin_addr);
    
    signal(SIGINT, tracker_stop);
    
    peer_tb_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(peer_tb_mutex, NULL);
    peer_tb = peer_table_create();
    
    file_tb_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(file_tb_mutex, NULL);
    file_tb = file_table_create();
    
    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, monitor_alive, NULL);
    
    pthread_t diff_platform_thread;
    pthread_create(&diff_platform_thread, NULL, differnt_platform_handler, NULL);

    while(1){
        struct sockaddr_in clt_addr;
        int cltlen = sizeof(clt_addr);
        int peer_conn = accept(lis_hdshake_conn, (struct sockaddr*)&clt_addr, &cltlen);
        if(peer_conn < 0){
            perror("ERROR accepting");
            continue;
        }
        
//        pthread_mutex_lock(peer_tb_mutex);
//        peer_table_add(peer_tb, &clt_addr.sin_addr, peer_conn);
//        peer_table_print(peer_tb);
//        pthread_mutex_unlock(peer_tb_mutex);
        
        int *conn = (int*) malloc(sizeof(int));
        *conn = peer_conn;
        pthread_t handshake_thread;
        pthread_create(&handshake_thread, NULL, listen_handshake, conn);
    }
}
