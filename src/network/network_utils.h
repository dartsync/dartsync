/*
 ============================================================================
 Name        : network_utils.h
 Author      : Vishal Gaurav
 Version     : 1.0
 Copyright   : CS60 dartsync project
 Description : utility for network operations
 ============================================================================
 */

#ifndef NETWORK_NETWORK_UTILS_H_
#define NETWORK_NETWORK_UTILS_H_

int get_server_socket_fd(int port, int maxConn);
int get_client_socket_fd(char* hostName, int portNumber);
unsigned long get_peer_address_l(int socketFD) ;
int get_client_socket_fd_ip(unsigned int ip_address, int portNumber);
int get_peer_address(int socketFD, char* ip, char length) ;
unsigned long get_my_ip();
unsigned int get_ip_address_hostname(char* hostname);


#endif /* NETWORK_NETWORK_UTILS_H_ */
