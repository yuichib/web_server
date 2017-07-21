/*
 * myweb.c
 *
 *  Created on: 2009/04/08
 *      Author: yuichi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "mystr.h"
#include "myweb.h"
#include "http.h"

#define STR_SIZE 256

void
Make_Status_Line(Status_Line status_line,char *buf)
{
	status_line.version = HTTP_1_0;
	strcpy(buf,status_line.version);
	strcat(buf,SP);
	strcat(buf,status_line.code_and_reason);
	strcat(buf,CRLF);


}

void
Make_Object_Header(int filesize,char *buf){

	char str[STR_SIZE];
	snprintf(str,sizeof(str),"Content-Length:%d",filesize);
	strcat(buf,str);
	strcat(buf,CRLF);

}

void Make_End(char *buf){

	strcat(buf,CRLF);

}


void
Set_Default(Status_Line *status_line)
{
	status_line->version = HTTP_1_0;
	status_line->code_and_reason = OK;

}

void
Init_Var(char **argv,int argvsize,struct sockaddr_in *cliskt,socklen_t *cliskt_len)
{
	memset(argv,0,sizeof(char *) * argvsize);
	*cliskt_len = sizeof(struct sockaddr_in);
	memset(cliskt,0,*cliskt_len);

}

int
Check_Header_End(char *buf,int bufsize)
{
	int i;
	for(i=0; i<bufsize; i++,buf++){
		if(i + 3 < bufsize){
			if(*buf == '\r' && *(buf+1) == '\n' && *(buf+2) == '\r' && *(buf+3) == '\n'){//CRLF CRLF
				return 1;
			}
		}
	}

	return -1;

}


int
Check_Version(char *version)
{
	char *p;
	if( (p = Search_Char(version,'\r',strlen(version))) == NULL){
		return -1;
	}
	*p = '\0';
	if( (strcmp(version,HTTP_1_0) == 0) || (strcmp(version,HTTP_1_1) == 0) ){
		return 1;
	}
	else{
		return -1;
	}

}
