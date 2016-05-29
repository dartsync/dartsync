/**
 * 
 */
package com.dartsync;

/**
 * @author Vishal Gaurav
 *
 */
public class SegmentTracker {
	// time interval that the peer should sending alive message periodically
	int interval;
	// piece length
	int pieceLength;
	// file number in the file table -- optional
	int file_table_size;
	// file table of the tracker -- your own design
	Node[] file_table = new Node[Constants.MAX_FILE_NUM];
}
