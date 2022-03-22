/****************************************************************************/
/* common.h								    */
/*									    */
/* Definitions for common.c						    */
/****************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define PROGRAM_NAME "unfold"

extern void* MYmalloc(size_t);
extern void* MYcalloc(size_t);
extern void* MYrealloc(void*,size_t);
extern char* MYstrdup(char*);
extern void nc_error (const char*,...);
extern void nc_warning (const char*,...);
int strtoint(char*);
char* ftokstr(char *, int, char);
char* ltokstr(char *, int, char);

//#define MALLOC_CHECK_ = 2;

#endif
