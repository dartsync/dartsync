#include "../utils/constants.h"

typedef struct _peer_side_peer_t {
// Remote peer IP address, 16 bytes.
char ip[IP_LEN];
//Current downloading file name.
char file_name[MAX_FILE_NAME_LEN];
//Timestamp of current downloading file.
unsigned long file_time_stamp;
//TCP connection to this remote peer.
int sockfd;
//Pointer to the next peer, linked list.
struct _peer_side_peer_t *next;
} peer_peer_t;

typedef struct filetable{
	int peernum;
	peer_peer_t* peer;
}peer_peer_table;

peer_peer_table* peertable_init();
int peer_add_peer(peer_peer_table* peertable,char* ip, char* name, unsigned long timestamp, int sockfd);
int ptable_destroy(peer_peer_t* ptable);