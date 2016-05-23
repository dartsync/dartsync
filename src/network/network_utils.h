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

int get_server_socket_fD(int port, int maxConn);
int get_client_socket_fd(char* hostName, int portNumber);
unsigned long get_peer_address_l(int socketFD) ;
int get_peer_address(int socketFD, char* ip, char length) ;
unsigned long get_my_ip();


#endif /* NETWORK_NETWORK_UTILS_H_ */
