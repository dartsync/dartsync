//
//  peertable.h
//  tracker
//

#ifndef peertable_h
#define peertable_h

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/utsname.h>
#include <assert.h>
#include "../utils/constants.h"

typedef struct _tracker_side_peer_t {
    //Remote peer IP address, 16 bytes.
    char ip[IP_LEN];
    //Last alive timestamp of this peer.
    unsigned long last_time_stamp;
    //TCP connection to this remote peer.
    int sockfd;
    //Pointer to the next peer, linked list.
    struct _tracker_side_peer_t *next;
} tracker_peer_t;

// In case if we want add some features into this structure
typedef struct _tracker_side_peer_table {
    tracker_peer_t *head;
    int peer_num;
} peer_table;

peer_table* peer_table_create();

int peer_table_destroy(peer_table *table);

int peer_table_add(peer_table *table, struct in_addr* addr, int conn);

int peer_table_delete(peer_table *table, const char *ip_addr);

int peer_table_update_timestamp(peer_table *table, int conn);

void peer_table_print(peer_table *table);

#endif /* peertable_h */
