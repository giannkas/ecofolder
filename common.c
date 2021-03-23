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

/****************************************************************************/
/* Wrapper functions for memory allocation.				    */

void* MYmalloc(size_t size)
{
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
