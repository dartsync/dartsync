/*
 ============================================================================
 Name        : network_utils.c
 Author      : Vishal Gaurav
 Version     : 1.0
 Copyright   : CS60 dartsync project
 Description : utility for network operations
 ============================================================================
 */
#define _DEFAULT_SOURCE
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
#include "network_utils.h"
#include "../utils/constants.h"
const int endian_check = 1;
#define is_bigendian() ( (*(char*)&endian_check) == 0 )

int get_server_socket_fd(int port, int maxConn){
	int conn_listen_fd = -1;
	// create socket 
	conn_listen_fd = socket (AF_INET, SOCK_STREAM, 0);
	if(conn_listen_fd < 0)
	{
		printf("Error creating new socket in get_server_socket_fD \n");
		return -1;
	}
	struct sockaddr_in node_addr; // server
	bzero(&node_addr,sizeof(node_addr));
	node_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	node_addr.sin_family = AF_INET;
    node_addr.sin_port = htons(port);
    int yes = 1;
	if ( setsockopt(conn_listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
    	close(conn_listen_fd);
    	printf("setsockopt error in binding ");
    	return -1;
	}
	// BIND socket
	if(bind(conn_listen_fd, (struct sockaddr *)&node_addr, sizeof(node_addr)) < 0) {
		close(conn_listen_fd);
        printf("Error in binding to socket %d \n",conn_listen_fd);
        return -1;
    }
    
    if (listen(conn_listen_fd, maxConn) < 0) { // max process to connect is 1
    	close(conn_listen_fd);
        printf("Error in binding to socket %d \n",conn_listen_fd);
        return -1;
    }
    fflush(stdout);
    return conn_listen_fd;
}

int get_client_socket_fd(char* hostName, int portNumber){
	struct hostent *serverAddress;
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	if(socketFD < 0 ){
		printf("Error in creating a new socket\n");
		return socketFD;
	}
	serverAddress = gethostbyname(hostName);
	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
    bcopy((char *)serverAddress->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, serverAddress->h_length);
    server_addr.sin_port = htons(portNumber);
    if(connect(socketFD,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        printf("connection failed to %s",hostName);
        close(socketFD);
        return -1;
    }
    return socketFD;
}

int get_client_socket_fd_ip(unsigned int ip_address, int portNumber){
	fflush(stdout);
	printf("get_client_socket_fd_ip \n");
	struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
	struct in_addr in;
	if(is_bigendian()){
		in.s_addr = htonl((unsigned int)ip_address);
	}else{
		in.s_addr = (unsigned int)ip_address;
	}
	char* ip = inet_ntoa(in);
	printf("requesting connection from ip :- %s and port %d \n" , ip, portNumber);
	return get_client_socket_fd(ip,portNumber);
}
int get_client_socket_fd_ip_diff(unsigned int ip_address, int portNumber){
	fflush(stdout);
	printf("get_client_socket_fd_ip \n");
	struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
	struct in_addr in;
	in.s_addr = htonl((unsigned int)ip_address);
	char* ip = inet_ntoa(in);
	printf("requesting connection from ip :- %s and port %d \n" , ip, portNumber);
	return get_client_socket_fd(ip,portNumber);
}

unsigned long get_my_ip(){
	char hostname[100];
	gethostname(hostname,sizeof(hostname)); // get the ip address of local machine
	struct hostent *hostInfo;
  	hostInfo = gethostbyname(hostname);
  	if(!hostInfo) {
  		printf("error in getting host name from string for %s \n", hostname);
  		return -1;
  	}
  	struct sockaddr_in servaddr;
  	memcpy((char *) &servaddr.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
  	unsigned long ipAddr = ntohl(servaddr.sin_addr.s_addr); // network order to host byte order
  	return ipAddr;
}


/* returns unsigned long type ip of peer from a socket */
unsigned long get_peer_address_l(int socketFD) {
    char ip_address[15];
    get_peer_address(socketFD,ip_address,15);
    return inet_addr(ip_address);
}
unsigned int get_ip_address_hostname(char* hostname)
{
  struct hostent *hostInfo;
  hostInfo = gethostbyname(hostname);
  if(!hostInfo) {
  	printf("error in getting host name from string for %s \n", hostname);
  	return -1;
  }
  struct sockaddr_in servaddr;
  memcpy((char *) &servaddr.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
//  printf("ip_Address = %s ");
  printf("ip_Address = %d ", servaddr.sin_addr.s_addr);
  return servaddr.sin_addr.s_addr;
}
/*
 * returns peer ip address for a socketFD
 */
int get_peer_address(int socketFD, char* ip, char length) {
    int result = -1;
    if(socketFD > 0){
	    struct sockaddr_in addr;
	    socklen_t addr_size = sizeof(struct sockaddr_in);
	    result = getpeername(socketFD, (struct sockaddr *)&addr, &addr_size);
	    bzero(ip, length);
	    strcpy(ip, inet_ntoa(addr.sin_addr));
	}
    return result;
}
/**
 * return TRUE if ipaddress is valid
 */
int is_ip_valid(char* ip_address, struct sockaddr_in *addr) {
    return inet_pton(AF_INET, ip_address, &addr->sin_addr) > 0 ? TRUE : FALSE  ;
}
