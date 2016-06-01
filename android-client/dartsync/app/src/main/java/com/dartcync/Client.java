/**
 * 
 */
package com.dartcync;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.math.BigInteger;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

/**
 * @author Vishal Gaurav
 *
 */
public class Client{
	
	public static final String TRACKER_ADDRESS = "tahoe.cs.dartmouth.edu";
	
	private Thread heartBeatThread = null;
	private FileMonitor fileMonitor = null;
	private Thread fileUploadThread = null;
	private Thread broadcastThread = null;
	
	private Boolean isClientRunning = false;
	private TrackerInfo trackerInfo;
	private File rootDir ;
	
	public Client(TrackerInfo info, File rootDir){
		isClientRunning = new Boolean(true);
		this.rootDir = rootDir;
		this.trackerInfo = info;

	}
	
	public boolean isClientRunning(){
		return isClientRunning;
	}

	public void onFileUpdate(File file){
		if(fileMonitor!=null){
			fileMonitor.onFileUpdated(file);
		}
	}
	public void onFileDelete(File file){
		if(fileMonitor!=null){
			fileMonitor.onFileDeleted(file);
		}
	}
	public void onFileCreate(File file){
		if(fileMonitor!=null){
			fileMonitor.onFileCreated(file);
		}
	}
	public void stopClient(){
		isClientRunning = false;
		try{
			if(broadcastThread!=null){
				broadcastThread.interrupt();
			}
			if(heartBeatThread!=null){
				heartBeatThread.interrupt();
			}
			if(fileUploadThread!=null){
				fileUploadThread.interrupt();
			}
			if(fileMonitor!=null){
				broadcastThread.interrupt();
			}
		}catch(Exception ex){
			ex.printStackTrace();
		}
	}

	public static final int getLocalIp() throws UnknownHostException{
		return ByteBuffer.wrap(InetAddress.getLocalHost().getAddress()).getInt();
	}
	
	public static final InetAddress getInetAddress(int ipAddress) throws IOException {
		byte[] bytes = BigInteger.valueOf(ipAddress).toByteArray();
		return InetAddress.getByAddress(bytes);
	}

	public static TrackerInfo connectToTracker(String trackerAddress){
		TrackerInfo info = null;
		try{
			Socket socket = new Socket(trackerAddress, Constants.PORT_TRACKER);
			socket.setKeepAlive(true);
			PrintWriter pw = new PrintWriter(socket.getOutputStream(),true);
			BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			SegmentPeer segment = new SegmentPeer();
			segment.peer_ip = ByteBuffer.wrap(InetAddress.getLocalHost().getAddress()).getInt();
			segment.protocolLength = 126;
			segment.type = Constants.SIGNAL_REGISTER;
			String sendMsg = segment.getTCPString();
			pw.println(sendMsg);
			String recvMessage = br.readLine();
			String[] chunks = recvMessage.split(",");
			System.out.println(recvMessage);
			SegmentTracker segTracker = new SegmentTracker();
			if(chunks.length >= 1)
					segTracker.interval = Integer.parseInt(chunks[0]);
			if(chunks.length >= 2)
				segTracker.pieceLength = Integer.parseInt(chunks[1]);
			if(chunks.length >= 3)
				segTracker.pieceLength = Integer.parseInt(chunks[2]);
			System.out.println("Interval = " + segTracker.interval);
			System.out.println("PieceLength = " + segTracker.pieceLength);
			System.out.println("table size = " + segTracker.file_table_size);
			info = new TrackerInfo(trackerAddress,socket, segTracker.interval, segTracker.pieceLength);
		}catch(IOException ex){
			System.out.println("IO Exception :- " + ex.getMessage());
			ex.printStackTrace();
		}
		return info;
	}
	
	public void startClient(){
		fileMonitor = new FileMonitor(rootDir, trackerInfo);
		heartBeatThread = new HeartBeat(trackerInfo,trackerInfo.heartBeatInterval);
		heartBeatThread.start();
		fileUploadThread = new FileUploader(rootDir);
		fileUploadThread.start();
		broadcastThread = new BroadcastThread(trackerInfo,rootDir);
		broadcastThread.start();
	}
	
	
}
