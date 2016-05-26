/*
 ============================================================================
 Name        : peer_runnner.c
 Author      : Vishal Gaurav
 Version     :
 Copyright   : CS60 dartsync project
 Description : this file will contain the main function for peer
 ============================================================================
 */
#include "./peer/peer.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	puts("!!!Hello World PEER!!!");
	if(argc > 2){
		start_peer(argv);
	}else{
		printf("tracker IP address and root directory are required as input parameters \n starting in test mode\n");
		start_peer_in_test();
	}
	return EXIT_SUCCESS;
}
