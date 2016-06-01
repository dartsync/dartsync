package com.dartcync;

import java.io.File;
import java.io.PrintWriter;
import java.util.Map;
import java.util.Set;

public class FileMonitor {

	

	private File rootDir = null;
	private TrackerInfo trackerInfo = null;

	public FileMonitor(File root, TrackerInfo trackerSocket){
		this.trackerInfo = trackerSocket;
		this.rootDir = root;
		updateExistingFiles();
	}

	public void onFileDeleted(File filePath) {
		// TODO Auto-generated method stub
		System.out.println("onFileDeleted");
		if(isValidFile(filePath)){
			if (trackerInfo.getFileTable().containsKey(filePath.getName())){
				if( trackerInfo.getFileTable().get(filePath.getName()).fileStatus == TrackerInfo.FILE_STATUS_DELETED){
					// this file is deleted due to a sync from tracker so we should not send update back to tracker
					System.out.println("deleted File exists in file table :- " + filePath.getName());
				}else{
					System.out.println("A valid file deleted :- " + filePath.getName());
					sendFileBroadCast();
				}
				trackerInfo.getFileTable().remove(filePath.getName());
				trackerInfo.getFileList().remove(filePath);
			}		
		}
	}
		
	public void onFileCreated(File filePath){
		if(isValidFile(filePath)){
			System.out.println("onFileCreated " + filePath.toString());
		}else{
			System.out.println("onFileCreated Discarded " + filePath.toString());
		}
	}

	public void onFileUpdated(File filePath) {
		System.out.println("onFileUpdated " + filePath.toString());
		if (isValidFile(filePath)) {
			if (trackerInfo.getFileTable().containsKey(filePath.getName())) {
				if (trackerInfo.getFileTable().get(filePath.getName()).fileStatus == TrackerInfo.FILE_STATUS_DOWNLOADED) {
					sendFileBroadCast();
				} else {
					System.out.println("onFileUpdated downloading ... :- " + filePath.getName());
				}
			} else {
				System.out.println("New file detected :- " + filePath.getName());
				// this file should be added to file list as well as file table
				// and update broadcast should be sent
				trackerInfo.getFileList().add(filePath);
				trackerInfo.getFileTable().put(filePath.getName(), new TrackerInfo.FileInfo(filePath, TrackerInfo.FILE_STATUS_DOWNLOADED));
				sendFileBroadCast();
			}
		} else {
			System.out.println("onFileUpdated Discarded " + filePath.toString());
		}

	}
	
	private boolean isValidFile(File filePath){
		return !filePath.isDirectory() && !filePath.getName().startsWith(".") && !filePath.getName().endsWith("~");
	}

	private boolean isAnyFileDownloading(){
		synchronized (trackerInfo){
			Set<Map.Entry<String, TrackerInfo.FileInfo>> localFileSet = trackerInfo.getFileTable().entrySet();
			for(Map.Entry<String,TrackerInfo.FileInfo> entry : localFileSet){
					if(entry.getValue().fileStatus == TrackerInfo.FILE_STATUS_DOWNLOADING) {
						System.out.println("File still downloading :- " + entry.getKey());
						return true;
					}
			}
		}
		return false;
	}
	private void sendFileBroadCast() {
		new Thread() {
			@Override
			public void run() {
				try
				{	if(!isAnyFileDownloading()) {
						System.out.println("Send file broadcast");
						SegmentPeer peer = new SegmentPeer();
						peer.type = Constants.SIGNAL_FILE_UPDATE;
						peer.protocolLength = 0;
						peer.peer_ip = Client.getLocalIp();
						peer.fileList.addAll(trackerInfo.getFileList());
						String tcpString = peer.getTCPString();
						System.out.println("File update = " + tcpString);
						System.out.println("Sending to socket");
						PrintWriter pw = new PrintWriter(trackerInfo.getSocket().getOutputStream(), true);
						pw.println(tcpString);
						System.out.println("sent to socket");
					}else{
						System.out.println("File broadcast discarded :- downloading something " );
					}
				} catch (Exception ex)
				{
					System.out.println("Error in sending file update :- " + ex.getMessage());
				}
			}
		}.start();
	}

	public void updateExistingFiles() {
		File[] files = rootDir.listFiles();
		if(files != null ){
			for (File file : files) {
				if(!trackerInfo.getFileList().contains(file) && isValidFile(file)){
					System.out.println("Already Exisitng :- " + file.getName());
					trackerInfo.getFileList().add(file);
					trackerInfo.getFileTable().put(file.getName(), new TrackerInfo.FileInfo(file, TrackerInfo.FILE_STATUS_DOWNLOADED));
				}
			}
		}
		if(trackerInfo.getFileList().size() > 0){
			sendFileBroadCast();
		}
	}
	
}
