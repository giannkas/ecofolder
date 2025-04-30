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
  int len = strlen(str), i = 0, c_delim = 0, j;
  char *tok = malloc(len+1);

  for (j = 0; i < len && c_delim <= ins; i++){
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
  if (strchr(tok, '\n') || (i == len && *(tok+j) != '\0')) 
    ins = c_delim + 1;
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

int compare(const void *a, const void *b) {
  return strcmp(*(const char **)a, *(const char **)b);
}

int compare_str_mrks(char *a, char *b) {
  int comparison = 1;
  // Make modifiable copies of the input strings
  char *a_copy = strdup(a);
  char *b_copy = strdup(b);
  if (!a_copy || !b_copy) comparison = 0;

  // Count tokens in both strings
  int max_tokens = 0;
  for (int i = 0; a[i]; i++) if (a[i] == ',') max_tokens++;
  for (int i = 0; b[i]; i++) if (b[i] == ',') max_tokens++;
  max_tokens++; // one more than number of commas

  char **tokens_a = malloc(max_tokens * sizeof(char *));
  char **tokens_b = malloc(max_tokens * sizeof(char *));
  if (!tokens_a || !tokens_b) comparison = 0;

  int count_a = 0, count_b = 0;
  char *token = strtok(a_copy, ",");
  while (token != NULL) 
  {
    tokens_a[count_a++] = strdup(token);
    token = strtok(NULL, ",");
  }

  token = strtok(b_copy, ",");
  while (token != NULL) 
  {
    tokens_b[count_b++] = strdup(token);
    token = strtok(NULL, ",");
  }

  if (count_a != count_b) 
  {
    // Clean up
    for (int i = 0; i < count_a; i++) free(tokens_a[i]);
    for (int i = 0; i < count_b; i++) free(tokens_b[i]);
    free(tokens_a); free(tokens_b);
      free(a_copy); free(b_copy);
      comparison = 0;
  }
    
  qsort(tokens_a, count_a, sizeof(char *), compare);
  printf("hola\n");
  qsort(tokens_b, count_b, sizeof(char *), compare);
    
  for (int i = 0; i < count_a && comparison; i++) {
    if (strcmp(tokens_a[i], tokens_b[i])) {
      // Clean up
      for (int j = 0; j < count_a; j++) free(tokens_a[j]);
      for (int j = 0; j < count_b; j++) free(tokens_b[j]);
      free(tokens_a); free(tokens_b);
      free(a_copy); free(b_copy);
      comparison = 0;
    }
  }

  // Clean up
  for (int i = 0; i < count_a; i++) {
      free(tokens_a[i]);
      free(tokens_b[i]);
  }
  free(tokens_a); free(tokens_b);
  free(a_copy); free(b_copy);

  return comparison;
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
