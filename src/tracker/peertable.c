// FILE: tracker/peertable.c
//
// Description: this file implements peer table
//
// Date: May 19,2016

#include "sys/time.h"
#include "peertable.h"

peer_table* peer_table_create(){
    peer_table *table = (peer_table*) malloc(sizeof(peer_table));
    table->head = NULL;
    table->peer_num = 0;
    return table;
}

int peer_table_destroy(peer_table *table){
    while(table->head != NULL){
        tracker_peer_t *t = table->head;
        table->head = t->next;
        free(t);
        table->peer_num--;
    }
    free(table);
    return 1;
}

int peer_table_add(peer_table *table, unsigned int ip_addr, int conn){
    //    char ip_addr[IP_LEN];
    //    memset(ip_addr, 0, sizeof(char) * IP_LEN);
    //    if(inet_ntop(AF_INET, &(addr->s_addr), ip_addr, INET_ADDRSTRLEN) == NULL){
    //        printf("Error: cannot get node ID from ip\n");
    //        return -1;
    //    }
    
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    
    tracker_peer_t *p = (tracker_peer_t*) malloc(sizeof(tracker_peer_t));
    // strncpy(p->ip, ip_addr, IP_LEN);
    // p->ip = ntohl(addr->s_addr);
    p->ip = ip_addr;
    p->last_time_stamp = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
    p->next = table->head;
    p->sockfd = conn;
    p->peer_type = PEER_TYPE_DEFAULT;
    table->head = p;
    table->peer_num++;
    printf("Add peer %u in connection %d\n", p->ip, conn);
    return 1;
}
int peer_table_add_d(peer_table *table, unsigned int ip_addr, int conn){
    //    char ip_addr[IP_LEN];
    //    memset(ip_addr, 0, sizeof(char) * IP_LEN);
    //    if(inet_ntop(AF_INET, &(addr->s_addr), ip_addr, INET_ADDRSTRLEN) == NULL){
    //        printf("Error: cannot get node ID from ip\n");
    //        return -1;
    //    }

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    tracker_peer_t *p = (tracker_peer_t*) malloc(sizeof(tracker_peer_t));
    // strncpy(p->ip, ip_addr, IP_LEN);
    // p->ip = ntohl(addr->s_addr);
    p->ip = ip_addr;
    p->last_time_stamp = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
    p->next = table->head;
    p->sockfd = conn;
    p->peer_type = PEER_TYPE_DIFFERENT;
    table->head = p;
    table->peer_num++;
    printf("Add peer %u in connection %d\n", p->ip, conn);
    return 1;
}

int peer_table_delete(peer_table *table, unsigned int ip_addr){
    if(table->peer_num <= 0)
        return -1;
    
    if(table->head->ip == ip_addr){
        tracker_peer_t *t = table->head;
        table->head = t->next;
        free(t);
        table->peer_num--;
        return 1;
    }
    
    tracker_peer_t *p = table->head;
    while(p->next != NULL){
        if(p->next->ip == ip_addr){
            tracker_peer_t *t = p->next;
            p->next = t->next;
            free(t);
            table->peer_num--;
            return 1;
        }
        p=p->next;
    }
    
    return -1;
}

int peer_table_get_type(peer_table *table, unsigned int ip_address){
    tracker_peer_t *p = table->head;
    while(p != NULL){
        if(p->ip == ip_address){
            return p->peer_type;
        }
        p=p->next;
    }
    return PEER_TYPE_DEFAULT;
}

int peer_table_update_timestamp(peer_table *table, int conn){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    tracker_peer_t *p = table->head;
    while(p != NULL){
        if(p->sockfd == conn){
            p->last_time_stamp = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
            return 1;
        }
        p=p->next;
    }
    return -1;
}

int peer_table_update_timestamp_new(peer_table *table, unsigned long time, int conn){
    tracker_peer_t *p = table->head;
    while(p != NULL){
        if(p->sockfd == conn){
            p->last_time_stamp = time;
            return 1;
        }
        p = p->next;
    }
    return -1;
}

void peer_table_print(peer_table *table){
    printf("\n------The contents of the peer table------\n");
    tracker_peer_t *p = table->head;
    while(p != NULL){
        printf("IP address: %u, connection port: %d\n", p->ip, p->sockfd);
        p = p->next;
    }
    printf("\n");
}





