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
	
	public HeartBeat(Socket tracerSocket){
		this.trackerSocket = tracerSocket;
	}
	
	@Override
	public void run() {
		try{
			while(trackerSocket != null && trackerSocket.isConnected()){
				System.out.println("heartbeat thread sleeping");
				Thread.sleep(Constants.HEART_BEAT_DURATION_MILLIS);
				sendHeartBeat();
			}
		}catch(InterruptedException ex){
			System.out.println("Heartbeat interrupted");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

	private void sendHeartBeat() throws IOException {
		SegmentPeer segment = new SegmentPeer();
		segment.type = Constants.SIGNAL_HEART_BEAT;
		PrintWriter pw = new PrintWriter(trackerSocket.getOutputStream(),true);
		pw.println(segment.getTCPString());
		System.out.println("heartbeat sent");
	}
	
}

