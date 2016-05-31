package com.dartsync;

import java.io.File;
import java.util.ArrayList;
import java.util.List;


public class SegmentPeer {
	
	public static final int MAX_BUFFER_SIZE = 1024;
	
	public int protocolLength; // 4
	// protocol name
	public char[] protocolName = new char[128]; // 32 
	// packet type : register, keep alive, update file table
	public int type; // 4
	// reserved space, you could use this space for your convenient, 8 bytes by default
	public char[] reserved = new char[Constants.RESERVED_LEN]; // 64
	// the peer ip address sending this packet
	public int peer_ip; // 4
	// listening port number in p2p
	public int port; // 4
	// the number of files in the local file table -- optional
	// file table of the client -- your own design
	public List<File> fileList = new ArrayList<>();
	
	public String getTCPString(){
		/**
		 * 1. type
		 * 2. protocol len
		 * 3. ip
		 * 4. file table size
		 */
		StringBuilder resultBuffer  = new StringBuilder(type+","+ protocolLength+","+ peer_ip+","+ fileList.size());
		if(fileList.size() > 0){
			for(int i = 0 ; i < fileList.size() ; i++){
				File file = fileList.get(i);
				resultBuffer.append("," + file.getName() + "," + file.length() + "," + file.lastModified());
			}
		}
		return resultBuffer.toString();
	}
	
	
}
