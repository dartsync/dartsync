/**
 * 
 */
package com.dartsync;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
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
	private Thread peerThread = null;
	private Thread trackerThread = null;
	
	private Boolean isClientRunning = false;
	private String trackerAddress;
	private File rootDir ;
	
	public Client(String trackerAddress, File rootDir){
		isClientRunning = new Boolean(true);
		this.trackerAddress = trackerAddress;
		this.rootDir = rootDir;
	}
	
	public void stopClient(){
		isClientRunning = false;
		try{
			if(trackerThread!=null){
				trackerThread.interrupt();
			}
			if(heartBeatThread!=null){
				heartBeatThread.interrupt();
			}
			if(peerThread!=null){
				peerThread.interrupt();
			}
			if(fileMonitor!=null){
				trackerThread.interrupt();
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
		final Client client = new Client(trackerAddress, rootDir);
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
	}
	
	public static TrackerInfo connectToTracker(String trackerAddress){
		TrackerInfo info = null;
		try{
			Socket socket = new Socket(trackerAddress, Constants.PORT_TRACKER);
			socket.setKeepAlive(true);
			socket.setSoTimeout(Constants.TIMEOUT);
			OutputStream os = socket.getOutputStream();
			InputStream is = socket.getInputStream();
			SegmentPeer segment = new SegmentPeer();
			segment.peer_ip = ByteBuffer.wrap(InetAddress.getLocalHost().getAddress()).getInt();
			segment.protocolLength = 126;
			segment.type = Constants.SIGNAL_REGISTER;
			segment.file_table_size = 0;
			byte[] segment_buf = segment.getBytes();
			os.write(segment_buf);
			System.out.println("Written " + segment_buf.length + "bytes -> " + new String(segment_buf));
			byte[] recv_seg = new byte[1024];
			is.read(recv_seg);
			ByteBuffer inputBuffer = ByteBuffer.wrap(recv_seg);
			SegmentTracker segTracker = new SegmentTracker();
			segTracker.interval = inputBuffer.getInt();
			System.out.println("Interval = " + segTracker.interval);
			segTracker.pieceLength = inputBuffer.getInt();
			System.out.println("PieceLength = " + segTracker.pieceLength);
			segTracker.file_table_size = inputBuffer.getInt();
			System.out.println("table size = " + segTracker.file_table_size);
			info = new TrackerInfo(socket, segTracker.interval, segTracker.pieceLength);
		}catch(IOException ex){
			System.out.println("IO Eception :- " + ex.getMessage());
			ex.printStackTrace();
		}
		return info;
	}
	
	public void startClient(){
		fileMonitor = new FileMonitor(rootDir, null, this);
		fileMonitor.start();
		try {
			if(heartBeatThread != null && heartBeatThread.isAlive()) heartBeatThread.join();
			if(fileMonitor != null && fileMonitor.isAlive()) fileMonitor.join();
			if(peerThread != null && peerThread.isAlive()) peerThread.join();
			if(trackerThread != null && trackerThread.isAlive())  trackerThread.join();
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
