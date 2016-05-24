/* The packet data structure sending from peer to tracker */
typedef struct segment_peer {
// protocol length
int protocol_len;
// protocol name
char protocol_name[PROTOCOL_LEN + 1];
// packet type : register, keep alive, update file table
int type;
// reserved space, you could use this space for your convenient, 8 bytes by default
char reserved[RESERVED_LEN];
// the peer ip address sending this packet
char peer_ip[IP_LEN];
// listening port number in p2p
int port;
// the number of files in the local file table -- optional
int file_table_size;
// file table of the client -- your own design
file_t file_table;
}ptp_peer_t;
