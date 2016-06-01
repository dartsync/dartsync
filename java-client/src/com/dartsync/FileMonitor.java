package com.dartsync;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Path;
import java.nio.file.StandardWatchEventKinds;
import java.nio.file.WatchEvent;
import java.nio.file.WatchEvent.Kind;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;

import com.dartsync.TrackerInfo.FileInfo;

public class FileMonitor extends Thread {
	
	public interface FileMonitorListener{
		void onFileCreated(File filePath);
		void onFileDeleted(File filePath);
		void onFileUpdated(File filePath);
	}
	
	public static File getDefaultRootDir(){
		String rootPath =  "file_dir";
		File rootDir = new File(rootPath);
		rootDir.mkdir();
		System.out.println(rootDir.getAbsolutePath());
		return rootDir;
	}
	
	private File rootDir = null;
	private FileMonitorListener mListener = null;
	private TrackerInfo trackerInfo = null;
	
	public FileMonitor(File root, TrackerInfo trackerSocket, FileMonitorListener fileListener){
		this.trackerInfo = trackerSocket;
		this.mListener = fileListener;
		this.rootDir = root;
	}
	
	private void updateFileChanges(WatchEvent<?> event){
		Kind<?> kind = event.kind();
        if (kind.equals(StandardWatchEventKinds.ENTRY_CREATE)) {
            Path pathCreated = (Path) event.context();
            if(mListener != null){
            	mListener.onFileCreated(pathCreated.toFile());
            }
            onFileCreated(pathCreated.toFile());
        } else if (kind.equals(StandardWatchEventKinds.ENTRY_DELETE)) {
            Path pathDeleted = (Path) event.context();
            if(mListener != null){
            	mListener.onFileDeleted(pathDeleted.toFile());
            }
            //System.out.println("File deleted: " + pathDeleted);
            onFileDeleted(pathDeleted.toFile());
        } else if (kind.equals(StandardWatchEventKinds.ENTRY_MODIFY)) {
            Path pathModified = (Path) event.context();
            if(mListener != null){
            	mListener.onFileUpdated(pathModified.toFile());
            }
            onFileUpdated(pathModified.toFile());
            //System.out.println("File updated: "+ pathModified);
        }
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
				trackerInfo.getFileTable().put(filePath.getName(), new FileInfo(filePath, TrackerInfo.FILE_STATUS_DOWNLOADED));
				sendFileBroadCast();
			}
		} else {
			System.out.println("onFileUpdated Discarded " + filePath.toString());
		}

	}
	
	private boolean isValidFile(File filePath){
		return !filePath.isDirectory() && !filePath.getName().startsWith(".") && !filePath.getName().endsWith("~");
	}
	
	@Override
	public void run(){
		System.out.println("File monitor started ...");
		Path path = rootDir.toPath();
		WatchService watchService = null;
		WatchKey watchKey = null;

		try {
			updateExistingFiles();
			if(trackerInfo.getFileList().size() > 0){
				sendFileBroadCast();
			}
			watchService = path.getFileSystem().newWatchService();
			path.register(watchService, StandardWatchEventKinds.ENTRY_CREATE,
                    StandardWatchEventKinds.ENTRY_MODIFY, StandardWatchEventKinds.ENTRY_DELETE);
			while(true){
                watchKey = watchService.take(); // this call is blocking until events are present
                // poll for file system events on the WatchKey
                for (final WatchEvent<?> event : watchKey.pollEvents()) {
                    updateFileChanges(event);
                }
                // if the watched directed gets deleted, get out of run method
                if (!watchKey.reset()) {
                    System.out.println("directory deleted");
                    watchKey.cancel();
                    watchService.close();
                    break;
                }
			}
		}
		catch (InterruptedException ex){
			releaseWatch(watchKey, watchService);
			ex.printStackTrace();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		
	}
	
	private void sendFileBroadCast() {
		try {
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
		} catch (Exception ex) {
			System.out.println("Error in sending file update :- " + ex.getMessage());
		}
	}

	private void updateExistingFiles() {
		File[] files = rootDir.listFiles();
		if(files != null ){
			for (File file : files) {
				if(!trackerInfo.getFileList().contains(file) && isValidFile(file)){
					System.out.println("Already Exisitng :- " + file.getName());
					trackerInfo.getFileList().add(file);
					trackerInfo.getFileTable().put(file.getName(), new FileInfo(file, TrackerInfo.FILE_STATUS_DOWNLOADED));
				}
			}
		}
		
	}

	private void releaseWatch(WatchKey key, WatchService service) {
		try {
			if (key != null) {
				key.cancel();
			}
			if (service != null) {
				service.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
}
