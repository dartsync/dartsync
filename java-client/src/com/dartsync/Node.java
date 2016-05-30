package com.dartsync;

import java.util.ArrayList;
import java.util.List;

/**
 * 
 * @author Vishal Gaurav
 *
 */
public class Node {

	// the size of the file
	int size;
	// the name of the file
	String name = "";
	// the timestamp when the file is modified or created
	long timestamp;
	// pointer to build the linked list
	Node nextNode;
	// the number of peers that have this file
	int peernum;
	// for the file table on peers, it is the ip address of the peer
	// for the file table on tracker, it records the ip of all peers which has
	// the
	// newest edition of the file
	int peerip[] = new int[Constants.MAX_PEERS_NUM];

	public static List<Node> getNodes(String broadcastLine) {
		List<Node> fileNode = new ArrayList<Node>();
		try {
			String[] nodes = broadcastLine.split(",");
			if(nodes.length > 0){
				int fileNum = Integer.parseInt(nodes[0]);
				if(fileNum > 0){
					for(int i = 1 ; i < nodes.length ; i++){
						String nodeInfo = nodes[i];
						String[] nodeValues = nodeInfo.split(";");
						Node nodeObj = new Node();
						nodeObj.name = nodeValues[0];
						nodeObj.size = Integer.parseInt(nodeValues[1]);
						nodeObj.timestamp = Long.parseLong(nodeValues[2]);
						nodeObj.peernum = Integer.parseInt(nodeValues[3]);
						if(nodeObj.peernum > 0){
							String peerInfo = nodeValues[4];
							String[] peerIps = peerInfo.split("-");
							for(int j = 0 ; j < nodeObj.peernum && j < Constants.MAX_PEERS_NUM ; j++ ){
								nodeObj.peerip[j] = Integer.parseInt(peerIps[j]);
							}
						}
						fileNode.add(nodeObj);
					}
				}
			}
			
		} catch (Exception ex) {
			System.out.println("Exception in parsing boradcast :- " + ex.getMessage());
		}
		return fileNode;
	}
}
