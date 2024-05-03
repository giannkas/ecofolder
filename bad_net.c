#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#define MAX_NUM_PLACES 1000
#define NUM_PLACES 100

/*****************************************************************************/

void usage(char *myname)
{
  fprintf(stderr,
    "%s -- creating the bad net from another net\n\n"
    "Usage: %s <LLnetfile> <multiset bad markings> \n\n"

  "Unless specified otherwise, all filenames will default to\n"
  "the basename of <LLnetfile> plus appropriate extensions.\n"
  "<multiset of bad markings> has comma-separated 'bad' places \n"
  "of a net with multilines to indicate different bad markings.\n\n"

  "Version 1.0.0 (29.04.2022)\n", myname, myname);

  exit(1);
}

/*****************************************************************************/


void bad_net(char* in_file1, char* in_file2){
  // Declare the file pointer
  FILE *r_pointer1, *r_pointer2 ;
  FILE *w_pointer = NULL;
    
  // Variables that come along
  char d_read[NUM_PLACES];
  char out_file[NUM_PLACES] = {0};
  char *token, *tmp = NULL;
  int header = 0, places = 0, trans = 0, pnames = 1,
    nbadnames = 0, nbadmarkings = 0, nbadtrans = 1, i,
    lentmp, dummy = 0;

  // Open the existing file using fopen()
  // in read mode using "r" attribute
  r_pointer1 = fopen(in_file1, "r");
  r_pointer2 = fopen(in_file2, "r");
    
  // Check if this r_pointer2 is null
  // which maybe if the file does not exist
  if ( r_pointer2 == NULL ){
    printf( "%s file failed to open.\n", in_file2 ) ;
  }
  else
  {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    
    while ((read = getline(&line, &len, r_pointer2)) >= 0)
      nbadmarkings++;
    // Check if this r_pointer1 is null
    // which maybe if the file does not exist
    if ( r_pointer1 == NULL && nbadmarkings == 0 ){
      printf( "%s file failed to open or %s has no content.\n", 
        in_file1, in_file2 ) ;
    }
    else
    {
      strcat(out_file, ftokstr(in_file1, 0, '.'));
      strcat(out_file, "_bad.ll_net");
      w_pointer = fopen(out_file, "w");
      char place_names[MAX_NUM_PLACES][NUM_PLACES] = {0};
      /* Read up to RD label and get number of transitions and places*/
      while(fgets(d_read, NUM_PLACES, r_pointer1) != NULL && 
        !strstr(d_read, "RD") && !strstr(d_read, "TP")){
        if (d_read[0] == 'P' && d_read[1] == 'L')
          fprintf(w_pointer, "%s", d_read);
        else if (places == header)
        {
          fprintf(w_pointer, "%s", d_read);
          header++;
        } 
        if (d_read[0] == 'T' && d_read[1] == 'R')
        {
          /* Compute header lines, number of places */
          header++;
          places = places - header;
          if (!strlen(tmp))
            fprintf(w_pointer, "\"bad_P\"M1\n");
          else
            fprintf(w_pointer, "%d\"bad_P\"M1\n",places+1);
          fprintf(w_pointer, "%s", d_read);
        }
        else if (trans == places)
        {
          if( strstr(d_read,"\"")){
            token = ftokstr(d_read,1,'\"');
            tmp = ftokstr(d_read,0,'\"');
            fprintf(w_pointer, "%s\"%s\"\n", tmp,token);
            if (!strlen(tmp))
              strcpy(place_names[pnames], token);
            else
              strcpy(place_names[strtoint(tmp)], token);
            //printf("%s\n",token);
            pnames++;
          }
          places++;
        }
        else if (trans != places && strstr(d_read,"\""))
        {
          dummy = strlen(ftokstr(d_read,0,'\"'));
          fprintf(w_pointer, "%s", d_read);
        }
        trans++;
      }
      /* Compute header lines, number of places and transitions  */
      header++;
      trans = trans - places - header;
      for(; nbadtrans <= nbadmarkings; nbadtrans++)
        if (!dummy)
          fprintf(w_pointer, "\"bad_T%d\"\n", nbadtrans);
        else
          fprintf(w_pointer, "%d\"bad_T%d\"\n",nbadtrans+trans, nbadtrans);
      fprintf(w_pointer, "%s", d_read);
      printf("header: %d\n", header);
      printf("places: %d\n", places);
      printf("trans: %d\n", trans);
      nbadtrans = 1;
      dummy = 0;

      /* Print TP arcs (production) in the original net */
      while(fgets(d_read, NUM_PLACES, r_pointer1) != NULL &&
       !strstr(d_read, "PT"))
        fprintf(w_pointer, "%s", d_read);
      
      /* Print TP arcs from new bad transitions to each 
       place in a bad marking*/
      fseek( r_pointer2, 0, SEEK_SET );
      while ( (read = getline(&line, &len, r_pointer2)) >= 0 )
      {
        if (strchr(line, ','))
        {
          while ((tmp = ftokstr(line, nbadnames, ',')))
          {
            i = 1;
            while (i <= places && strcmp(place_names[i], tmp)) i++;
            if (i <= places)
            {
              fprintf(w_pointer, "%d<%d\n", trans+nbadtrans, i);
              nbadnames++;
            }
          }
          if ((tmp = ltokstr(line, nbadnames-1, ',')))
          {
            i = 1;
            while (i <= places && strcmp(place_names[i], tmp)) i++;
            if (i <= places)
              fprintf(w_pointer, "%d<%d\n", trans+nbadtrans, i);
          }  
          /* printf("line size of: %zu\n", read);
          printf("%s", line); */
          if (nbadnames > 0) nbadtrans++;
          else nbadtrans = 0;
          nbadnames = 0;
        }
        else
        {
          i = 1;
          lentmp = strlen(line);
          if (line[lentmp-1] == '\n')
            line[lentmp-1] = '\0';
          while (i <= places && strcmp(place_names[i], line)) i++;
          if (i <= places)
          {
            fprintf(w_pointer, "%d<%d\n", trans+nbadtrans, i);
            nbadtrans++;
          }
          else nbadtrans = 0;
        }
      }

      /* Print PT arcs (consumption) in the original net */
      fprintf(w_pointer, "%s", d_read);
      while(fgets(d_read, NUM_PLACES, r_pointer1) != NULL &&
       !strstr(d_read, "RS"))
        fprintf(w_pointer, "%s", d_read);

      /* Print PT arcs from new bad place to each 
       new bad transitions */
      if (nbadtrans > 0)
      {
        nbadtrans = trans + 1;
        for (; nbadtrans <= trans + nbadmarkings; nbadtrans++)
          fprintf(w_pointer, "%d>%d\n", places+1, nbadtrans);
      }
      
      /* Print RS arcs (reset) remaining of the original net */
      if (strstr(d_read,"RS"))
      {
        dummy = 1;
        fprintf(w_pointer, "%s", d_read);
      }
      while(fgets(d_read, NUM_PLACES, r_pointer1) != NULL)
        fprintf(w_pointer, "%s", d_read);

      /* Print RS arcs (reset) from the new bad transitions to 
      each place in the net */
      if (dummy && nbadtrans > 0)
      {
        nbadtrans = trans + 1;
        for (; nbadtrans <= trans + nbadmarkings; nbadtrans++)
        {
          i = 1;
          while (i <= places)
          {
            fprintf(w_pointer, "%d>%d\n", i, nbadtrans);
            i++;
          }
        }
      }

      fclose(r_pointer1);
      fclose(r_pointer2);
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
  bad_net(argv[1], argv[2]);
  exit(0);
}