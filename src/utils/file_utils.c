/*
 * file_utils.c
 *
 *  Created on: May 23, 2016
 *      Author: Vishal Gaurav
 */
#include <unistd.h>

#include "constants.h"

/**
* function to check for the existence of a file
* returns TRUE or FALSE
*/
int is_file_exists(char *fname){
	return (access( fname, F_OK ) != -1 )? TRUE : FALSE ;
}

