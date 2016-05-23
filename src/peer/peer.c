/*
 * peer.c
 *
 *  Created on: May 23, 2016
 *      Author: Vishal Gaurav
 */
#define TRACKER_ADDRESS_LENGTH 100

char tracker_host_name[100];

void start_peer(char *argv[]){
	//TODO fetch ip_address and root dir from argv.
	input_string("Enter the IP address of tracker (max 100 chars ) \n",tracker_host_name,TRACKER_ADDRESS_LENGTH);
}

void stop_peer(){

}

