/**
 * 
 */
package com.dartsync;

import java.io.File;
import java.net.Socket;
import java.util.List;
import java.util.Vector;

/**
 * @author Vishal Gaurav
 *
 */
public class TrackerInfo {
	
	public String trackerAddress = null;
	private Socket socket = null;
	public int heartBeatInterval = -1;
	public int pieceLength = 0;
	private List<File> fileList = new Vector<File>();
	
	public TrackerInfo(String trackerAddress, Socket socket, int heartBeatInterval, int pieceLength) {
		super();
		this.trackerAddress = trackerAddress;
		this.socket = socket;
		this.heartBeatInterval = heartBeatInterval;
		this.pieceLength = pieceLength;
	}
	
	public synchronized Socket getSocket(){
		return socket;
	}
	public synchronized List<File> getFileList(){
		return fileList;
	}
}
