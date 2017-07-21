/*
 * mystr.c
 *
 *  Created on: 2009/04/06
 *      Author: yuichi
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int isblank(int c);


static int
Get_1Line_Size(char *linebuf){

	int i=0;
	while(*linebuf != '\n'){
		i++;
		linebuf++;
	}
	i++;
	return i;//contain \n

}

char *Search_Char
(char *str,int search,int size){

	int i = 0;
	char *p = str;
	for( ; *p != '\0' && i<size; p++){
		if(*p == (char)search){
			return p;
		}
		i++;
	}
	return NULL;

}

char *Get_Line
(char *buf,int line,int bufsize){

	int num=0;//mojisuu
	int linenum = 1;//1 origin
	char *pmal;
	int malsize;

	while(1){
		if(num >= bufsize)//end buffer
			return NULL;
		if(linenum == line)
			break;
		if(*buf == '\n')
			linenum++;
		buf++;
		num++;
	}
	//store line
	malsize = Get_1Line_Size(buf);
	if( (pmal = (char *)malloc(malsize)) == NULL){
		fprintf(stderr,"Get_Line() Error\n");
		return NULL;
	}
	strncpy(pmal,buf,malsize);
	pmal[malsize-1] = '\0';
	return pmal;

}





void getargs
(char *cp, int *argc, char **argv){

  *argc = 0;
  argv[*argc] = NULL;
loop:
  while (*cp && isblank(*cp)) //skip space and tab
  cp++;

  if (*cp == '\0'){
    argv[(*argc)] = NULL;
    return;
  }
  argv[(*argc)++] = cp;

  while (*cp && !isblank(*cp)) //search for end of word
   cp++;
  if(*cp == '\0'){
    argv[(*argc)] = NULL;
    return;
  }

  *cp++ = '\0';
  goto loop;

}

