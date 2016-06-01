/**
 * 
 */
package com.dartsync;

import java.io.File;
import java.net.Socket;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Vector;

/**
 * @author Vishal Gaurav
 *
 */
public class TrackerInfo {
	
	public static final int FILE_STATUS_DOWNLOADING = 1; 
	public static final int FILE_STATUS_DOWNLOADED = 2; 
	public static final int FILE_STATUS_UPDATING = 3;
	public static final int FILE_STATUS_DELETED = 4;
			
	public static class FileInfo{
		public File file;
		public int fileStatus;
		public FileInfo(File file, int fileStatus) {
			super();
			this.file = file;
			this.fileStatus = fileStatus;
		}
		
		
	}
	
	public String trackerAddress = null;
	private Socket socket = null;
	public int heartBeatInterval = -1;
	public int pieceLength = 0;
	private List<File> fileList = new Vector<File>();
	private Hashtable<String,FileInfo> fileMap = new Hashtable<>();
	
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
	public synchronized Map<String,FileInfo> getFileTable(){
		return fileMap;
	}
}
