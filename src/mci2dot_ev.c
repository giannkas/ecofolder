#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define LINE_SIZE 100


typedef struct cut_t
{
  int repeat;
  int szcut, szevscut;
  int *cut;
  int *evscut;
} cut_t;

typedef struct clist_t
{
  int   idcond;
  struct clist_t* next;
} clist_t;

typedef struct evprepost
{
  clist_t *preset;
  clist_t *postset;
  struct evprepost *next;
} evprepost;

clist_t* clist_add(clist_t** list, int idpl)
{
  clist_t* newco = malloc(sizeof(clist_t));
  newco->idcond = idpl;
  newco->next = *list;
  return *list = newco;
}

/**
 * @brief auxiliary function to find event successors of a particular event.
 * 
 * @param rows is the number of rows that the *matrix has.
 * @param cols is the number of cols that the *matrix has.
 * @param matrix matrix variable to look in for the event.
 * @param pre_ev the event source whose successor (post_ev) is needed.
 * @param post_ev the event target which it is a successor of (pre_ev).
 * @return integer we the event's index number if found else 0.
 */

int find_successor(int rows, int cols, int (*ev_succs)[cols], int pre_ev, int post_ev ){
  size_t j, found = 0;
  if(ev_succs[pre_ev][post_ev] != post_ev){ // if post_ev is not in the 
                                          //immediate successors of pre_ev.
    // we do loops to search in breadth.
    for ( j = pre_ev+1; j < cols && found == 0; j++){
      if( ev_succs[pre_ev][j] > 0)
        found = find_successor(rows, cols, ev_succs, ev_succs[pre_ev][j], post_ev);
      //if (found > 0) ev_succs[pre_ev][post_ev] = found;    
    }
  }
  else
    found = post_ev;
  return found;
}

int find_predecessor(int rows, int cols, int (*ev_predc)[cols], int pre_ev, int post_ev ){
  size_t j, found = 0;
  if(ev_predc[post_ev][pre_ev] != pre_ev){ // if pre_ev is not in the 
                                          //immediate predecessors of post_ev.
    // we do loops to search in breadth.
    for ( j = 1; j < post_ev && found == 0; j++){
      if( ev_predc[post_ev][j] > 0)
        found = find_predecessor(rows, cols, ev_predc, pre_ev, ev_predc[post_ev][j]);
    }
  }
  else
    found = pre_ev;
  return found;
}

int compute_cone_height(int numev, int (*ev_predc)[numev], int srcev, int height)
{
  int i;
  for (i = 1; i <= numev && !ev_predc[srcev][i]; i++) {}
  if (ev_predc[srcev][i])
  {
    if (srcev == 2)
      printf("ev_predc[srcev][%d] = %d\n", i, ev_predc[srcev][i]);
    height = compute_cone_height(numev, ev_predc, ev_predc[srcev][i], ++height);
  }
  return height;
}

void check_cone(int numev, int (*ev_predc)[numev], char cone_ev[], int confl_evs[numev], int srcev)
{
  char str[10];
  sprintf(str, "%d,", srcev);
  strcat(cone_ev, str);
  for (int i = 1; i <= numev; i++)
    if (ev_predc[srcev][i])
      check_cone(numev, ev_predc, cone_ev, confl_evs, ev_predc[srcev][i]);
}

void print_pathway(int numev, int (*ev_predc)[numev], 
  int(*ev_succs)[numev], int (*path_evs)[numev], char cone_ev[], 
  char cone_ev2[], int minlen, int confl_evs[numev], 
  int pre_ev, int link_ev, int seq_size, int path_seq[seq_size])
{
  int chk_predc = 0;
  for (int j = 1; j <= numev; j++)
  {
    int chk_inpath = 0;
    if (ev_predc[pre_ev][j] && !confl_evs[j] && !path_evs[j][link_ev])
    {
      chk_predc = 1;
      for (size_t i = 0; i < seq_size; i++)
      {
        if (path_seq[i] == ev_predc[pre_ev][j])
        {
          printf("  e%d -> e%d [minlen=%d];\n", j, link_ev, pre_ev == link_ev ? 1 : minlen); // write the connection.
          path_evs[j][0] = j;
          path_evs[j][link_ev] = link_ev;
          chk_inpath = 1;
          print_pathway(numev, ev_predc, ev_succs, path_evs, 
            cone_ev, cone_ev2, 1, confl_evs, ev_predc[pre_ev][j], 
            ev_predc[pre_ev][j], seq_size, path_seq);
        }
      }      
    }
    // If conflict event, then make connection and continue
    if (ev_predc[pre_ev][j] && confl_evs[j] && !path_evs[j][link_ev])
    {
      chk_predc = 1;
      printf("  e%d -> e%d [minlen=%d];\n", j, link_ev, pre_ev == link_ev ? 1 : minlen); // write the connection.
      path_evs[j][0] = j;
      path_evs[j][link_ev] = link_ev;
      print_pathway(numev, ev_predc, ev_succs, path_evs, 
        cone_ev, cone_ev2, 1, confl_evs, ev_predc[pre_ev][j], 
        ev_predc[pre_ev][j], seq_size, path_seq);
    }
    else if (ev_predc[pre_ev][j] && !confl_evs[j] && !path_evs[j][link_ev] && !chk_inpath)
    {
      chk_predc = 1;
      cone_ev[0] = '\0';
      cone_ev2[0] = '\0';
      int k = j;
      int chk_confl = 0, conct = 0;
      while (k <= numev && k)
      {
        // Check triangular (redundant) arcs.
        // If no conflict event, save it for later,
        // possible direct connection to the root.
        if (ev_predc[pre_ev][k] && confl_evs[k])
        {
          chk_confl = 1;
          check_cone(numev, ev_predc, cone_ev, confl_evs, j);
          check_cone(numev, ev_predc, cone_ev2, confl_evs, k);
          int m = 0;
          char *tmp = ftokstr(cone_ev, m, ',');
          while(tmp && !strstr(cone_ev2, tmp))
            tmp = ftokstr(cone_ev, ++m, ',');
          if(tmp) k = -1;
        }
        else if (ev_predc[pre_ev][k] && !confl_evs[k] && !ev_succs[0][k] && !chk_confl)
          conct = link_ev;
        k++;
      }
      // connection to the root
      if (k > numev && !chk_confl && conct)
        path_evs[conct][0] = conct;
      if (k)
        print_pathway(numev, ev_predc, ev_succs, path_evs, 
          cone_ev, cone_ev2, ++minlen, confl_evs, 
          ev_predc[pre_ev][j], link_ev, seq_size, path_seq);
    }
  }
  if (!chk_predc)
    path_evs[pre_ev][0] = pre_ev;
}

