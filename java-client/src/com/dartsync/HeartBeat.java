/**
 * 
 */
package com.dartsync;

import java.io.IOException;
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
		trackerSocket.sendUrgentData(Constants.SIGNAL_HEART_BEAT);
		System.out.println("heartbeat sent");
	}
	
}

