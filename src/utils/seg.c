#include "seg.h"

int peer_sendseg(int network_conn, ptt_seg_t* sendseg){
    char bufstart[2];
	char bufend[2];
    int segSize;
    
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufend[0] = '!';
	bufend[1] = '#';

	if (send(network_conn, bufstart, 2, 0) < 0) {
        free(sendseg);
		return -1;
	}

	//  segSize=  ;
	if(send(network_conn, sendseg, sizeof(ptt_seg_t), 0) < 0) {
        free(sendseg);
		return -1;
	}

	if(send(network_conn, bufend, 2, 0) < 0) {
        free(sendseg);
		return -1;
	}
	free(sendseg);
}

int tracker_sendseg(int network_conn, ttp_seg_t* sendseg){
    char bufstart[2];
	char bufend[2];
    int segSize;
    
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufend[0] = '!';
	bufend[1] = '#';

	if (send(network_conn, bufstart, 2, 0) < 0) {
		return -1;
	}

	//  segSize=  ;
	if(send(network_conn, sendseg, sizeof(ttp_seg_t), 0) < 0) {
		return -1;
	}

	if(send(network_conn, bufend, 2, 0) < 0) {
		return -1;
	}
}

int peer_recvseg(int network_conn, ttp_seg_t* recvseg){
	char buf[sizeof(ttp_seg_t)+2];
	char c;
	int idx = 0;
	// state can be 0,1,2,3; 
	// 0 starting point 
	// 1 '!' received
	// 2 '&' received, start receiving segment
	// 3 '!' received,
	// 4 '#' received, finish receiving segment 
	int state = 0;
	while(recv(network_conn,&c,1,0)>0) {
		if (state == 0) {
		        if(c=='!')
				state = 1;
		}
		else if(state == 1) {
			if(c=='&') 
				state = 2;
			else
				state = 0;
		}
		else if(state == 2) {
			if(c=='!') {
				buf[idx]=c;
				idx++;
				state = 3;
			}
			else {
				buf[idx]=c;
				idx++;
			}
		}
		else if(state == 3) {
			if(c=='#') {
				buf[idx]=c;
				idx++;
				memcpy(recvseg,buf,idx-2);
				state = 0;
				idx = 0;
/*				if(seglost(segPtr)>0) {
                                	printf("seg lost!!!\n");
                                	continue;
                        	}
				if(checkchecksum(segPtr)<0) {
		                    printf("checksum error!\n");
                		    continue;
                		}*/
				return 1;
			}
			else if(c=='!') {
				buf[idx]=c;
				idx++;
			}
			else {
				buf[idx]=c;
				idx++;
				state = 2;
			}
		}
	}
	return -1;

}

int tracker_recvseg(int network_conn, ptt_seg_t* recvseg){
	char buf[sizeof(ptt_seg_t)+2];
	char c;
	int idx = 0;
	// state can be 0,1,2,3; 
	// 0 starting point 
	// 1 '!' received
	// 2 '&' received, start receiving segment
	// 3 '!' received,
	// 4 '#' received, finish receiving segment 
	int state = 0;
	while(recv(network_conn,&c,1,0)>0) {
		if (state == 0) {
		        if(c=='!')
				state = 1;
		}
		else if(state == 1) {
			if(c=='&') 
				state = 2;
			else
				state = 0;
		}
		else if(state == 2) {
			if(c=='!') {
				buf[idx]=c;
				idx++;
				state = 3;
			}
			else {
				buf[idx]=c;
				idx++;
			}
		}
		else if(state == 3) {
			if(c=='#') {
				buf[idx]=c;
				idx++;
				memcpy(recvseg,buf,idx-2);
				state = 0;
				idx = 0;
/*				if(seglost(segPtr)>0) {
                                	printf("seg lost!!!\n");
                                	continue;
                        	}
				if(checkchecksum(segPtr)<0) {
		                    printf("checksum error!\n");
                		    continue;
                		}*/
				return 1;
			}
			else if(c=='!') {
				buf[idx]=c;
				idx++;
			}
			else {
				buf[idx]=c;
				idx++;
				state = 2;
			}
		}
	}
	return -1;

}