int find_conflict(int rows, int cols, int (*ev_confl)[cols],
  int (*ev_confl_copy)[cols], int(*ev_succs)[cols], int ev_cfl, int ev_src ){
  size_t k;
  int found = 0;
  for (k = ev_cfl+1; k < cols && found == 0; k++){
    if(ev_confl[ev_cfl][k] != 0){
      found = find_successor(rows, cols, ev_succs, ev_src, ev_confl[ev_cfl][k]);
      if(found > 0){
        ev_confl_copy[ev_cfl][k] = 0;
      }
      else 
        ev_confl_copy[ev_cfl][k] = find_conflict(rows, cols, ev_confl, ev_confl_copy, ev_succs, 
            ev_confl[ev_cfl][k], ev_src) > 0 ? 0 : k;
      //if (k > ev_src) ev_succs[ev_src][k] = ev_succs[ev_src][k] == 0 ? k : 0;
    }
  }
  return found;
}

void display_matrix(int rows, int cols, int (*matrix)[cols]){
  int i, j;
  for (i=0; i<rows; i++)
  {
      for(j=0; j<cols; j++)
      {
        // upper triangular matrix
        /* if(j >= i){
          printf("%d   ", matrix[i][j]);
        }else{
          printf("    ");
        } */

        // lower triangular matrix
        // if(i >= j) printf("%d   ", matrix[i][j]);
        //printf("%*c", j, ' ');
        // complete matrix
        int tmp = matrix[i][j];
        if (tmp <= 9) printf("%d   ", tmp);
        else printf("%d  ", tmp);
      }
      printf("\n");
  }
}

/**
 * @brief function that reads sequentially an .mci file format
 * 
 * @param mcifile string that corresponds to the needed mcifile.
 */
