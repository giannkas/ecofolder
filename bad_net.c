#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#define MAX_LINE_SIZE 500
#define MAX_RESET_PLACES 100

/*****************************************************************************/

void usage(char *myname)
{
  fprintf(stderr,
    "%s -- creating the bad net from another net\n\n"
    "Usage: %s <LLnetfile> <multiset bad markings> \n\n"

  "Unless specified otherwise, all filenames will default to\n"
  "the basename of <LLnetfile> plus appropriate extensions.\n"
  "<multiset of bad markings> has comma-separated 'bad' places of a net with multilines to indicate different bad markings.\n\n"

  "Version 1.0.0 (29.04.2022)\n", myname, myname);

  exit(1);
}

/*****************************************************************************/


char* bad_net(char* in_file1, char* in_file2){
  // Declare the file pointer
  FILE *r_pointer1, *r_pointer2 ;
  FILE *w_pointer = NULL;
    
  // Variables that come along
  char d_read[MAX_RESET_PLACES];
  char out_file[MAX_RESET_PLACES] = {0};
  char *token, *tmp, *tmp1, *token2;
  int header = 0, places = 0, trans = 0, complement_places = 0, num_tmp, num_tmp1, names = 1;

  // Open the existing file using fopen()
  // in read mode using "r" attribute
  r_pointer1 = fopen(in_file1, "r");
  r_pointer2 = fopen(in_file2, "r");
    
  // Check if this r_pointer2 is null
  // which maybe if the file does not exist
  if ( r_pointer2 == NULL ){
    printf( "%s file failed to open.", in_file2 ) ;
  }
  else
  {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    while ((read = getline(&line, &len, r_pointer2)) != -1)
    {
      printf("line size of: %zu\n", read);
      //printf("len value: %d\n", len);
      printf("%s\n", line);
    }

    // Check if this r_pointer1 is null
    // which maybe if the file does not exist
    if ( r_pointer1 == NULL ){
      printf( "%s file failed to open.", in_file1 ) ;
    }
    else
    {
      
      fclose(r_pointer1);
      fclose(w_pointer);
    }
  }
}

int main (int argc, char **argv)
{
  if (argc != 3){
    fprintf(stderr,"usage: bad_net <LLnetfile> <multiset bad markings>\n");
    exit(1);
  }
  bad_net(argv[2], argv[3]);
  exit(0);
}