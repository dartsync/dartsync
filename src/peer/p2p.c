#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <assert.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <unistd.h>
#include "../utils/constants.h"
#include "p2p.h"
#include "peer.h"

int send_p2p_seg(int network_conn, peer2peer_seg* send_seg){
	return send(network_conn,send_seg,sizeof(peer2peer_seg),0);
}


/**
 * will return the next chunk size to be uploaded
 */
int get_chunk_size(int total_sent, int size) {
	int chunk_size = 0;
	int remaining = size - total_sent;
	if (remaining < FILE_BUFFER_SIZE) {
		chunk_size = remaining;
	} else {
		chunk_size = FILE_BUFFER_SIZE;
	}
	return chunk_size;
}

int upload_file(int sock_fd, char* filename, int offset, int size) {
	char* directory=get_dir();
	char full_path[256];
	sprintf(full_path,"%s/%s",directory,filename);
	FILE *upload_file = fopen(full_path, "r");
	if (upload_file == NULL || fseek(upload_file, offset, SEEK_SET) != 0) {
		printf("File requested is not available :- %s , file not found or seek error \n", filename);
		return FALSE;
	}
	printf("sending file to peer");
	int total_sent = 0;
	char file_buffer[FILE_BUFFER_SIZE];
	while (total_sent < size) {
		int chunk_size = get_chunk_size(total_sent, size);
		bzero(file_buffer, FILE_BUFFER_SIZE);
		int bytes_read = fread(file_buffer, sizeof(char), chunk_size, upload_file);
		if (bytes_read >= 0 && send(sock_fd, file_buffer, bytes_read, 0) >= 0) {
			total_sent += bytes_read;
		} else {
			printf("Error in reading from file chunk_size = %d \n", chunk_size);
			fclose(upload_file);
			close(sock_fd);
			return FALSE;
		}
	}
	fclose(upload_file);
	close(sock_fd);
	return TRUE;
}

int parse_upload_req(char* buffer, peer2peer_seg *seg, char* delimiter){
	char *token ;
	/**
	 * 1. filename
	 * 2. offset
	 * 3. piecelength
	 */
	token = strtok(buffer,delimiter) ;
	printf("%s\n",token);
	if(token == NULL) return FALSE;
	memcpy(seg->file_name,token,sizeof(token));
	if((token = strtok(NULL,delimiter)) == NULL) return FALSE ;
	printf("%s\n",token);
	seg->start_idx = atoi(token);
	if((token = strtok(NULL,delimiter)) == NULL) return FALSE ;
	printf("%s\n",token);
	seg->piece_len = atoi(token);
	return TRUE;
}

void *p2p_upload_diff(void *arg){
	int sock_fd = *(int * )arg;
	char buffer[PIECE_LENGTH] ;
	if(sock_fd < 0 ){
		printf("Error in p2p_upload_diff socket fd = %d\n", sock_fd);
		pthread_exit(NULL);
	}
	if(recv(sock_fd,buffer,sizeof(PIECE_LENGTH),0) < 0){
		close(sock_fd);
		printf("Error in p2p_upload_diff recv ERROR  \n");
		pthread_exit(NULL);
	}
	printf("p2p_upload_diff received = %s \n", buffer);
	peer2peer_seg *header = (peer2peer_seg * ) malloc(sizeof(peer2peer_seg));
	parse_upload_req(buffer,header,",");
	if( upload_file(sock_fd,header->file_name,header->start_idx, header->piece_len)){
		printf("upload file success diff \n");
	}else{
		printf("upload file failed diff \n");
	}
	free(header);
	return NULL;

}

void *p2p_upload(void* arg){
	char filename[256];
	char* directory=get_dir();

	fflush(stdout);
	int *sock_fd_p = (int *) arg;
	int sock_fd = *sock_fd_p;
	if(sock_fd < 0 ){
		printf("Error in p2p_upload socket fd = %d\n", sock_fd);
		pthread_exit(NULL);
	}
	peer2peer_seg *header = (peer2peer_seg * ) malloc(sizeof(peer2peer_seg));
	bzero(header, sizeof(peer2peer_seg));
	if(recv(sock_fd,header,sizeof(peer2peer_seg),0) < 0){
		close(sock_fd);
	    free(header);
		printf("Error in p2p_upload recv ERROR  \n");
		pthread_exit(NULL);
	}

	sprintf(filename,"%s/%s",directory,header->file_name);

	int offset = header->start_idx;
	int size = header->piece_len;
	printf("uploading file %s requested size = %d & offset = %d \n",filename, size, offset);
	FILE *upload_file = fopen(filename, "r");
	if (upload_file == NULL || fseek(upload_file,offset,SEEK_SET) != 0)  {
	    free(header);
	    printf("File requested is not available :- %s , file not found or seek error \n", filename);
	}
	printf("sending file to peer");

	int total_sent = 0 ;
	char file_buffer[FILE_BUFFER_SIZE];
	while(total_sent < size){
		int chunk_size = get_chunk_size(total_sent,size);
		bzero(file_buffer, FILE_BUFFER_SIZE);
        
		int bytes_read = fread(file_buffer, sizeof(char), chunk_size, upload_file);
		if(bytes_read >= 0 && send(sock_fd , file_buffer, bytes_read,0) >= 0){
			total_sent += bytes_read ;
		}else{
			printf("Error in reading from file chunk_size = %d \n",chunk_size);
			fclose(upload_file);
			close(sock_fd);
		    free(header);
			pthread_exit(NULL);
		}
	}

	fclose(upload_file);
    	free(header);
	close(sock_fd);
	fflush(stdout);
	return NULL;
}
