/**
 * 
 */
package com.dartsync;

import java.io.File;

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
		if(args!= null && args.length >= 2){
			trackerAddress = args[0];
			rootDir = new File(args[1]);
			rootDir.mkdir();
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
