// FILE: tracker/peertable.c
//
// Description: this file implements peer table
//
// Date: May 19,2016

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

int peer_table_add(peer_table *table, struct in_addr *addr, int conn){
    char ip_addr[IP_LEN];
    memset(ip_addr, 0, sizeof(char) * IP_LEN);
    if(inet_ntop(AF_INET, &(addr->s_addr), ip_addr, INET_ADDRSTRLEN) == NULL){
        printf("Error: cannot get node ID from ip\n");
        return -1;
    }
    
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    
    tracker_peer_t *p = (tracker_peer_t*) malloc(sizeof(tracker_peer_t));
    strncpy(p->ip, ip_addr, IP_LEN);
    p->last_time_stamp = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
    p->next = table->head;
    p->sockfd = conn;
    table->head = p;
    table->peer_num++;
    
    return 1;
}

int peer_table_delete(peer_table *table, const char* ip_addr){
    if(table->peer_num <= 0)
        return -1;
    
    if(strcmp(ip_addr, table->head->ip) == 0){
        tracker_peer_t *t = table->head;
        table->head = t->next;
        free(t);
        table->peer_num--;
        return 1;
    }
    
    tracker_peer_t *p = table->head;
    while(p->next != NULL){
        if(strcmp(ip_addr, p->next->ip) == 0){
            tracker_peer_t *t = p->next;
            p->next = t->next;
            free(t);
            table->peer_num--;
            return 1;
        }
    }
    
    return -1;
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
    }
    return -1;
}

void peer_table_print(peer_table *table){
    printf("------The contents of the peer table------\n");
    tracker_peer_t *p = table->head;
    while(p != NULL)
        printf("IP address: %s, connection port: %d\n", p->ip, p->sockfd);
    printf("\n");
}





