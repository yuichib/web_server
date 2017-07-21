/*
 * myweb.h
 *
 *  Created on: 2009/04/08
 *      Author: yuichi
 */


typedef struct{
	char *version;
	char *code_and_reason;
}Status_Line;


void Make_Status_Line(Status_Line status_line,char *buf);
void Make_Object_Header(int filesize,char *buf);
void Make_End(char *buf);

void Set_Default(Status_Line *status_line);
void Init_Var(char **argv,int argvsize,struct sockaddr_in *cliskt,socklen_t *cliskt_len);
int Check_Header_End(char *buf,int bufsize);
int Check_Version(char *version);
