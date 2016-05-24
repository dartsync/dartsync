
#ifndef tracker_h
#define tracker_h

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/utsname.h>
#include <assert.h>

#include "../utils/constants.h"
#include "../utils/seg.h"
#include "peertable.h"

void* monitor_alive();

void* listen_handshake(void* arg);

void tracker_stop();

int start_tracker();

#endif /* tracker_h */
