/**
 * 
 */
package com.dartsync;

/**
 * @author Vishal Gaurav
 *
 */
public final class Constants {
	
	
	/**
	 * heart beat duration in seconds
	 */
	public static final int HEART_BEAT_DURATION = 1; 
	public static final int HEART_BEAT_DURATION_MILLIS = 1 * 1000;
	
	public static final int SIGNAL_HEART_BEAT = 1; 
	public static final int SIGNAL_REGISTER = 0; 
	public static final int SIGNAL_FILE_UPDATE = 2; 

	public static final int PORT_TRACKER = 4880;
	public static final int PORT_PEER_DOWNLOAD = 4881;
	
	public static final int TIMEOUT = 5000;
	
	public static final int RESERVED_LEN = 256;
	public static final int MAX_FILE_NUM = 128;
	public static final int MAX_PEERS_NUM  = 10 ;
	public static final int MAX_FILE_NAME_LEN = 256 ;
	public static final int  PIECE_LENGTH  = 1500 ;
	public static final int FILE_BUFFER_SIZE = 1024 ;


	
}
