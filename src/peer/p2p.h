#include "../utils/constants.h"

typedef struct p2p_seg{
	char file_name[MAX_FILE_NAME_LEN];
	int piece_len;
	int start_idx;
	int end_idx;
} peer2peer_seg;

typedef struct download_seg{
	int socket;
	unsigned int peer_ip;
	int isSuccess;
	FILE *tempFile;
	peer2peer_seg seg;
} peerdownload_seg;
typedef struct temp_download_seg{
	peerdownload_seg *download_seg;
	pthread_t thread;
}temp_download_t;
int send_p2p_seg(int network_conn, peer2peer_seg* send_seg);
void *p2p_upload(void* arg);
void *p2p_upload_diff(void *arg);
