#include "peer_peertable.h"
#include <stdio.h>
#include <stdlib.h>

peer_peer_table* peertable_init(){
	peer_peer_table* tmp=(peer_peer_table*)malloc(sizeof(peer_peer_table));
	tmp->peernum=0;
	tmp->peer=NULL;
	return tmp;
}


int peer_add_peer(peer_peer_table* peertable,char* ip, char* name, unsigned long timestamp, int sockfd){

}

int ptable_destroy(peer_peer_t* ptable){

}