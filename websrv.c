/*
 * websrv.c
 *
 *  Created on: 2009/04/06
 *      Author: yuichi
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>//close
#include <signal.h>
#include <sys/wait.h>
#include "mystr.h"
#include "myfile.h"
#include "myweb.h"
#include "http.h"

#define SRV_PORT 10010
#define MAX_QUEUE 5
#define BUF_SIZE 1024
#define ARGV_SIZE 64
#define FILE_SIZE 64
#define ANY_CHILD -1

#define TRUE  1
#define FALSE 0

//global var
int status;//for signal


void MyWait(int sig)
{

	/* waitpid return -1 or 0 or pid
	 * if -1 -> error not child
	 * if  0 -> child don't change state
	 * if pid -> child kill
	 */
	while(waitpid(ANY_CHILD,&status,WNOHANG) > 0);


}


int
main(int argc,char *argv[],char *envp[])
{


	/**************************** var *******************************/
	//socket
	int fd,new_fd;
	struct sockaddr_in srvskt,cliskt;
	socklen_t cli_sktlen = sizeof(cliskt);
	in_port_t srv_port = SRV_PORT;
	int real_rcvsize = 0,real_sendsize = 0;
	char rcvbuf[BUF_SIZE],sendbuf[BUF_SIZE];
	char *tmp_ptr;int tmpsize = 0;//for check header
	int msglen;
	//analyze
	char *line;
	int myargc = 0;
	char *myargv[ARGV_SIZE];
	//file
	char filename[FILE_SIZE];
	char *file_contents;
	int filesize;
	//header
	Status_Line status_line;
	int head_flag = FALSE;
	//fork
	int pid;
	/********************************** var end ***************************/

	/*******************set server socket **************/
	if((fd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("socket");
		exit(-1);
	}

	//srv addr
	memset(&srvskt,0,sizeof(srvskt));
	srvskt.sin_family = AF_INET;
	srvskt.sin_port = htons(srv_port);
	srvskt.sin_addr.s_addr = htonl(INADDR_ANY);

	if( bind(fd,(struct sockaddr *)&srvskt,sizeof(srvskt)) < 0 ){
		perror("bind");
		exit(-1);
	}

	if(listen(fd,MAX_QUEUE) < 0){
		perror("listen");
		exit(-1);
	}
	/*******************set server socket end**************/
	//set signal
	signal(SIGCHLD,MyWait);
	//set default
	Set_Default(&status_line);

	while(1){//while start
		Init_Var(myargv,ARGV_SIZE,&cliskt,&cli_sktlen);
		//accept
		if( (new_fd = accept(fd,(struct sockaddr *)&cliskt,&cli_sktlen)) < 0){
			perror("accept");
			continue;
		}
		pid = fork();
		if(pid < 0){//error(only parent)
			fprintf(stderr,"pid() error!!\n");
			continue;
		}
		else if(pid != 0){//parent
			close(new_fd);
			continue;
		}
		else{//child(pid == 0)
			/*************************** recv ***********************/
			tmp_ptr = rcvbuf;
			while(1){
				if((real_rcvsize = recv(new_fd,tmp_ptr,(sizeof(rcvbuf)-tmpsize),0)) <= 0){
					if(real_rcvsize < 0){
						perror("recv");
					}
					close(new_fd);
					_exit(-1);
				}
				tmp_ptr += real_rcvsize;
				tmpsize += real_rcvsize;
				if(Check_Header_End(rcvbuf,tmpsize) > 0){//CRLF CRLF
					break;//normal
				}
				else if(tmpsize >= sizeof(rcvbuf)){//buffer over flow
					//fprintf(stderr,"buffer over flow\n");
					break;//error
				}
			}//kokomadede,kanarazu,rcvbuf,kanseisaseru!
			real_rcvsize = tmpsize;
			rcvbuf[BUF_SIZE-1] = '\n';//for getline
			/*************************** recv end ***********************/

			if((line = Get_Line(rcvbuf,1,real_rcvsize)) == NULL){//error 500 server error
				status_line.code_and_reason = INTERNAL_SERVER_ERROR;
				strncpy(filename,"error.html",sizeof(filename));
				goto COMMON;
			}
			getargs(line,&myargc,myargv);

			if(myargc == 3){
				//set filename
				snprintf(filename,sizeof(filename),".%s",myargv[1]);
				//Version Check
				if(Check_Version(myargv[2]) < 0){//error 501 Server Not Implemented
					status_line.code_and_reason = NOT_IMPLEMENTED;
					strncpy(filename,"error.html",sizeof(filename));
				}
				if(strcmp(GET,myargv[0]) == 0){//GET
					head_flag = FALSE;
				}
				else if(strcmp(HEAD,myargv[0]) == 0){//HEAD
					head_flag = TRUE;
				}
				else{//error unknown method
					status_line.code_and_reason = METHOD_NOT_ALLOWED;
					strncpy(filename,"error.html",sizeof(filename));
				}
			}
			else if(myargc != 3){//error 400 client bad request(not filename)
				status_line.code_and_reason = BAD_REQUEST;
				strncpy(filename,"error.html",sizeof(filename));
			}



			//kokomadede,filename,status_line,hakanseisaseteoku!
			/******************* SEND COMMON *********************/
			COMMON:
			if(Check_File(filename) < 0){//error 404
				status_line.code_and_reason = NOT_FOUND;
				strncpy(filename,"error.html",sizeof(filename));//strcpy ha '\0' mohukumete copy
			}
			filesize = Get_File_Size(filename);
			//Make Header
			Make_Status_Line(status_line,sendbuf);
			Make_Object_Header(filesize,sendbuf);
			Make_End(sendbuf);
			msglen = strlen(sendbuf);

			//first send (header)
			if((real_sendsize = send(new_fd,sendbuf,msglen,0)) < 0){//head
				perror("send");
				_exit(-1);
			}
			//second send (file)
			if((head_flag == FALSE) || (strcmp(status_line.code_and_reason,OK) != 0)){//any error?
				if((file_contents = Get_File_Contents(filename,filesize)) == NULL){//error not memory
					fprintf(stderr,"Child:Can't allocate memory!!\n");
					_exit(-1);
				}
				if(file_contents != NULL){
					if((real_sendsize = send(new_fd,file_contents,filesize,0)) < 0){//file
						perror("send");
						_exit(-1);
					}
					free(file_contents);
				}
			}
			//free
			if(line != NULL)
				free(line);
			close(new_fd);//when srv closed,send EOF
			_exit(0);//child end
			/************************ SEND COMMON END *****************/


		}
	}//while end

	fprintf(stderr,"server down!!\n");
	return 0;

}







