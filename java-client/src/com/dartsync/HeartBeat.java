/**
 * 
 */
package com.dartsync;

import java.io.IOException;
import java.io.PrintWriter;

/**
 * @author Vishal Gaurav
 *
 */
public class HeartBeat extends Thread {
	
	private TrackerInfo trackerSocket = null;
	private int interval ;
	
	public HeartBeat(TrackerInfo tracerSocket, int interval){
		this.interval = interval;
		this.trackerSocket = tracerSocket;
	}
	
	@Override
	public void run() {
		try{
			while(trackerSocket.getSocket() != null && trackerSocket.getSocket().isConnected()){
				//System.out.println("heartbeat thread sleeping");
				Thread.sleep(interval * 1000);
				sendHeartBeat();
			}
		}catch(InterruptedException ex){
			System.out.println("Heartbeat interrupted");
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}

	private void sendHeartBeat() throws IOException {
		SegmentPeer segment = new SegmentPeer();
		segment.type = Constants.SIGNAL_HEART_BEAT;
		segment.peer_ip = Client.getLocalIp();
		PrintWriter pw = new PrintWriter(trackerSocket.getSocket().getOutputStream(),true);
		pw.println(segment.getTCPString());
	}
	
}

