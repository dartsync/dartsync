/*
 * utils.c
 *
 *  Created on: May 23, 2016
 *      Author: Vishal Gaurav
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <assert.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "constants.h"

/*
* this method will call "clear command" through exec
*/
int clear_screen(int shouldWaitToFinish){
	int pid, rc, status ;
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	/* child process for clearing screen starts */
	rc  = fork();
	if (0 == rc) {
		pid = getppid();
		char *args[] = {NULL};
		rc = execvp("clear",args);
		exit(11);
	}
	pid = rc ;
	rc = waitpid(pid, &status, 0);
	return rc ;
	/* child process for clear ends */
}

long current_time_millis(){
	struct timeval stop, start;
	gettimeofday(&start, NULL);
	return (long)start.tv_usec;
}

/*void wait_for_some_time_n(int nano_sec){
	// sleep for 1000 nano seconds before next iteration
	struct timespec tim;
	tim.tv_sec = 0;
	tim.tv_nsec = nano_sec;
	nanosleep(&tim,NULL);
}
*/
/**
*helper method to ask user to press enter and continue
*/
void enter_to_continue(void){
	printf("Please press enter to continue : \n");
		while (TRUE){
			int c = getchar();
      if (c == '\n' || c == EOF)
				break;
			}
}

/** method for getting input from user
// Credits :- https://ssl.cs.dartmouth.edu/~sws/cs58-f15/proj1/demo.c
// prompt the user with message, and save input at buffer
// (which should have space for at least len bytes)
*/
int input_string(char *message, char *buffer, int len) {
  int rc = 0, fetched, lastchar;
  if (NULL == buffer)
    return -1;
  if (message)
    printf("%s: ", message);
  // get the string.  fgets takes in at most 1 character less than
  // the second parameter, in order to leave room for the terminating null.
  // See the man page for fgets.
  fgets(buffer, len, stdin);
  fetched = strlen(buffer);
  // warn the user if we may have left extra chars
  if ( (fetched + 1) >= len) {
    fprintf(stderr, "warning: might have left extra chars on input\n");
    rc = -1;
  }
  // consume a trailing newline
  if (fetched) {
    lastchar = fetched - 1;
    if ('\n' == buffer[lastchar])
      buffer[lastchar] = '\0';
  }
  return rc;
}
