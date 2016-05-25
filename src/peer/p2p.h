#include "../utils/constants.h"

typedef struct p2p_seg{
	char file_name[MAX_FILE_NAME_LEN];
	int piece_len;
	int start_idx;
	int end_idx;
} peer2peer_seg;

typedef struct download_seg{
	int socket;
	peer2peer_seg seg;
} peerdownload_seg;

int send_p2p_seg(int network_conn, peer2peer_seg* send_seg);
void peerdownload(void* arg);
void peerupload(void* arg);