void read_mci_file_ev (char *mcifile, char* evevfile, int m_repeat, int cutout, char* conf, int pathway, int csv)
{
  #define read_int(x) fread(&(x),sizeof(int),1,mcif)
  /* define a micro substitution to read_int.
    &(x) - is the pointer to a block of memory with a minimum
            size of "sizeof(int)*1" bytes.
    sizeof(int) - is the size in bytes of each element to be read.
    1 - is the number of elements, each one with a size of 
        "sizeof(int)" bytes.
    file - is the pointer to a FILE object that specifies an input stream.
  */

  FILE *mcif, *evevf, *file_nodes, *file_ends;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, qnumcutoff = 0, qnumconfl = 0,
    numco, numev, numpl, numtr, idpl, idtr, sz, i, j, value, ev1, ev2, chk = 0, tmp2 = 0;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0, tmp = 0, _ = 0, count_mrk = 1, seq_size = 0;
  int *co2pl, *ev2tr, *tokens, *queries_co,
    *queries_ev, *cutoffs, *harmfuls, *confl_evs, *leaves_evs, *evs_conf;
  char **plname, **trname, *c;
  char valuech[20] = "", valuechtmp[20] = "";
  char fullfilename[LINE_SIZE], *fname;
  cut_t **cuts;
  evprepost **evprps;


  if (!(mcif = fopen(mcifile,"rb")))
  {
    fprintf(stderr,"cannot read file %s\n",mcifile);
    exit(1);
  }

  if (evevfile)
  {
    if (!(evevf = fopen(evevfile, "r")))
    {
      fprintf(stderr,"cannot read file %s\n",evevfile);
      exit(1);
    }
  }

  if (csv)
  {  
    fname = strtok(mcifile, ".");
    sprintf(fullfilename,"%s_nodes.csv",fname);
    if (!(file_nodes = fopen(fullfilename,"w")))
    {
      fprintf(stderr,
        "cannot write file_in %s\n",fullfilename);
      exit(1);
    }
    fprintf(file_nodes, "id,type,name,tokens,cutoff\n");

    sprintf(fullfilename,"%s_ends.csv",fname);
    if (!(file_ends = fopen(fullfilename,"w")))
    {
      fprintf(stderr,
        "cannot write file_in %s\n",fullfilename);
      exit(1);
    }
    fprintf(file_ends,"type,src,dst\n");
  }

  printf("digraph test {\n"); // start to creating the output 
                              //  file in dot format.

  read_int(numco); // read from input file the total number of conditions.
  read_int(numev); // read from input file the total number of events.

  co2pl = malloc((numco+1) * sizeof(int)); // reserve empty memory for the total number 
                                           // conditions.
  tokens = malloc((numco+1) * sizeof(int)); // reserve the same amount of empty memory
                                            // to save the particular tokens' conditions.
  queries_co = malloc((numco+1) * sizeof(int)); // reserve the same amount of empty memory
                                            // to save the particular queries' conditions.
  queries_ev = malloc((numev+1) * sizeof(int)); // reserve the same amount of empty memory
                                            // to save the particular queries' events.
  ev2tr = malloc((numev+1) * sizeof(int)); // reserve empty memory for the total number 
                                           // events.
  //leaves_evs = calloc(numev+1, sizeof(int)); // collect events with no successors. 
  confl_evs = calloc(numev+1, sizeof(int)); // events that have at least one conflict.

  cutoffs = calloc(numev+1, sizeof(int));
  harmfuls = calloc(numev+1, sizeof(int));
  leaves_evs = calloc(numev+1, sizeof(int)); // collect events with no successors in a evev file
                                          // which are not cutoff events.
  evs_conf = calloc(numev+1, sizeof(int)); // collect events in a configuration from a evev file
                                          
  evprps = calloc(numev+1, sizeof(evprepost*));
  for (i = 0; i <= numev; i++) {
    evprps[i] = malloc(sizeof(evprepost));
    evprps[i]->preset = NULL;
    evprps[i]->postset = NULL;
  }

  int (*co_postsets)[numev+1] = calloc(numco+1, sizeof *co_postsets);
                                           // conditions' postsets to detect conflicts in events.
  if (co_postsets == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
  }
  int (*ev_predc_copy)[numev+1] = calloc(numev+1, sizeof *ev_predc_copy); // matrix to record events' predecesors.
  int (*ev_predc)[numev+1] = calloc(numev+1, sizeof *ev_predc); // matrix to record events' predecesors.
  int (*ev_succs)[numev+1] = calloc(numev+1, sizeof *ev_succs); // matrix to record events' successors.
  int (*ev_confl)[numev+1] = calloc(numev+1, sizeof *ev_confl); // matrix to record events' conflicts.
  int (*ev_confl_copy)[numev+1] = calloc(numev+1, sizeof *ev_confl_copy); // a copy of the previous variable.
  int (*path_evs)[numev+1] = calloc(numev+1, sizeof *path_evs); // matrix to record events' pathway structure.

  read_int(nqure);
  nqure_ = abs(nqure);
  cuts = calloc((szcuts = nqure_+1), sizeof(cut_t*));
  if(nqure_ && !m_repeat)
    dummy = nqure_;
  else if(nqure_ && (m_repeat > szcuts-1 || m_repeat < 0))
    m_repeat = -1;
  while(nqure_)
  {
    read_int(nquszcut);
    read_int(nquszevscut);
    cuts[nqure_] = malloc(sizeof(cut_t));
    cuts[nqure_]->repeat = nqure;
    cuts[nqure_]->szcut = nquszcut;
    cuts[nqure_]->szevscut = nquszevscut;
    cuts[nqure_]->cut = calloc(nquszcut+1, sizeof(int));
    cuts[nqure_]->evscut = calloc(nquszevscut+1, sizeof(int));
    for (i = 1; i <= nquszcut; i++)
      read_int(cuts[nqure_]->cut[i]);
    for (i = 1; i <= nquszevscut; i++)
      read_int(cuts[nqure_]->evscut[i]);
    read_int(nqure);
    nqure_ = abs(nqure);
  }

  for (i = 1; i <= numev; i++){
    read_int(ev2tr[i]); // assign a value to an entry in the array ev2tr for every event
                        // in the unfolding in order to map its respective 
                        // transition, eg., ev1 -> tr3 (ev2tr[1] -> 3)
    read_int(tmp);
    if (!evevfile && m_repeat > 0 && cuts[m_repeat] && cuts[m_repeat]->repeat > 0) 
      queries_ev[i] = tmp; // assign a value to an entry in the array queries_ev for every event
                        // in the unfolding in order to map its respective
                        // query number, eg., ev1 -> 1 (queries_ev[1] -> 1)
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]); // assign a value to the ith entry co2pl array for
                        // every condition in the unfolding in order to
                        // map its respective place, eg., c2 -> pl4 
                        // (co2pl[2] -> 4)
    read_int(tokens[i]); // assign a value to the ith entry tokens array
    read_int(queries_co[i]); // assign a value if the condition is queried but this feature is not displayable.
                         // for every condition in the unfolding in order to
                         // keep track of the conditions that are empty or full
                         // with tokens due to reset arcs.
    read_int(pre_ev); // read the respective event number (mark attribute
                      // in event_t structure definition), if any, which is the only
                      // event in the preset of a condition in the unfolding.
    if (pre_ev && conf && tokens[i] && !cutout)
      clist_add(&evprps[pre_ev]->postset, i);
    else if (!pre_ev && conf && tokens[i] && !cutout)
      clist_add(&evprps[0]->postset, i);
    do {
      read_int(post_ev);  // read the respective event number (mark attribute
                          // in event_t structure definition), if any, which 
                          // are the events in the postset of a condition in
                          // the unfolding.
      /* if (pathway && pre_ev && !post_ev && dummy != pre_ev) 
      {
        leaves_evs[pre_ev] = 1; 
        dummy = pre_ev;
      } */
      if (!cutout && post_ev && tokens[i] && conf)
        clist_add(&evprps[post_ev]->preset, i);
      if(pre_ev && post_ev && ev_succs[pre_ev][post_ev] == 0 && tokens[i]){ 
        ev_predc[post_ev][pre_ev] = pre_ev; // matrix of predeccesors to only print
        if (post_ev == 2)
          printf("ev_predc[srcev][%d] = %d\n", pre_ev, ev_predc[post_ev][pre_ev]);
        ev_predc_copy[post_ev][pre_ev] = pre_ev; // matrix of predeccesors to only print
                                            // immediate predecessors. Comment out if
                                            // you want all dependencies. 
        ev_succs[pre_ev][post_ev] = post_ev; 
        /* Uncomment next 4 lines if you want to see all dependencies in events. */
        // if (cutout && queries_ev[pre_ev] && queries_ev[post_ev])
        //   printf("  e%d -> e%d;\n",pre_ev,post_ev); // write the connection.
        // else if (!cutout)
        //   printf("  e%d -> e%d;\n",pre_ev,post_ev); // write the connection.
      }
      // This else if is necessary to show dependencies to initial cut.
      /* else if (!pre_ev && post_ev && ev_succs[0][post_ev] == 0)
      {
        ev_succs[0][post_ev] = post_ev;
        if (!cutout)
          printf("  e0 -> e%d;\n", post_ev);
        else if (cutout && queries_ev[post_ev])
          printf("  e0 -> e%d;\n", post_ev);
      } */      
      if (post_ev && tokens[i]) co_postsets[i][post_ev] = post_ev; 
    } while (post_ev); // if post_ev is not null.
  }
  /* check immediate connections to events */
  for (int i = 1; i <= numev; i++){
    for (int j = 1; j <= i; j++){
      for (int k = j+1; k <= i; k++)
        if (ev_predc[i][j] > 0 && ev_predc[i][k] > 0)
        {
          if (ev_predc_copy[i][j] == 0) 
            continue;
          else
            ev_predc_copy[i][j] = 
            find_predecessor(numev+1, numev+1, ev_predc, 
              ev_predc[i][j], ev_predc[i][k]) ? 0 : ev_predc[i][j]; 
        }
    }
  }

  if(evevfile)
  {
    tmp = 0;
    if (!m_repeat)
    { 
      while (fscanf(evevf," %s", valuech) != EOF)
        if (strstr(valuech, "+")) // in case of reading redundant events
          strcpy(valuechtmp, valuech);
        else
        {
          value = strtoint(valuech);
          evs_conf[_] = value;
          if (tmp && strcmp(valuechtmp, ""))
          {
            for(i = 0; evs_conf[i] && !chk; i++)
            {
              // check whether previous events (in causal relation) have connections
              // with the event which may lose the link with a redundant event.
              if (ev_predc_copy[value][evs_conf[i]])
                chk = 1;
            }
            if (!chk)
            {
              // if there is no connection with previous events, then
              // connect with the last event before the redundant event.
              ev_predc_copy[value][tmp] = tmp;
              ev_succs[tmp][value] = value;
            }
            // remove connections of the redundant event.
            valuechtmp[strlen(valuechtmp)-1] = '\0';
            tmp2 = strtoint(valuechtmp);
            ev_predc_copy[value][tmp2] = 0;
            strcpy(valuechtmp, "");
            chk = 0;
          }
          
          if (value > 0)
          {
            queries_ev[value] = 1;
            tmp = value;
            _++;
          }
          else if (!value)
          {
            leaves_evs[tmp] = 1;
            //printf("tmp = %d\n", tmp);
            memset(evs_conf, 0,(numev)*sizeof(int));
            _ = 0, tmp = 0;
          }
        }
    }
    else if (m_repeat > 0)
      while (fscanf(evevf," %s", valuech) != EOF && count_mrk <= m_repeat)
      {
        if (strstr(valuech, "+")) // in case of reading redundant events
          strcpy(valuechtmp, valuech);
        else
        {
          value = strtoint(valuech);
          evs_conf[_] = value;
          if (tmp && strcmp(valuechtmp, ""))
          {
            for(i = 0; evs_conf[i] && !chk; i++)
            {
              // check whether previous events (in causal relation) have connections
              // with the event which may lose the link with a redundant event.
              if (ev_predc_copy[value][evs_conf[i]])
                chk = 1;
            }
            if (!chk)
            {
              // if there is no connection with previous events, then
              // connect with the last event before the redundant event.
              ev_predc_copy[value][tmp] = tmp;
              ev_succs[tmp][value] = value;
            }
            // remove connections of the redundant event.
            valuechtmp[strlen(valuechtmp)-1] = '\0';
            tmp2 = strtoint(valuechtmp);
            ev_predc_copy[value][tmp2] = 0;
            strcpy(valuechtmp, "");
            chk = 0;
          }
          tmp = value;
          _++;
          if (value > 0 && !queries_ev[value] && count_mrk == m_repeat)
            queries_ev[value] = 1;
          else if (!value)
          {
            leaves_evs[tmp] = 1;
            memset(evs_conf, 0,(numev)*sizeof(int));
            _ = 0, tmp = 0;
            count_mrk++;
          }
        }
      }
    else
    {
      while (fscanf(evevf," %s", valuech) != EOF)
      {
        if (strstr(valuech, "+")) // in case of reading redundant events
          strcpy(valuechtmp, valuech);
        else
        {
          value = strtoint(valuech);
          evs_conf[_] = value;
          if (tmp && strcmp(valuechtmp, ""))
          {
            for(i = 0; evs_conf[i] && !chk; i++)
            {
              // check whether previous events (in causal relation) have connections
              // with the event which may lose the link with a redundant event.
              if (ev_predc_copy[value][evs_conf[i]])
                chk = 1;
            }
            if (!chk)
            {
              // if there is no connection with previous events, then
              // connect with the last event before the redundant event.
              ev_predc_copy[value][tmp] = tmp;
              ev_succs[tmp][value] = value;
            }
            // remove connections of the redundant event.
            valuechtmp[strlen(valuechtmp)-1] = '\0';
            tmp2 = strtoint(valuechtmp);
            ev_predc_copy[value][tmp2] = 0;
            strcpy(valuechtmp, "");
            chk = 0;
          }
          tmp = value;
          _++;
          if (value > 0 && !queries_ev[value])
            queries_ev[value] = 1;
          else if (!value && strstr(evevfile, ".evco"))
            break;
        }
      }
    }
  }
  else if (m_repeat > 0 && cuts[m_repeat] && cuts[m_repeat]->repeat < 0)
  {
    for (i = 1; i <= cuts[m_repeat]->szcut; i++)
      queries_co[cuts[m_repeat]->cut[i]] = 1;
    for (i = 1; i <= cuts[m_repeat]->szevscut; i++)
      {queries_ev[cuts[m_repeat]->evscut[i]] = 1;
      //printf("e%d, ", cuts[m_repeat]->evscut[i]);
      }
    //printf("\n");
  }
  else if (!m_repeat)
  {
    for (j = 1; j <= dummy; j++)
    {
      for (i = 1; i <= cuts[j]->szcut; i++)
        queries_co[cuts[j]->cut[i]] = 1;
      for (i = 1; i <= cuts[j]->szevscut; i++)
        queries_ev[cuts[j]->evscut[i]] = 1;
    }
  }

  /* print immediate connections to events */
  if (!pathway)
  {
    for (int i = 1; i <= numev; i++){
      for (int j = 1; j <= i; j++){
        if (ev_predc_copy[i][j] > 0)
        {
          if (cutout && queries_ev[i] && queries_ev[j])
          {
            printf("  e%d -> e%d;\n",j,i); // write the connection.
            if(csv) fprintf(file_ends,"\"edge\",\"e%d\",\"e%d\"\n",j, i);
          }
          else if (!cutout)
          {
            printf("  e%d -> e%d;\n",j,i); // write the connection.
            if(csv) fprintf(file_ends,"\"edge\",\"e%d\",\"e%d\"\n",j, i);
          }
        }
      }
    }
  }

  for (;;) {
    read_int(harmful);
    if (!harmful) break;
    harmfuls[harmful] = harmful;
  }
  
  if (!cutout || !m_repeat)
  {  
    // A loop over co_postsets matrix to fill ev_confl matrix with conflicts
    // among events part of the same condition's postset. We make a copy 
    // conflicts to find immediate conflicts correctly in the function right after.
    for (size_t i = 1; i <= numco; i++){
      for (size_t j = 1; j <= numev-1; j++){
        for (size_t k = j+1; k <= numev; k++){
          ev1 = co_postsets[i][j]; ev2 = co_postsets[i][k];
          if (cutout && queries_ev[j] && queries_ev[k] && 
              ev1 != 0 && ev2 != 0 && ev_confl[ev1][ev2] == 0)
          {
            ev_confl[ev1][ev2] = ev2;
            ev_confl_copy[ev1][ev2] = ev2;
          }
          else if (!cutout && ev1 != 0 && ev2 != 0 && ev_confl[ev1][ev2] == 0)
          {
            ev_confl[ev1][ev2] = ev2;
            ev_confl_copy[ev1][ev2] = ev2;
          }
        }
      }
    }
    // A loop over ev_confl matrix to find two pairwise conflict between events, eg.,
    // e2->e3 and e3->e4, thereafter with find_successor we use ev_succs to check 
    // if e4 is a successor of e2, if yes we remove the conflict between e4 and e3
    // because the conflict is inherited from e2 and e3, so we leave only immediate
    // conflicts. Finally, we use the matrix ev_confl_copy to remove those "redundant
    // conflicts" while ev_confl remains unchangeable to keep the trace of conflicts
    // for posterior relations.
    //display_matrix(numev+1, numev+1, ev_confl_copy);
    for (size_t i = 1; i <= numev; i++)
      for (size_t j = i+1; j <= numev; j++)
        if (cutout && queries_ev[i] && queries_ev[j] && ev_succs[i][j] == 0)
          ev_succs[i][j] = find_successor(numev+1, numev+1, ev_succs, i, j);
        else if(!cutout && ev_succs[i][j] == 0)
          ev_succs[i][j] = find_successor(numev+1, numev+1, ev_succs, i, j);
    
    
    for (size_t i = 1; i <= numev; i++){
      for (size_t j = i+1; j <= numev; j++){
        if(cutout && queries_ev[i] && queries_ev[j] && ev_confl[i][j] > 0)
        {
          for(size_t k = i+1; k <= numev; k++)
            if(ev_succs[i][k] > 0){
              if(k > j) ev_confl_copy[j][k] = 0;
              else ev_confl_copy[k][j] = 0;
            }
              //{ev_confl_copy[j][k] = k > j ? ev_confl[j][k] > 0;}
          for(size_t k = j+1; k <= numev; k++)
            if(ev_succs[j][k] > 0){
              if(k > i) ev_confl_copy[i][k] = 0;
              else ev_confl_copy[k][i] = 0;
            }

          for (size_t k = i+1; k <= numev; k++)
            for (size_t m = j+1; m <= numev; m++)
              if(ev_succs[i][k] > 0 && ev_succs[j][m] > 0){
                if(k > m) ev_confl_copy[m][k] = 0;
                else ev_confl_copy[k][m] = 0;
              }
        }
        else if(!cutout && ev_confl[i][j] > 0)
        {
          for(size_t k = i+1; k <= numev; k++)
            if(ev_succs[i][k] > 0){
              if(k > j) ev_confl_copy[j][k] = 0;
              else ev_confl_copy[k][j] = 0;
            }
              //{ev_confl_copy[j][k] = k > j ? ev_confl[j][k] > 0;}
          for(size_t k = j+1; k <= numev; k++)
            if(ev_succs[j][k] > 0){
              if(k > i) ev_confl_copy[i][k] = 0;
              else ev_confl_copy[k][i] = 0;
            }

          for (size_t k = i+1; k <= numev; k++)
            for (size_t m = j+1; m <= numev; m++)
              if(ev_succs[i][k] > 0 && ev_succs[j][m] > 0){
                if(k > m) ev_confl_copy[m][k] = 0;
                else ev_confl_copy[k][m] = 0;
              }
        }
      }
    }
  }
  /* Printing event corresponding to the initial cut. We make use of 
  the empty vector of index 0 in ev_succs matrix to collect all events 
  that have predecesors and hence to depict the arcs in the resulting 
  dot file */
  for (int i = 1; i <= numev; i++){
    for (int j = i+1; j <= numev; j++){
      if (ev_succs[i][j] > 0 && ev_succs[0][j] == 0) 
        ev_succs[0][j] = ev_succs[i][j];
    }
  }
  
  if (!pathway)
  {
    for (int i = 1; i <= numev; i++){
      if (!cutout && ev_succs[0][i] == 0) 
      {
        printf("  e0 -> e%d;\n", i);
        if(csv) fprintf(file_ends,"\"edge\",\"e0\",\"e%d\"\n", i);
      }
      else if (cutout && ev_succs[0][i] == 0 && queries_ev[i])
      {
        printf("  e0 -> e%d;\n", i);
        if(csv) fprintf(file_ends,"\"edge\",\"e0\",\"e%d\"\n", i);
      }
    }
  }

  if(!cutout || !m_repeat)
  {  
    printf("\n//conflicts\n");
    //display_matrix(numev+1, numev+1, ev_succs);
    //display_matrix(numev+1, numev+1, ev_confl_copy);
    // After leaving only immediate conflicts we do a loop over ev_confl
    // to write in the output file those conflict relations.
    for (int i = 1; i <= numev; i++){
      for (int j = i+1; j <= numev; j++)
      {
        if (cutout && queries_ev[i] && queries_ev[j] && ev_confl_copy[i][j])
        {
          confl_evs[i] = j;
          confl_evs[j] = i;
          qnumconfl += 2;
          printf("  e%d -> e%d [arrowhead=none color=gray60 style=dashed constraint=false];\n",i,ev_confl_copy[i][j]);
          if(csv) fprintf(file_ends,"\"conflict\",\"e%d\",\"e%d\"\n", i,ev_confl_copy[i][j]);
        }
        else if (!cutout && ev_confl_copy[i][j])
        {
          if (!evevfile && pathway)
          {
            confl_evs[i] = j;
            confl_evs[j] = i;
            qnumconfl += 2;
          }
          printf("  e%d -> e%d [arrowhead=none color=gray60 style=dashed constraint=false];\n",i,ev_confl_copy[i][j]);
          if(csv) fprintf(file_ends,"\"conflict\",\"e%d\",\"e%d\"\n", i,ev_confl_copy[i][j]);
        }
      }
    }
    printf("\n");
  }

  for (;;) {
    read_int(cutoff);
    if (!cutoff) break;
    cutoffs[cutoff] = cutoff;
    if (queries_ev[cutoff]) qnumcutoff++;
    else if (!evevfile && pathway) qnumcutoff++;
#ifdef CUTOFF
    printf("  e%d [style=filled];\n",cutoff);
#endif
    read_int(dummy);
#ifdef CUTOFF
    printf("  e%d [style=dashed];\n",dummy);
#endif
  }


  if (pathway)
  {
    seq_size = (qnumconfl+qnumcutoff)*2;
    int path_seq[seq_size];
    memset(path_seq, 0, sizeof(path_seq));
    char cone_ev[seq_size*seq_size];
    char cone_ev2[seq_size*seq_size];
    /* printf("qnumconfl: %d\n", qnumconfl);
    printf("qnumcutoff: %d\n", qnumcutoff); */
    dummy = 0;
    tmp = 1;
    int k, dummy2 = 0;
    for (i = 1; i <= numev; i++)
      if ((queries_ev[i] && cutoffs[i]) || (!evevfile && cutoffs[i]))
      {
        path_seq[dummy] = i;
        // printf("1. path_seq[%d]: %d\n", dummy, i);
        dummy++;
      }
      else if ((queries_ev[i] && !leaves_evs[i] && !cutoffs[i]) ||
      (!evevfile && !cutoffs[i]))
      {
        for (j = i+1; j <= numev && tmp; j++)
        {
          if ((ev_succs[i][j] && queries_ev[j]) || 
              (!evevfile && ev_succs[i][j]))
            tmp = 0;
          /* if (i == 5)
            printf("ev_succs[%d][%d]: %d\n", i, j, ev_succs[i][j]); */
        }
        /* if (i == 5)
          printf("tmp = %d, leaves_evs[i] = %d, path_seq[%d] = %d\n", tmp, leaves_evs[i], dummy, path_seq[dummy]); */
        if (tmp && !leaves_evs[i] && !path_seq[dummy])
        {
          path_seq[dummy] = i;
          // printf("2. path_seq[%d]: %d\n", dummy, i);
          leaves_evs[i] = 1;
          dummy++;
        }
        else 
          tmp = 1;
      }
      // if evev file is given then leaves have been already retrieved.
      // And they will be the starting point regardless whether
      // they are cutoffs.
      else if (queries_ev[i] && leaves_evs[i])
      {
        path_seq[dummy] = i;
        // printf("3. path_seq[%d]: %d\n", dummy, i);
        dummy++;
      }
    dummy = 0;
    for (i = 0; i < seq_size; i++)
      if (path_seq[i])
        print_pathway(numev+1, ev_predc_copy, ev_succs, 
          path_evs, cone_ev, cone_ev2, 1, confl_evs, 
          path_seq[i], path_seq[i], seq_size, path_seq);
    
    memset(path_seq, 0, sizeof(path_seq));
    for (i = 1; i <= numev; i++)
      if (path_evs[i][0] && !dummy)
      {
        printf("  e0 -> e%d [minlen=%d];\n", path_evs[i][0], compute_cone_height(numev+1,ev_predc_copy,path_evs[i][0],1));
        if(csv) fprintf(file_ends,"\"edge\",\"e0\",\"e%d\"\n", path_evs[i][0]);
        path_seq[0] = path_evs[i][0];
        dummy++;
      }
      else if (path_evs[i][0] && dummy)
      {
        for (k = 0; k < seq_size; k++)
        {
          for (j = 1; j <= numev && !dummy2; j++)
            if (path_seq[k] && path_evs[path_seq[k]][j] == path_evs[i][0])
              dummy2 = 1;
        }
        if (!dummy2) 
        {
          printf("  e0 -> e%d [minlen=%d];\n", path_evs[i][0], compute_cone_height(numev+1,ev_predc_copy,path_evs[i][0],1));
          if(csv) fprintf(file_ends,"\"edge\",\"e0\",\"e%d\"\n", path_evs[i][0]);
        }
        path_seq[dummy] = path_evs[i][0];
        dummy++;
        dummy2 = 0;
      }
      else if (i == numev)
        printf("\n");
  }

  do { read_int(dummy); } while(dummy);
  read_int(numpl);
  read_int(numtr);
  read_int(sz);

  plname = malloc((numpl+2) * sizeof(char*));
  trname = malloc((numtr+2) * sizeof(char*));

  for (i = 1; i <= numpl+1; i++) plname[i] = malloc(sz+1);
  for (i = 1; i <= numtr+1; i++) trname[i] = malloc(sz+1);

  for (i=1; i <= numpl; i++)
  {
    read_int(idpl);
    c = plname[idpl];
    do { fread(c,1,1,mcif); } while (*c++);
  }
  fread(c,1,1,mcif);

  for (i=1; i <= numtr; i++)
  {
    read_int(idtr);
    c = trname[idtr];
    do { fread(c,1,1,mcif); } while (*c++);
  }
  fread(c,1,1,mcif);

  int *cut = calloc(numco+1, sizeof(int));
  /* firing sequence */
  int *frsq = calloc(numev+1, sizeof(int));

  if(conf)
  {  
    clist_t *list;
    char* sub;
    int coid;
    int subint;
    char* conf_copy = strdup(conf);
    /* Add initial cut */
    for(list = evprps[0]->postset; list; list = list->next)
    {
      coid = list->idcond;
      cut[coid] = coid;
    }

    sub = strtok(conf_copy, ",");
    while (sub != NULL)
    {
      subint = strtoint(sub);
      frsq[subint] = subint;
      /* Remove event's preset */
      for(list = evprps[subint]->preset; list; list = list->next)
      {
        coid = list->idcond;
        cut[coid] = 0;
      }
      /* Add event's poset */
      for(list = evprps[subint]->postset; list; list = list->next)
      {
        coid = list->idcond;
        cut[coid] = coid;
      }
      sub = strtok(NULL, ",");
    }
  }

  char color1[] = "#cce6cc"; // or "palegreen";
  char color2[] = "cornflowerblue";
  char color3[] = "orange";
  char color4[] = "black";
  char color5[] = "firebrick2";
  char color6[] = "#409f40";
  char *fillcolor, *color;

  for (i = 1; i <= numev; i++)
  {
    if (pathway && queries_ev[i] && (confl_evs[i] || cutoffs[i] || leaves_evs[i]))
    {
      if (i == harmfuls[i])
        fillcolor = color5;
      else if (i == cutoffs[i])
        fillcolor = color2;
      else
        fillcolor = queries_ev[i] ? color3 : color1;

      color = queries_ev[i] ? color4 : color6;

      printf("  e%d [color=\"%s\" fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
            i, color, fillcolor, queries_ev[i] ? color3 : fillcolor, trname[ev2tr[i]], i);
      if (csv) 
        fprintf(file_nodes,"\"e%d\",\"event\",\"%s\",,\"%d\"\n",i,trname[ev2tr[i]],i == cutoffs[i] ? 1 : 0);
      
    }
    else if (cutout && !pathway && queries_ev[i])
    {
      if (i == harmfuls[i])
        fillcolor = color5;
      else if (i == cutoffs[i])
        fillcolor = color2;
      else
        fillcolor = queries_ev[i] ? color3 : color1;

      color = queries_ev[i] ? color4 : color6;

      printf("  e%d [color=\"%s\" fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
            i, color, fillcolor, queries_ev[i] ? color3 : fillcolor, trname[ev2tr[i]], i);
      if (csv) 
        fprintf(file_nodes,"\"e%d\",\"event\",\"%s\",,\"%d\"\n",i,trname[ev2tr[i]],i == cutoffs[i] ? 1 : 0);
    }
    else if (!cutout)
    {
      int queries_ev_or_frsq = queries_ev[i] || frsq[i];
      if (i == harmfuls[i])
        fillcolor = color5;
      else if (i == cutoffs[i])
        fillcolor = color2;
      else
        fillcolor = queries_ev_or_frsq ? color3 : color1;

      color = queries_ev_or_frsq || harmfuls[i] || cutoffs[i] ? color4 : color6;

      printf("  e%d [color=\"%s\" fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
          i, color, fillcolor, queries_ev_or_frsq ? color3 : fillcolor, trname[ev2tr[i]], i);
      if (csv) 
        fprintf(file_nodes,"\"e%d\",\"event\",\"%s\",,\"%d\"\n",i,trname[ev2tr[i]],i == cutoffs[i] ? 1 : 0);
    }
  }
  printf("  e0 [fillcolor=\"white\" label=\"⊥\" shape=box style=filled];\n");
  printf("}\n");

  fclose(mcif);
  if (evevfile) fclose(evevf);
}

