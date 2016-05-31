#include <arpa/inet.h>          // inet_ntoa
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include "../utils/filetable.h"
#include "peer.h"

#define EVENT_LEN		(sizeof(struct inotify_event ))
#define EVENT_BUF_LEN		(1024 * (EVENT_LEN + 16))


typedef struct {
char filepath[1024]; // Path of the file
int size; // Size of the file
unsigned int lastModifyTime;// time stamp of last modification
} FileInfo;

typedef struct moniterlist{
char dirpath[1024]; // Path of the file
int wd; // Size of the file
struct moniterlist *pNext;
} dirlist;

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
