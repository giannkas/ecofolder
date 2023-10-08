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

char* ftokstr(char *str, int ins, char delim)
{    
  int len = strlen(str), i = 0, c_delim = 0;
  char *tok = malloc(len+1);

  for (int j = 0; i < len && c_delim <= ins; i++){
    if(str[i] == delim && c_delim != ins){
      *tok = '\0';
      c_delim++; j = 0;
    }else if(str[i] == delim){
      c_delim++; *(tok+j) = '\0';
    }
    else{
      *(tok+j) = str[i];
      j++;
    }
  }
  if (strchr(tok, '\n')) ins = c_delim + 1;
  return i == len && ins > c_delim ? NULL : tok;
}

char* bltokstr(char *str, int ins, char delim){
  int len = strlen(str), pos, i = 0, c_delim = 0;
  char *tok = malloc(len+1);

  for (pos = len-1; pos >= 0 && c_delim <= ins; pos--){
    if (str[pos] == delim){
      if (c_delim == ins) break;
      c_delim++;
    }
  }

  if(str[pos] == delim){
    for (i = 0; i < pos; i++)
      *(tok+i) = *(str+i);
  }
  *(tok+i) = '\0';
  return tok;
}

char* ltokstr(char *str, int ins, char delim){
  int len = strlen(str), pos, i = 0, c_delim = 0;
  char *tok = malloc(len+1);

  for (pos = 0; pos < len && c_delim <= ins; pos++){
    if (str[pos] == delim){
      if (c_delim == ins)
        break;
      c_delim++;
    }
  }

  if(str[pos] == delim){
    for (i = 0; i < len && pos+1 < len; i++, pos++)
      if (str[pos+1] == '\n')
        pos = len;
      else
        *(tok+i) = *(str+pos+1);
  }
  *(tok+i) = '\0';
  return tok;
}

/****************************************************************************/
/* Wrapper functions for memory allocation.				    */

void* MYmalloc(size_t size)
{
  //printf("size: %lu\n", size);
  void * m;
  if (!(m = malloc(size))) nc_error("MYmalloc memory allocation rejected!");
  return m;
}

void* MYcalloc(size_t size)
{
  void * m;
  if (!(m = calloc(size, 1))) nc_error("MYcalloc memory allocation rejected!");
  memset(m,0,size);
  return m;
}

void* MYrealloc(void *ptr, size_t size)
{
  void *r;
  if (!(r = realloc(ptr,size))) nc_error("MYrealloc memory allocation rejected!");
  return r;
}

char* MYstrdup(char *string)
{
  char *str;
  if (!(str = strdup(string))) nc_error("MYstrdup memory allocation rejected!");
  return str;
}
