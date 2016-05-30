package com.dartsync;
/**
 * 
 * @author Vishal Gaurav
 *
 */
public class Node {
	
	//the size of the file
	int size;
	//the name of the file
	char[] name = new char[Constants.MAX_FILE_NAME_LEN];
	//the timestamp when the file is modified or created
	long timestamp;
	//pointer to build the linked list
	Node nextNode;
	//the number of peers that have this file
	int peernum;
	//for the file table on peers, it is the ip address of the peer
	//for the file table on tracker, it records the ip of all peers which has the
	//newest edition of the file
	int peerip[] = new int[Constants.MAX_PEERS_NUM];
}
