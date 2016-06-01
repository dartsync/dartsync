#include "../peer/filemonitor.h"

#define	REGISTER 0
#define	KEEP_ALIVE 1
#define	FILE_UPDATE 2


/* The packet data structure sending from tracker to peer */
typedef struct segment_tracker{
// time interval that the peer should sending alive message periodically
int interval;
// piece length
int piece_len;
// file number in the file table -- optional
int file_table_size;
// file table of the tracker -- your own design
Node file_table[MAX_FILE_NUM];
} ttp_seg_t;

/* The packet data structure sending from peer to tracker */
typedef struct segment_peer {
// protocol length
int protocol_len;
// protocol name
char protocol_name[128];
// packet type : register, keep alive, update file table
int type;
// the peer ip address sending this packet
unsigned int peer_ip;
// listening port number in p2p
int port;
// the number of files in the local file table -- optional
int file_table_size;
// reserved space, you could use this space for your convenient, 8 bytes by default
char reserved[RESERVED_LEN];
// file table of the client -- your own design
Node file_table[MAX_FILE_NUM];

int peer_type ;
}ptt_seg_t;


//peer_tracker_upload(int netconn,struct node* filetable);
//register_to_tracker();
int peer_sendseg(int network_conn, ptt_seg_t* sendseg);
int tracker_sendseg(int network_conn, ttp_seg_t* sendseg);
int peer_recvseg(int network_conn, ttp_seg_t* recvseg);
int tracker_recvseg(int network_conn, ptt_seg_t* recvseg);
