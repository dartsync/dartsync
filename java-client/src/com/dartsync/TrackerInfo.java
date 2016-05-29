/**
 * 
 */
package com.dartsync;

import java.net.Socket;

/**
 * @author Vishal Gaurav
 *
 */
public class TrackerInfo {
	
	public String trackerAddress = null;
	public Socket socket = null;
	public int heartBeatInterval = -1;
	public int pieceLength = 0;
	
	public TrackerInfo(String trackerAddress, Socket socket, int heartBeatInterval, int pieceLength) {
		super();
		this.trackerAddress = trackerAddress;
		this.socket = socket;
		this.heartBeatInterval = heartBeatInterval;
		this.pieceLength = pieceLength;
	}

}
