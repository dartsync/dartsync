
int get_server_socket_fD(int port, int maxConn);
int get_client_socket_fd(char* hostName, int portNumber);
unsigned long get_peer_address_l(int socketFD) ;
int get_peer_address(int socketFD, char* ip, char length) ;
unsigned long get_my_ip();