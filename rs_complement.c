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
    "%s -- reset complement encoding for reset nets\n\n"
    "Usage: %s <LLnetfile> \n\n"

  "Unless specified otherwise, all filenames will default to\n"
  "the basename of <LLnetfile> plus appropriate extensions.\n\n"

  "Version 1.0.0 (29.04.2022)\n", myname, myname);

  exit(1);
}

/*****************************************************************************/


char* rs_complement(char* in_file){
  // Declare the file pointer
  FILE *r_pointer ;
  FILE *w_pointer = NULL;
    
  // Variables that come along
  char d_read[MAX_RESET_PLACES];
  char out_file[MAX_RESET_PLACES] = {0};
  char *token, *tmp, *tmp1, *token2;
  int header = 0, places = 0, trans = 0, complement_places = 0, num_tmp, num_tmp1, names = 1;

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
    char place_names[MAX_RESET_PLACES][MAX_RESET_PLACES] = {0};
    /* Read up to RD label and get number of transitions and places*/
    while(fgets(d_read, MAX_RESET_PLACES, r_pointer) != NULL &&
       !strstr(d_read, "RD") && !strstr(d_read, "TP")){
      if (d_read[0] == 'P' && d_read[1] == 'L');
      else if (places == header) header++;
      if (d_read[0] == 'T' && d_read[1] == 'R');
      else if (trans == places){
        if( d_read[0] == '\"'){
          strcpy(place_names[names], d_read);
          names++;
        }
        places++;
      }
      trans++;
    }
    /* Compute header lines, number of places and transitions  */
    header++;
    places = places - header;
    header++;
    trans = trans - places - header;
    printf("header: %d\n", header);
    printf("places: %d\n", places);
    printf("trans: %d\n", trans);
    while(fgets(d_read, MAX_RESET_PLACES, r_pointer) != NULL && !strstr(d_read, "RS")){}
    if(strstr(d_read, "RS")){
      strcat(out_file, ftokstr(in_file, 0, '.'));
      strcat(out_file, "_rs.ll_net");
      w_pointer = fopen(out_file, "w"); // if we have reset arcs then create a new file.      
      char buf_arcs[MAX_RESET_PLACES];
      int* complement_place = calloc(places+1, sizeof(int));
      int* id_complement_place = calloc(places+1, sizeof(int));
      int (*presets)[trans+1] = calloc(places+1, sizeof *presets);
      int (*postsets)[trans+1] = calloc(places+1, sizeof *postsets);
      int (*resets)[trans+1] = calloc(places+1, sizeof *resets);
            
      char buffer_pl[MAX_LINE_SIZE] = {0};      

      /* Check for those places that have at least one reset arc and make its respective
        complement in buffer_pl which will be pasted into the output file */
      while(fgets(d_read, MAX_RESET_PLACES, r_pointer) != NULL && isdigit(d_read[0])){        
        token = strtok(d_read, ">");        
        num_tmp = strtol(token, &token2, 10);        
        if(complement_place[num_tmp] == 0){
          if (!strcmp(ltokstr(place_names[num_tmp], 1, '\"'),"M1\n"))
            sprintf(buf_arcs, "\"%s%s\"\n,", "¬", 
              ftokstr(place_names[num_tmp], 1, '\"'));
          else
            sprintf(buf_arcs, "\"%s%s\"%s\n,", "¬", 
              ftokstr(place_names[num_tmp], 1, '\"'),"M1");
          strcat(buffer_pl, buf_arcs);
          complement_places++;
          id_complement_place[complement_places] = num_tmp;
          complement_place[num_tmp] = places+complement_places;
        }
      }

      /* Print into the output file the places that come from the input file plus
      those that were replicated (print the content from buffer_pl) */      
      fseek( r_pointer, 0, SEEK_SET );
      while(fgets(d_read, MAX_RESET_PLACES, r_pointer) != NULL && !strstr(d_read, "TR")){
        fprintf(w_pointer, "%s", d_read);
      }
      if (strstr(d_read, "TR")){
        token = strtok(buffer_pl, ",");
        while (token != NULL){
          fprintf(w_pointer, "%s", token);
          token = strtok(NULL, ",");
        }
      }

      fseek( r_pointer, -strlen(d_read), SEEK_CUR );
      while(fgets(d_read, MAX_RESET_PLACES, r_pointer) != NULL && !strstr(d_read, "TP")){
        fprintf(w_pointer, "%s", d_read);
      }

      /* TP SECTION */
      /* Reading the transitions that are part of the preset set of a place whose reset set is not empty to create later on its complement. This part is just to build the preset of certain places which have not empty reset set*/
      if(strstr(d_read, "TP")){
        while(fgets(d_read, MAX_RESET_PLACES, r_pointer) && !strstr(d_read, "PT")){
          if( strlen(d_read) > 2 && isdigit(d_read[0]) ){
            tmp1 = ltokstr(d_read, 0, '<');
            tmp = ftokstr(d_read, 0, '<');
            num_tmp = strtol(tmp, &token2, 10);
            num_tmp1 = strtol(tmp1, &token2, 10);
            if(complement_place[num_tmp1] > 0)
              presets[num_tmp1][num_tmp] = num_tmp;
          }
        }
      }

      /* PT SECTION */
      /* Reading the transitions that are part of the postset set of a place whose reset set is not empty to create its complement*/
      if(strstr(d_read, "PT")){        
        while(fgets(d_read, MAX_RESET_PLACES, r_pointer) && !strstr(d_read, "RS")){
          if( strlen(d_read) > 2 && isdigit(d_read[0]) ){
            tmp = ftokstr(d_read, 0, '>');
            tmp1 = ltokstr(d_read, 0, '>');
            num_tmp = strtol(tmp, &token2, 10);
            num_tmp1 = strtol(tmp1, &token2, 10);
            if(complement_place[num_tmp] > 0)
              postsets[num_tmp][num_tmp1] = num_tmp1;
          }
        }
      }

      /* RS SECTION */
      /* Reading the transitions that are part of the reset set of a place to create its complement*/
      if(strstr(d_read, "RS")){        
        while(fgets(d_read, MAX_RESET_PLACES, r_pointer)){
          if( strlen(d_read) > 2 && isdigit(d_read[0]) ){
            tmp = ftokstr(d_read, 0, '>');
            tmp1 = ltokstr(d_read, 0, '>');
            num_tmp = strtol(tmp, &token2, 10);
            num_tmp1 = strtol(tmp1, &token2, 10);
            if(complement_place[num_tmp] > 0)
              resets[num_tmp][num_tmp1] = num_tmp1;
          }
        }
      }

      /* making complement places */
      fseek( r_pointer, 0, SEEK_SET );
      while(fgets(d_read, MAX_RESET_PLACES, r_pointer) != NULL && !strstr(d_read, "TP")){}
      
      /* Constructing the preset of ¬p according to p: pre(¬p) = (post(p) U res(p))\pre(p) */
      if(strstr(d_read, "TP")){
        fprintf(w_pointer, "%s", d_read);
        for (size_t i = 1; i <= complement_places; i++){
          num_tmp1 = id_complement_place[i];
          if (complement_place[num_tmp1] > 0){
            for (size_t k = 1; k <= trans; k++){
              if(postsets[num_tmp1][k] != 0 && postsets[num_tmp1][k] != presets[num_tmp1][k]){
                sprintf(buf_arcs, "%d<%d", postsets[num_tmp1][k], complement_place[num_tmp1]);
                fprintf(w_pointer, "%s\n", buf_arcs);
              }
              if(resets[num_tmp1][k] != 0 && resets[num_tmp1][k] != presets[num_tmp1][k] && resets[num_tmp1][k] != postsets[num_tmp1][k]){
                sprintf(buf_arcs, "%d<%d", resets[num_tmp1][k], complement_place[num_tmp1]);
                fprintf(w_pointer, "%s\n", buf_arcs);
              }
            }
            complement_place[num_tmp1] *= -1;
          }
        }
        while(fgets(d_read, MAX_RESET_PLACES, r_pointer) && !strstr(d_read, "PT"))
          fprintf(w_pointer, "%s", d_read);
      }

      /* Constructing the postset of ¬p according to p: post(¬p) = pre(p)\(post(p) U res(p)) */
      if(strstr(d_read, "PT")){
        fprintf(w_pointer, "%s", d_read);
        for (size_t i = 1; i <= complement_places; i++){
          num_tmp = id_complement_place[i];
          if (complement_place[num_tmp] < 0){
            for (size_t k = 1; k <= trans; k++){
              if(presets[num_tmp][k] != 0 && presets[num_tmp][k] != postsets[num_tmp][k] && presets[num_tmp][k] != resets[num_tmp][k]){
                sprintf(buf_arcs, "%d>%d", complement_place[num_tmp]*-1, presets[num_tmp][k]);
                fprintf(w_pointer, "%s\n", buf_arcs);
              }
            }
            complement_place[num_tmp] *= -1;
          }
        }
        while(fgets(d_read, MAX_RESET_PLACES, r_pointer) && !strstr(d_read, "RS"))
          fprintf(w_pointer, "%s", d_read);
      }

      /* Constructing the reset of ¬p according to p: res(¬p) = res(p) */
      if(strstr(d_read, "RS")){
        fprintf(w_pointer, "%s", d_read);
        for (size_t i = 1; i <= complement_places; i++){
          num_tmp = id_complement_place[i];
          if (complement_place[num_tmp] > 0){
            for (size_t k = 1; k <= trans; k++){
              if(resets[num_tmp][k] != 0){
                sprintf(buf_arcs, "%d>%d", complement_place[num_tmp], resets[num_tmp][k]);
                fprintf(w_pointer, "%s\n", buf_arcs);
              }
            }
            complement_place[num_tmp] *= -1;
          }
        }
        while(fgets(d_read, MAX_RESET_PLACES, r_pointer))
          fprintf(w_pointer, "%s", d_read);
      }
    }
    fclose(r_pointer);
    fclose(w_pointer);
  }
  return out_file[0] != '\0' ? out_file : in_file;
}

int main (int argc, char **argv)
{
  if (argc != 2){
    fprintf(stderr,"usage: rs_encoding <ll_netfile>\n");
    exit(1);
  }
  rs_complement(argv[1]);
  exit(0);
}