void usage ()
{
  fprintf(stderr,"usage: mci2dot_ev [options] <mcifile> <evevfile>\n\n"

    "     Options:\n"
    "      -c --cutout  if a marking is queried or \n                  part of a reachability check then\n                  it will show a cutout of\n                  the whole unfolding\n"
    "      -p --pathway   display pathway structure instead of event structure."
    "      -r <instance>  highlight <instance> of a repeated marking - <0> will show all instances. \n"
    "      -cf <confg>:   used to return the marking led \n by the configuration <confg>(string type).\n You cannot enable cutouts and this \n flag at the same time.\n"
    "      -csv           create csv files to include the event struture, and its corresponding conflict file.\n\n"

    "<evevfile> is an optional file whose lines contain\n"
    "the IDs of firing sequences of events, each line ends with a 0\n"
    "indicating the end of the line. This file is similar to <evcofile>\n"
    "used in <mci2dot>\n\n");
    exit(1);
}

int main (int argc, char **argv)
{
  int i, m_repeat = -1, cutout = 0, pathway = 0, csv = 0;
  char *mcifile = NULL, *evevfile = NULL;
  char *configuration = NULL;

  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-r"))
    {
      if (++i == argc) usage();
      m_repeat = atoi(argv[i]);
    }
    else if (!strcmp(argv[i], "-cf"))
    {
      if (++i == argc) usage();
      configuration = argv[i];
    }
    else if (!strcmp(argv[i],"-c") || !strcmp(argv[i],"--cutout"))
      cutout = 1;
    else if (!strcmp(argv[i],"-p") || !strcmp(argv[i],"--pathway"))
      pathway = 1 ;
    else if (!strcmp(argv[i],"-csv"))
      csv = 1 ;
    else if (!mcifile)
      mcifile = argv[i];
    else
      evevfile = argv[i];

  if (!mcifile || (cutout && configuration)) usage();
  read_mci_file_ev(mcifile, evevfile, m_repeat, cutout, configuration, pathway, csv);
  exit(0);
}
