/**
 * 
 */
package com.dartsync;

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

import com.dartsync.FileMonitor.FileMonitorListener;

/**
 * @author Vishal Gaurav
 *
 */
public class Client implements FileMonitorListener{
	
	public static final String TRACKER_ADDRESS = "tahoe.cs.dartmouth.edu";
	
	private Thread heartBeatThread = null;
	private Thread fileMonitor = null;
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
	
	/**
	 * @param args :- it should have 2 arguments ip address of the tracker and root directory to watch
	 */
	public static void main(String[] args) {
		String trackerAddress = TRACKER_ADDRESS ;
		File rootDir = null ;
		if(args!= null){
			if(args.length >=2){
				rootDir = new File(args[1]);
				rootDir.mkdir();
				trackerAddress = args[0];
			}else if(args.length >= 1){
				trackerAddress = args[0];
			}else{
				rootDir = FileMonitor.getDefaultRootDir(); 
			}
		}else{
			rootDir = FileMonitor.getDefaultRootDir(); 
		}
		TrackerInfo info = connectToTracker(trackerAddress);
		if(info != null){
			final Client client = new Client(info, rootDir);
			Runtime.getRuntime().addShutdownHook(new Thread()
	        {
	            @Override
	            public void run()
	            {
	            	System.out.println("shutdown called");
	                if(client!=null){
	                	client.stopClient();
	                }
	            }
	        });
			System.out.println("starting client");
			client.startClient();
		}else{
			System.out.println("Tracker not available ");
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
			System.out.println("IO Eception :- " + ex.getMessage());
			ex.printStackTrace();
		}
		return info;
	}
	
	public void startClient(){
		fileMonitor = new FileMonitor(rootDir, trackerInfo, this);
		fileMonitor.start();
		heartBeatThread = new HeartBeat(trackerInfo,trackerInfo.heartBeatInterval);
		heartBeatThread.start();
		fileUploadThread = new FileUploader(rootDir);
		fileUploadThread.start();
		broadcastThread = new BroadcastThread(trackerInfo,rootDir);
		broadcastThread.start();
		try {
			if(heartBeatThread != null && heartBeatThread.isAlive()) heartBeatThread.join();
			if(fileMonitor != null && fileMonitor.isAlive()) fileMonitor.join();
			if(fileUploadThread != null && fileUploadThread.isAlive()) fileUploadThread.join();
			if(broadcastThread != null && broadcastThread.isAlive())  broadcastThread.join();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	@Override
	public void onFileCreated(File filePath) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onFileDeleted(File filePath) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onFileUpdated(File filePath) {
		// TODO Auto-generated method stub
		
	}
	
	
}
