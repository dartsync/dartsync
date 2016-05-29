#include "seg.h"

int peer_sendseg(int network_conn, ptt_seg_t* sendseg){
    int segSize;
	//  segSize=  ;
	if(send(network_conn, sendseg, sizeof(ptt_seg_t), 0) < 0) {
        free(sendseg);
		return -1;
	}
	free(sendseg);
}

int tracker_sendseg(int network_conn, ttp_seg_t* sendseg){
    int segSize;
	//  segSize=  ;
	if(send(network_conn, sendseg, sizeof(ttp_seg_t), 0) < 0) {
		return -1;
	}
}

int peer_recvseg(int network_conn, ttp_seg_t* recvseg){
	if(recv(network_conn,recvseg,sizeof(ttp_seg_t),0)>0) {
		return 1;	
	}
	return -1;
}

int tracker_recvseg(int network_conn, ptt_seg_t* recvseg){
	if(recv(network_conn,recvseg,sizeof(ptt_seg_t),0)>0){
		return 1;
	}
	return -1;
}
