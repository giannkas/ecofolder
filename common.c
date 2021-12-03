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

int last_strchr(const char *str, int c){
	
	int i = -1;
	for (i = strlen(str)-1; i >= 0; i--)
	{
		if (str[i] == c)
			break;
	}
	return i;	
}

char * extract_substr_up_to(const char *str, int c){
	char *substr = c != -1 ? MYmalloc(c) : "";
	for (int i = 0; i < c; i++)		
		substr[i] = str[i];
	if (c != -1)
		substr[c] = '\0';
	return substr;
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
