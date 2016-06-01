/**
 * 
 */
package com.dartsync;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;


/**
 * @author Vishal Gaurav
 *
 */
public class FileUploader extends Thread {
	
	private File rootDir = null;
	
	public FileUploader(File rootDir) {
		this.rootDir = rootDir;
	}
	
	@Override
	public void run() {
		try (ServerSocket serverSocket = new ServerSocket(Constants.PORT_PEER_DOWNLOAD)){
			System.out.println("File uploader started .. waiting for connection on IP = " + Client.getLocalIp() + " and port = " + Constants.PORT_PEER_DOWNLOAD);
			System.out.println(Client.getInetAddress(Client.getLocalIp()).toString());
			while(true){
				final Socket clientSocket = serverSocket.accept();
				System.out.println("Peer connected :- " + clientSocket.getInetAddress().getHostAddress());
				new Thread(){
					@Override
					public void run() {
						try{
							uploadFile(clientSocket);
						}catch (IOException ex){
							System.out.println("Error in uploading file :- ");
						}
					}
				}.start();
			}
			
		} catch (IOException e) {
			System.out.println("Error in server socket");
			e.printStackTrace();
		}
	}

	private void uploadFile(Socket clientSocket) throws IOException {
		if(clientSocket !=  null && clientSocket.isConnected()){
			try{
				BufferedReader br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
				System.out.println("waiting for header");
				String fileInfo = br.readLine();
				/**
				 * 1. fileName
				 * 2. offset
				 * 3. pieceLength
				 */
				System.out.println("uploadFile :- " + fileInfo);
				String[] tokens = fileInfo.split(",");
				if(tokens.length >= 3){
					String fileName = tokens[0];
					int offset = Integer.parseInt(tokens[1]);
					int pieceLength = Integer.parseInt(tokens[2]);
					pieceLength  = (pieceLength > 0) ? pieceLength : Constants.PIECE_LENGTH;
					File fileToUpload = new File(rootDir.getAbsolutePath() + "/" + fileName );
					if(fileToUpload.exists()){
						writeFileToStream(fileToUpload, clientSocket.getOutputStream(), offset, pieceLength);
					}else{
						System.out.println("File requested doesn't exists :- " + fileToUpload.getAbsolutePath());
					}
				}else{
					System.out.println("Insufficent information sent to get file");
				}
			}catch(Exception ex){
				System.out.println(ex.getMessage());
				ex.printStackTrace();
			}
			clientSocket.close();
		}
	}
	private int getChunkSize(int total_sent, int size) {
		int chunk_size = 0;
		int remaining = size - total_sent;
		if (remaining < Constants.FILE_BUFFER_SIZE) {
			chunk_size = remaining;
		} else {
			chunk_size = Constants.FILE_BUFFER_SIZE;
		}
		return chunk_size;
	}
	private void writeFileToStream(File fileToUpload, OutputStream os, int offset, int pieceLength) throws IOException{
		System.out.println("writeFileToStream :- " + offset + "" + pieceLength);
		FileInputStream fio = new FileInputStream(fileToUpload);
		fio.skip(offset);
		byte[] buffer = new byte[Constants.FILE_BUFFER_SIZE];
		int totalSent = 0 ;	
		while(totalSent < pieceLength){
			int chunkSize = getChunkSize(totalSent, pieceLength);
			int bytesRead = fio.read(buffer,0,chunkSize);
			os.write(buffer, 0, chunkSize);
			totalSent += bytesRead;
		}
		os.flush();
		fio.close();
	}
	
}
