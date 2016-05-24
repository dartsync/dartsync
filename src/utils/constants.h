/*
 * constants.c
 *
 *  Created on: May 23, 2016
 *      Author: vishal gaurav
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define TRUE 1
#define FALSE 0

/*port to which peer connect to tracker*/
#define TRACKER_PORT 4877

/*port to which peer should connect when uploading to other peer*/
#define PEER_UPLOAD_PORT 4878

/*port to which peer should connect when downloading to other peer*/

#define PEER_DOWNLOAD_PORT 4879

#define MAX_FILE_NUM 128

#define MAX_FILE_NAME_LEN 128

#define PEER_PORT 3356

#define RESERVED_LEN 256

#define COMMUNICATION_PORT 2341

#define HEARTBEAT_INTERVAL 1

#define PIECE_LENGTH 1500

#define MAX_PEERS_NUM 10

#define IP_LEN 16


#endif
