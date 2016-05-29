package com.dartsync;

import java.nio.ByteBuffer;


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
	public int file_table_size; // 4
	// file table of the client -- your own design
	public Node[] fileTable = new Node[Constants.MAX_FILE_NUM]; // 
	
	public String getTCPString(){
		/**
		 * 1. type
		 * 2. protocol len
		 * 3. ip
		 * 4. file table size
		 */
		return type+","+
		 	   protocolLength+","+
		 	   peer_ip+","+
		 	   file_table_size;
	}
	
	public byte[] getBytes(){
		
		ByteBuffer buffer = ByteBuffer.allocate(MAX_BUFFER_SIZE);
		buffer.putInt(protocolLength);
		buffer.put(new String(protocolName).getBytes());
		buffer.putInt(type);
		buffer.put(new String(reserved).getBytes());
		buffer.putInt(peer_ip);
		buffer.putInt(port);
		buffer.putInt(file_table_size);
		return buffer.array();
		
	}
	
}
