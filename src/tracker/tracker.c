// FILE: tracker/tracker.c
//
// Description: this file implements tracker
//
// Date: May 19,2016

#include "sys/time.h"
#include "tracker.h"

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
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        unsigned long int time = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
        
        int change = 0;
        
        pthread_mutex_lock(peer_tb_mutex);
        pthread_mutex_lock(file_tb_mutex);
        
        tracker_peer_t *p = peer_tb->head;

//        peer_table_print(peer_tb);

        while(p != NULL){
            // memset(ip_addr, 0, sizeof(char) * IP_LEN);
	    printf("time: time %lu last time: %lu \n",time,p->last_time_stamp);
            if(time - p->last_time_stamp >= HEARTBEAT_INTERVAL * 1000000){
                printf("Peer time out\n");
                memset(ip_addr, 0, IP_LEN);
                strncpy(ip_addr, p->ip, IP_LEN);
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

void* listen_handshake(void* arg){
    int conn = *((int*)arg);
    printf("LISTENING: %d \n",conn);
    ptt_seg_t pkt;
    ttp_seg_t sendpkg;
    while(tracker_recvseg(conn, &pkt)>0){
        switch (pkt.type) {
            case REGISTER:
                // send back ack
                printf("received register pkg\n");
                memset(&sendpkg, 0, sizeof(ttp_seg_t));
                sendpkg.interval = HEARTBEAT_INTERVAL;
                sendpkg.piece_len = PIECE_LENGTH;
                sendpkg.file_table_size = 0;

                if(tracker_sendseg(conn,&sendpkg)<0){
                    printf("Tracker send seg error\n");
                }
                break;
                
            case KEEP_ALIVE:
                printf("received keep alive pkg in socket: %d\n",conn);
                pthread_mutex_lock(peer_tb_mutex);
                peer_table_update_timestamp(peer_tb, conn);
                pthread_mutex_unlock(peer_tb_mutex);
                break;
                
            case FILE_UPDATE:
                printf("received file update package\n");
                pthread_mutex_lock(peer_tb_mutex);
                pthread_mutex_lock(file_tb_mutex);
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

    int fnum=file_tb->filenum;
    sendpkg->file_table_size=fnum;
    Node* sendfnode=file_tb->file;
    printf("Sending filetable: filenum: %d\n",fnum);
    int i=0;
    for(i=0;i<fnum;i++){
        memcpy(sendpkg->file_table+i,sendfnode,sizeof(Node));
        printf("name: %s\n",sendfnode->name);
        printf("timestamp: %lu\n",sendfnode->timestamp);
        printf("IP: %u\n",sendfnode->peerip);
        sendfnode=sendfnode->pNext;
    }

    tracker_peer_t *phead=peer_tb->head;
    while(phead!=NULL){
        if(tracker_sendseg(phead->sockfd,sendpkg)<0){
            printf("Tracker send broadcast pkg error\n");
            return -1;
        }
        phead=phead->next;
    }
    return 1;
}

int tracker_update_filetable(ptt_seg_t* recvseg){
    ptt_seg_t pkt;
    int num=recvseg->file_table_size;
    Node* head=recvseg->file_table;
    Node* curftable=file_tb->file;
    printf("Received filetable filenum: %d \n",num);

    if(num!=file_tb->filenum){
        peer_file_table* tmp=file_tb;
        filetable_destroy(tmp);
        file_tb=file_table_create();
        if(num==0){
            return 1;
        }
        int i;
        for(i=0;i<num;i++){
	    printf("add node name: %s \n",head->name);
            filetable_addnode(file_tb, head->size, head->name, head->timestamp);
            head++;
        }
        return 1;
    }
    else{
        int i;
        int change=0;
        for(i=0;i<num;i++){
            if(compareNode(head,curftable)<0){
                change=1;
                break;
            }
            head++;
            curftable=curftable->pNext;
        }
        if(change!=0){
            peer_file_table* tmp=file_tb;
            filetable_destroy(tmp);
            file_tb=file_table_create();
            if(num==0){
                return 1;
            }
            int i;
            for(i=0;i<num;i++){
                filetable_addnode(file_tb, head->size, head->name, head->timestamp);
                head++;
            }
            return 1;
        }
        else{
            return 0;
        }
    }


}

int compareNode(Node* a, Node* b){
    if(a->size==b->size&&(a->timestamp==b->timestamp)&&(a->peernum==b->peernum)&&(strcmp(a->name,b->name)==0)){
        return 1;
    }
    else{
        return -1;
    }

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
    
    printf("binding socket:\n");

    if(bind(lis_hdshake_conn, (struct sockaddr*) &svr_addr, sizeof(svr_addr)) < 0){
        perror("ERROR on binding");
        return -1;
    }
    printf("listening socket:\n");
    
    if(listen(lis_hdshake_conn, MAX_LISTEN_PEER) < 0){
        perror("ERROR listening\n");
        return -1;
    }
    
    printf("Listening request from peer:");
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
    
    while(1){
        struct sockaddr_in clt_addr;
        int cltlen = sizeof(clt_addr);
        int peer_conn = accept(lis_hdshake_conn, (struct sockaddr*)&clt_addr, &cltlen);
        if(peer_conn < 0){
            perror("ERROR accepting");
            continue;
        }
        
        pthread_mutex_lock(peer_tb_mutex);
        peer_table_add(peer_tb, &clt_addr.sin_addr, peer_conn);
        pthread_mutex_unlock(peer_tb_mutex);

	pthread_mutex_lock(peer_tb_mutex);
	peer_table_print(peer_tb);
	pthread_mutex_unlock(peer_tb_mutex);
        
        int *conn = (int*) malloc(sizeof(int));
        *conn = peer_conn;
        pthread_t handshake_thread;
        pthread_create(&handshake_thread, NULL, listen_handshake, conn);
    }
}
