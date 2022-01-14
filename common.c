#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

#include "common.h"

/****************************************************************************/
/* nc_error								    */
/* Outputs an error message to stderr and exits.			    */

void nc_error (const char* msg, ...)
{
	va_list	the_args;
	va_start(the_args,msg);
	fprintf(stderr,"%s: ",PROGRAM_NAME);
	vfprintf(stderr,msg,the_args);
	fprintf(stderr,"\n");
	va_end(the_args);
	exit(1);
}

/****************************************************************************/
/* nc_warning								    */
/* Emits a warning, but doesn't terminate.				    */

void nc_warning (const char* msg, ...)
{
	va_list	the_args;
	va_start(the_args,msg);
	fprintf(stderr,"%s warning: ",PROGRAM_NAME);
	vfprintf(stderr,msg,the_args);
	fprintf(stderr,"\n");
	va_end(the_args);
}

int strtoint(char *num) {
    int  i, len;
    int result = 0;

    len = strlen(num);

    for(i=0; i<len; i++)
        result = result * 10 + ( num[i] - '0' );
    return result;
}

void revstr(char *str){    
    int i, len, tmp;  
    len = strlen(str);
      
    for (i = 0; i < len/2; i++)  
    {  
        tmp = str[i];  
        str[i] = str[len-i-1];  
        str[len-i-1] = tmp;  
    }  
}

void flip(char *str, char oldsym, char newsym){
    int i, len;  
    len = strlen(str);
      
    for (i = 0; i < len && str[i] != oldsym; i++);
    if (i < len && str[i] == oldsym)
        str[i] = newsym;
}

char* ftokstr(char *str, int pos, char delim){
    int len = strlen(str), i = 0, c_delim = 0;
    char tok[len];
    char *result;
    
    for (int j = 0; i < len && c_delim <= pos; i++){
        if(str[i] == delim && c_delim != pos){
            tok[0] = '\0';
            c_delim++; j = 0;
        }else if(str[i] == delim){
            c_delim++; tok[j] = '\0';
        }
        else{
            tok[j] = str[i];
            j++;
        }        
    }
    result = strdup(tok);
    return i == len && pos > c_delim ? NULL : result;
}

char* ltokstr(char *str, char delim){
    int len = strlen(str), pos;
    char tok[len];
    char *result;

    for (pos = 0; pos < len && str[pos] != delim; pos++);
    /* while (pos < len && str[pos] != delim)
        pos++; */
    
    if(str[pos] == delim){
        for (int i = 0; i < strlen(tok) && pos+1 < len; i++, pos++)
            tok[i] = str[pos+1];
    }
    //strcat(tok,"\0");
    result = strdup(tok);
    //strcat(result, "\0");
    return result;
}

/****************************************************************************/
/* Wrapper functions for memory allocation.				    */

void* MYmalloc(size_t size)
{
	//printf("size: %lu\n", size);
	void * m;
	if (!(m = malloc(size))) nc_error("memory allocation rejected!");
	return m;
}

void* MYcalloc(size_t size)
{
	void * m;
	if (!(m = calloc(size, 1))) nc_error("memory allocation rejected!");
	memset(m,0,size);
	return m;
}

void* MYrealloc(void *ptr, size_t size)
{
	void *r;
	if (!(r = realloc(ptr,size))) nc_error("memory allocation rejected!");
	return r;
}

char* MYstrdup(char *string)
{
	char *str;
	if (!(str = strdup(string))) nc_error("memory allocation rejected!");
	return str;
}
