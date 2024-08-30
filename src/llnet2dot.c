#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#define READ_PLACES 500

/*****************************************************************************/

void usage(char *myname)
{
  fprintf(stderr,
    "%s -- llnet format to dot format for depicting nets\n\n"
    "Usage: %s <LLnetfile> \n\n"

  "Unless specified otherwise, all filenames will default to\n"
  "the basename of <LLnetfile> plus appropriate extensions.\n\n"

  "Version 1.1.0 (17.01.2023)\n", myname, myname);

  exit(1);
}

/*****************************************************************************/


void llnet2dot(char* in_file){
  // Declare the file pointer
  FILE *r_pointer ;
  FILE *w_pointer = NULL;
    
  // Variables that come along
  char d_read[READ_PLACES];
  char out_file[READ_PLACES] = {0};
  char *token, *left, *right;    
  int header = 0, places = 0, trans = 0, 
    num_left, num_right;

  // Open the existing file using fopen()
  // in read mode using "r" attribute
  r_pointer = fopen(in_file, "r") ;
    
  // Check if this r_pointer is null
  // which maybe if the file does not exist
  if ( r_pointer == NULL ){
    printf( "%s file failed to open.", in_file ) ;
  }
  else
  {
    /* Read up to RD label and get number of transitions and places*/
    while(fgets(d_read, READ_PLACES, r_pointer) != NULL && !strstr(d_read, "RD\n")
       && !strstr(d_read, "TP\n"))
    {
      if (strstr(d_read, "PL\n"));
      else if (places == header) header++;
      if (strstr(d_read, "TR\n"));
      else if (trans == places) places++;
      trans++;
    }
    /* Compute header lines, number of places and transitions  */
    header++;
    places = places - header;
    header++;
    trans = trans - places - header;
    printf("header: %d\n", --header);
    printf("places: %d\n", places);
    printf("trans: %d\n", trans);
    strcat(out_file, ftokstr(in_file, 0, '.'));
    strcat(out_file, "_net.dot");
    w_pointer = fopen(out_file, "w"); // if we have read arcs then create a new file.      
    fprintf(w_pointer, "digraph test {\n");
    if(strstr(d_read, "RD\n")){
      while(fgets(d_read, READ_PLACES, r_pointer) != NULL && !strstr(d_read, "TP\n")){
        left = ftokstr(d_read, 0, '>');
        right = ltokstr(d_read, 0, '>');
        num_left = left ? strtol(left, &token, 10) : 0;
        num_right = right ? strtol(right, &token, 10) : 0;
        if (num_left && num_right)
          fprintf(w_pointer, "  p%d -> t%d [arrowhead=none color=\"blue\"];\n", num_left, num_right);
      }
    }
    if(strstr(d_read, "TP\n")){
      while(fgets(d_read, READ_PLACES, r_pointer) != NULL && !strstr(d_read, "PT\n")){
        left = ftokstr(d_read, 0, '<');
        right = ltokstr(d_read, 0, '<');
        num_left = left ? strtol(left, &token, 10) : 0;
        num_right = right ? strtol(right, &token, 10) : 0;
        if (num_left && num_right)
          fprintf(w_pointer, "  t%d -> p%d;\n", num_left, num_right);
      }
    }

    if(strstr(d_read, "PT\n")){
      while(fgets(d_read, READ_PLACES, r_pointer) != NULL && !strstr(d_read, "RS\n")){
        left = ftokstr(d_read, 0, '>');
        right = ltokstr(d_read, 0, '>');
        num_left = left ? strtol(left, &token, 10) : 0;
        num_right = right ? strtol(right, &token, 10) : 0;
        if (num_left && num_right)
          fprintf(w_pointer, "  p%d -> t%d;\n", num_left, num_right);
      }
    }

    if(strstr(d_read, "RS\n")){
      while(fgets(d_read, READ_PLACES, r_pointer) != NULL){
        left = ftokstr(d_read, 0, '>');
        right = ltokstr(d_read, 0, '>');
        num_left = left ? strtol(left, &token, 10) : 0;
        num_right = right ? strtol(right, &token, 10) : 0;
        if (num_left && num_right)
          fprintf(w_pointer, "  p%d -> t%d [arrowhead=odot];\n", num_left, num_right);
      }
    }

    fseek( r_pointer, 0, SEEK_SET );
    int i = 1;
    while(fgets(d_read, READ_PLACES, r_pointer) != NULL && !strstr(d_read, "TR\n")){
      if(strstr(d_read, "\"")){
        left = ftokstr(d_read, 0, '\"');
        if (strlen(left)) num_left = strtol(left, &token, 10);
        token = ftokstr(d_read, 1, '\"');
        if(i <= places){
          if(strstr(d_read, "M1\n"))
            fprintf(w_pointer, "  p%d [label=\"⬤\" xlabel= <<FONT COLOR=\"red\">%s</FONT>> shape=circle ];\n",strlen(left) ? num_left : i,token);
          else
            fprintf(w_pointer, "  p%d [label=\"\" xlabel= <<FONT COLOR=\"red\">%s</FONT>> shape=circle ];\n",strlen(left) ? num_left : i,token);
        }
        i++;
      }
    }

    i = 1;
    if(strstr(d_read, "TR\n")){
      while(fgets(d_read, READ_PLACES, r_pointer) != NULL && strstr(d_read, "\"")){
        left = ftokstr(d_read, 0, '\"');
        if (strlen(left)) num_left = strtol(left, &token, 10);
        token = ftokstr(d_read, 1, '\"');
        if(i <= trans)
          fprintf(w_pointer, "  t%d [fillcolor=\"#cccccc\" label=\"%s\" shape=box style=filled];\n",strlen(left) ? num_left : i,token);
        i++;
      }
    }
    
    fprintf(w_pointer, "}");

    fclose(r_pointer) ;
    fclose(w_pointer) ;

  }
}

int main (int argc, char **argv)
{
  if (argc != 2 || !strstr(argv[1],".ll_net") || !strstr(argv[1],".ll")){
    fprintf(stderr,"usage: llnet2dot <ll_netfile>\n");
    exit(1);
  }
  llnet2dot(argv[1]);
  exit(0);
}