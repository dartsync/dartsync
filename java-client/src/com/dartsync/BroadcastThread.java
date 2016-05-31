/**
 * 
 */
package com.dartsync;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.dartsync.TrackerInfo.FileInfo;

/**
 * @author Vishal Gaurav
 *
 */
public class BroadcastThread extends Thread {
	private TrackerInfo trackerInfo = null;
	private File rootDir = null;
	
	public BroadcastThread(TrackerInfo trackerInfo, File rootDir){
		this.trackerInfo = trackerInfo;
		this.rootDir = rootDir;
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
					syncRootDirectory(nodeList);
				}
			}
		}catch(Exception ex){
			System.out.println("Exception in Broadcast Thread " + ex.getMessage());
		}
	}
	
	private void syncRootDirectory(List<Node> nodeList) {
		HashSet<String> allFileMap = new HashSet<>();
		for (Node node : nodeList) {
			System.out.println("File name = " + node.name + " File size :- " + node.size + " Peer num = " + node.peernum);
			addOrUpdateFile(node);
			allFileMap.add(node.name);
		}
		deleteFiles(allFileMap);
	}
	/**
	 * will delete those files which are not in sync with / deleted from tracker 
	 * @param allFileMap
	 */
	private void deleteFiles(HashSet<String> allFileMap) {
		Set<Map.Entry<String, FileInfo>> localFileSet = trackerInfo.getFileTable().entrySet();
		for (Map.Entry<String, FileInfo> entry : localFileSet) {
			if(!allFileMap.contains(entry.getKey())){
				System.out.println("Found a file to delte : - " + entry.getKey());
				// Delete file from local 
				File file = entry.getValue().file;
				// for synchronization hash table is accessed again
				int prevFileStatus = trackerInfo.getFileTable().get(entry.getKey()).fileStatus;
				trackerInfo.getFileTable().get(entry.getKey()).fileStatus = TrackerInfo.FILE_STATUS_DELETED;
				if(!file.delete()){ // if delete is not success 
					System.out.println("Not able to delete :- " + entry.getKey());
					entry.getValue().fileStatus = prevFileStatus;
				}
			}
		}
		
	}

	private void addOrUpdateFile(Node node) {
		Map<String, FileInfo> fileMap = trackerInfo.getFileTable();
		synchronized (fileMap) {
			if (fileMap != null) {
				final String fileName = node.name;
				final int updatedSize = node.size;
				final long updatedTime = node.timestamp;
				final int peerNum = node.peernum;
				final int[] peersIps = node.peerip;
				if (fileMap.containsKey(fileName)) {
					FileInfo info = fileMap.get(node.name);
					if (info.file.length() != updatedSize && info.file.lastModified() < updatedTime) {
						// file mismatch found should update file
						System.out.println("file mismatch - " + fileName + " previous size = " + info.file.length() + " updated size = " + updatedSize);
						updateFile(fileName,peerNum, peersIps,updatedSize);
					}else{
						System.out.println("file is already synced - " + fileName + " previous size = " + info.file.length() + " updated size = " + updatedSize);
					}
				} else {
						downloadFile(fileName, peerNum, peersIps,updatedSize);
				}
			}
		}
	}

	private void downloadFile(final String name, final int peerNum, final int[] peersIps, final int fileSize) {
		new Thread() {
			@Override
			public void run() {
				try {
					File tempFile = File.createTempFile("dart", "dart");
					if(downloadChunk(name, peersIps[0], 0, fileSize,tempFile)){
						File newFile = new File(rootDir.getAbsolutePath() + "/" + name);
						synchronized (trackerInfo) {
							FileInfo fileInfo = (trackerInfo.getFileTable().containsKey(name)) ? trackerInfo.getFileTable().get(name): new FileInfo(newFile, TrackerInfo.FILE_STATUS_DOWNLOADING);;
							fileInfo.fileStatus = TrackerInfo.FILE_STATUS_DOWNLOADING;
							trackerInfo.getFileTable().put(name, fileInfo);
							trackerInfo.getFileList().add(newFile);
						}
						if(copyFile(tempFile, newFile)){
							System.out.println("copyFile Sucess for :- " + newFile.getName());
							synchronized (trackerInfo) {
								FileInfo fileInfo = trackerInfo.getFileTable().get(name);
								fileInfo.fileStatus = TrackerInfo.FILE_STATUS_DOWNLOADED;
							}
						}
					}
					
				} catch (IOException ex) {
					System.out.println("Exception in downloading file :- " + ex.getMessage());
					ex.printStackTrace();
				}
			}
		}.start();
	}

	private boolean copyFile(File tempFile, File newFile) throws IOException {
		boolean result = false;
		if (!newFile.exists()) {
			newFile.createNewFile();
		}
		FileInputStream fis = new FileInputStream(tempFile);
		FileOutputStream fos = new FileOutputStream(newFile,true);
		int totalWritten = 0 ;
		int received = 0 ;
		byte buffer[] = new byte[1024];
		while ((received = fis.read(buffer)) > 0) {
			totalWritten += received;
			fos.write(buffer);
		}
		System.out.println("copyFile to " + newFile.getName() + " total written = " + totalWritten);
		fis.close();
		fos.close();
		result = true;
		return result;
	}
	private void updateFile(final String name, final int peerNum, final int[] peersIps, final int fileSize) {
		System.out.println("updateFile :- " + name + " peerNum = " + peerNum + " peerIp = " + peersIps[0] + " fileSize = " + fileSize );
		downloadFile(name, peerNum, peersIps, fileSize);
	}
	
	private boolean downloadChunk(String fileName, int ipAddress, int offset, int length, File toFile) {
		System.out.println("downloading chunk :- " + fileName + " offset = " + offset + " size = " + length);
		boolean result = false;
		try {
			InetAddress addr = Client.getInetAddress(ipAddress);
			Socket socket = new Socket(addr, Constants.PORT_PEER_DOWNLOAD);
			socket.setKeepAlive(true);
			socket.setSoTimeout(5000);
			PrintWriter pw = new PrintWriter(socket.getOutputStream(),true);
			InputStream is = socket.getInputStream();
			FileOutputStream fis = new FileOutputStream(toFile);
			pw.println(fileName + "," + offset + "," + length);
			byte[] bufferRead = new byte[1024];
			int received_size = 0 ;
			int totalRead = 0 ;
			while(totalRead < length && (received_size = is.read(bufferRead)) > 0){
				totalRead += received_size;
				fis.write(bufferRead);
			}
			if(totalRead == length){
				result = true;
			}
			System.out.println("downloading chunk :- " + fileName + " total read = " + totalRead);
			fis.close();
			socket.close();
		} catch (Exception ex) {
			System.out.println("Exception in downloadChunk :- " + ex.getMessage());
			System.out.println("Error in downloading chunk :- " + fileName + " offset = " + offset + " size = " + length);
			ex.printStackTrace();
		}
		return result;
	}
	
	
}
