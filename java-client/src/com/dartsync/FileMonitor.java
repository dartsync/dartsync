package com.dartsync;

import java.io.File;
import java.io.IOException;
import java.net.Socket;
import java.nio.file.Path;
import java.nio.file.StandardWatchEventKinds;
import java.nio.file.WatchEvent;
import java.nio.file.WatchEvent.Kind;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;

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
	private Socket trackerSocket = null;
	
	public FileMonitor(File root, Socket trackerSocket, FileMonitorListener fileListener){
		this.trackerSocket = trackerSocket;
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
            System.out.println("File created: " + pathCreated);
        } else if (kind.equals(StandardWatchEventKinds.ENTRY_DELETE)) {
            Path pathDeleted = (Path) event.context();
            if(mListener != null){
            	mListener.onFileDeleted(pathDeleted.toFile());
            }
            System.out.println("File deleted: " + pathDeleted);
        } else if (kind.equals(StandardWatchEventKinds.ENTRY_MODIFY)) {
            Path pathModified = (Path) event.context();
            if(mListener != null){
            	mListener.onFileUpdated(pathModified.toFile());
            }
            System.out.println("File updated: "+ pathModified);
        }
	}
	
	@Override
	public void run(){
		System.out.println("File monitor started ...");
		Path path = rootDir.toPath();
		WatchService watchService = null;
		WatchKey watchKey = null;

		try {
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
