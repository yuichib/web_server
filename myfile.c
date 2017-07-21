/*
 * myfile.c
 *
 *  Created on: 2009/04/07
 *      Author: yuichi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


int
Check_File(char *filename)
{
	int fd;
	if((fd = open(filename,O_RDONLY)) < 0){
		return -1;
	}
	close(fd);
	return 1;

}

int
Get_File_Size(char *filename)
{
	int bytes = 0;
	int c;
	FILE *fp;

	if((fp = fopen(filename,"r")) == NULL){
		return -1;
	}
	while((c = fgetc(fp)) != EOF){
		bytes++;
	}
	fclose(fp);

	return bytes;

}

char *
Get_File_Contents(char *filename,int filesize)
{
	int c;
	FILE *fp;
	char *file_contents;
	char *ret;

	if((fp = fopen(filename,"r")) == NULL){//404
		return NULL;
	}
	if((file_contents = (char *)malloc(filesize+1)) == NULL){//for '\0'
		fclose(fp);
		return NULL;
	}
	ret = file_contents;
	while((c = fgetc(fp)) != EOF){
		*file_contents++ = (char)c;
	}
	*file_contents = '\0';
	fclose(fp);

	return ret;

}
