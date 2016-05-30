#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <arpa/inet.h>
#include "../utils/filetable.h"
#include "peer.h"

#define EVENT_LEN		(sizeof(struct inotify_event ))
#define EVENT_BUF_LEN		(1024 * (EVENT_LEN + 16))


typedef struct {
char filepath[1024]; // Path of the file
int size; // Size of the file
unsigned int lastModifyTime;// time stamp of last modification
} FileInfo;


int watchDirectory(peer_file_table* ptable,char* directory);
int readConfigFile( char* filename ); //Read config file from disk.
int fileAdded(peer_file_table* ptable,char* filename); //Called when a file is added to the directory.
int fileModified(peer_file_table* ptable,char* filename ); //Called when a file is modified.
int fileDeleted(peer_file_table* ptable,char* filename); //Called when a file is deleted.
int getAllFilesInfo(); //Get all filenames/sizes/timestamps in the directory
FileInfo* getFileInfo(char* filename); //Get information of a specific file
void blockUpdate();
void unblockUpdate();
int freeAll(); //Free all memory.
int blockFileAddListenning(); //Prevent unnecessary alert when downloading new files
int unblockFuleAddListenning(); //Unblock it.
int blockFileWriteListenning(); //Prevent unnecessary alert when modifying a file
int unblockFileWriteListenning(); //Unblock it.
int blockFileDeleteListenning(); //Prevent unnecessary alert when deleting file
int unblockFileDeleteListenning(); //Unblock it.
