#include "p2p.h"

int send_p2p_seg(int network_conn, peer2peer_seg* send_seg){
	send(network_conn,send_seg,sizeof(peer2peer_seg),0);
}



void peerdownload(void* arg){
	peerdownload_seg* download=(peerdownload_seg*)arg;
	peer2peer_seg send_seg=(peer2peer_seg*)malloc(sizeof(peer2peer_seg));

	int peer_conn=download->socket;
	int startidx=download->seg->start_idx;
	int piece_len=download->seg->piece_len;
	int endidx=download->seg->end_idx;

	memcpy();
	send_seg->piece_len=piece_len;
	send_seg->start_idx=startidx;
	send_seg->end_idx=endidx;

	filelen=end_idx-startidx;
	if(peer_conn<0){
		printf("download thread error: wrong network socket\n");


	}
	//send file info to target peer
	send_p2p_seg(peer_conn,send_seg);
	printf("Peer download thread: start download file from remote peer\n");
	int downloaded_size=0;
	char* buf=(char*)malloc(filelen);
	FILE *fp = NULL;
	char recvfilename[MAX_FILE_LEN];
	sprintf(recvfilename,"%s_%d_%d~",download->seg.file_name,startidx,endidx);
	char recvbuf[filelen];
	int recv_len=0;
	fp=fopen(recvfilename, "a");
	if ((fp = fopen()) == NULL) {
        printf("Fail to open file\n");
    }

	while(recv_len<filelen){
		//memset();
		int tmp;
		bzero(recvbuf,sizeof(recvbuf));
		if(tmp=recv(peer_conn,recvbuf+recv_len,sizeof(recvbuf)-recv_len,0)<=0){
			printf("receive data fail\n");
		}
		recv_len+=tmp;

	}
	fwrite(recvbuf,sizeof(char),,fp);
	
	close(peer_conn);
	fclose(fp);
	pthread_exit(NULL);
}

void peerupload(void* arg){
	printf("Now in peer upload thread \n");
	int p2p_conn=(int*)arg;
	if(p2p_conn<0){
		printf("peerupload thread error: wrong connection socket\n");


	}
	//receive the p2p seg from requested peer
	peer2peer_seg recv_seg;
	p2p_recvseg(p2p_conn,&recv_seg);
	int piece_len=recv_seg.piece_len;
	int startidx=recv_seg.start_idx;
	int endidx=recv_seg.end_idx;
	int filelen=endidx-startidx;
	int upload_len=0;
	char sendbuf[piece_len];

    FILE *fp;
    if ((fp = fopen()) == NULL) {
        printf("Fail to open file\n");
    }
    else{
    	while(upload_len<filelen){
    		int len;
    		if(filelen-upload_len>piece_len){
    			len=piece_len;
    		}
    		else{
    			len=filelen-upload_len;
    		}
    		if(fseek(fp,startidx+upload_len,0)<0){
    			printf("Fseek error!\n");
    		}
    		if(fread(sendbuf,sizeof(char),len,fp)<0){
    			printf("Fread error \n");
    		}
    		if(send(p2p_conn,sendbuf,len)<0){
    			printf("peerupload: send data error\n");
    		}
    		upload_len+=len;
		}
    }
    fclose(fp);
}