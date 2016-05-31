/**
 * 
 */
package com.dartsync;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;

/**
 * @author Vishal Gaurav
 *
 */
public class BroadcastThread extends Thread {
	private TrackerInfo trackerInfo = null;
	
	public BroadcastThread(TrackerInfo trackerInfo){
		this.trackerInfo = trackerInfo;
	}
	
	@Override
	public void run(){
		try{
			while(trackerInfo.getSocket() !=null && trackerInfo.getSocket().isConnected()){
				BufferedReader br = new BufferedReader(new InputStreamReader(trackerInfo.getSocket().getInputStream()));
				System.out.println("waiting for broadcast");
				String broadCast = br.readLine();
				System.out.println("Received broadcast :- " + broadCast);
				if(broadCast != null){
					List<Node> nodeList = Node.getNodes(broadCast);
					for (Node node : nodeList) {
						System.out.println("File name = " + node.name + " File size :- " + node.size + " Peer num = " + node.peernum);
					}
				}
			}
		}catch(Exception ex){
			System.out.println("Exception in Broadcast Thread " + ex.getMessage());
		}
	}
}
