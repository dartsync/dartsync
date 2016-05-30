/**
 * 
 */
package com.dartsync;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;

/**
 * @author Vishal Gaurav
 *
 */
public class HeartBeat extends Thread {
	
	private Socket trackerSocket = null;
	private int interval ;
	
	public HeartBeat(Socket tracerSocket, int interval){
		this.interval = interval;
		this.trackerSocket = tracerSocket;
	}
	
	@Override
	public void run() {
		try{
			while(trackerSocket != null && trackerSocket.isConnected()){
				//System.out.println("heartbeat thread sleeping");
				Thread.sleep(interval);
				sendHeartBeat();
			}
		}catch(InterruptedException ex){
			System.out.println("Heartbeat interrupted");
		} catch (IOException e) {
			// TODO Auto-generated catch blocktokens[]to
			e.printStackTrace();
		}
		
	}

	private void sendHeartBeat() throws IOException {
		SegmentPeer segment = new SegmentPeer();
		segment.type = Constants.SIGNAL_HEART_BEAT;
		PrintWriter pw = new PrintWriter(trackerSocket.getOutputStream(),true);
		pw.println(segment.getTCPString());
		//System.out.println("heartbeat sent");
	}
	
}

