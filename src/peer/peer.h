#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <assert.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <unistd.h>

int send_filetable();

void start_peer(char *argv[]);
void start_peer_in_test();
char* get_dir();

void lock_file_table();

void unlock_file_table();

void lock_download_table();

void unlock_download_table();
