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

	public Socket socket = null;
	public int heartBeatInterval = -1;
	public int pieceLength = 0;
	
	public TrackerInfo(Socket socket, int heartBeatInterval, int pieceLength) {
		super();
		this.socket = socket;
		this.heartBeatInterval = heartBeatInterval;
		this.pieceLength = pieceLength;
	}

}
