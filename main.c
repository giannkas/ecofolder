#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

/*****************************************************************************/

void usage(char *myname)
{
  fprintf(stderr,
    "%s -- low level reset net unfolder\n\n"
    "Usage: %s [options] <LLnetfile> [FileOptions]\n\n"

  "     Options:\n"
  "      -T <name>        stop when transition <name> is inserted.\n"
  "      -d <depth>       unfold up to given <depth>.\n"
  "      -i               interactive mode.\n"
  "      -r <instance>    highlight <instance> of a repeated marking.\n"
  "      -c               compressed view\n"
  "      -mcmillan        unfolds with mcmillan criteria.\n"
  "      -confmax         when used, it will enable interactive mode to display maximal configurations only.\n"
  "      -freechk         used to check freeness. When used, you should enable -badchk <badunf> to\n                       do a proper freeness check, otherwise you will have the initial prefix.\n                       The parameter will enable -mcmillan flag as well.\n                       It cannot be used with -T <name> option."
  "      -badchk <badunf> used to check badness <badunf> is a mci file containing an unfolding\n                       prefix of the corresponding bad net.\n"
  "      -verbose         if used, Ecofolder will print information concerning the prefix produced.\n"
  "      -useids         when used, Ecofolder will use ids for places and transitions given in\n                       the input file.\n\n"

  "     FileOptions:\n"
  "      -m <filename>  file to store the unfolding in\n\n"

  "Unless specified otherwise, all filenames will default to\n"
  "the basename of <LLnetfile> plus appropriate extensions.\n\n"

  "Version 2.0.0 (08.11.2023)\n", myname, myname);

  exit(1);
}

/*****************************************************************************/

int main (int argc, char **argv)
{
  int  i, given_name = 0;
  char    *llnet = NULL, *mcifile;
  char    **dptr = &llnet;
  char	*tmpname, *idx;
  char    *stoptr_name = NULL;

  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-m"))
    {
      dptr = &mcifile; 
      given_name = 1;
    }
    else if (!strcmp(argv[i],"-T"))
    {
      if (++i == argc) usage(argv[0]);
      stoptr_name = argv[i];
    }
    else if (!strcmp(argv[i],"-d"))
    {
      if (++i == argc) usage(argv[0]);
      unfold_depth = atoi(argv[i]);
    }
    else if (!strcmp(argv[i],"-i"))
      interactive = 1;
    else if (!strcmp(argv[i],"-confmax"))
      confmax = interactive = 1;
    else if (!strcmp(argv[i],"-r"))
    {
      if (++i == argc) usage(argv[0]);
      m_repeat = atoi(argv[i]);
    }
    else if (!strcmp(argv[i],"-c"))
      compressed = 1;
    else if (!strcmp(argv[i],"-mcmillan"))
      mcmillan = 1;
    else if (!strcmp(argv[i],"-verbose"))
      verbose = 1;
    else if (!strcmp(argv[i],"-useids"))
      useids = 1;
    else if (!strcmp(argv[i],"-badchk"))
    {
      if (++i == argc) usage(argv[0]);
      badunf = argv[i];
    }
    else if (!strcmp(argv[i],"-freechk"))
    {
      freechk = 1;
      mcmillan = 1;
    }
    else
    {
      if (!dptr) usage(argv[0]);
      *dptr = argv[i];

      tmpname = MYmalloc(strlen(argv[i])+5);
      strcpy(tmpname,argv[i]);
      idx = strrchr(tmpname,'.');
      if (!idx) strcat(tmpname,".");
      idx = strrchr(tmpname,'.');

      if (dptr == &llnet)
      {
        if(!given_name) strcpy(idx,"_unf.mci");
        else strcpy(idx,".mci");
        mcifile = MYstrdup(tmpname);
      }

      dptr = NULL;
    }

  if (!llnet) usage(argv[0]);
  
  /* if (data)
    if(interactive || confmax) 
      usage(argv[0]); */
  if (stoptr_name && freechk) usage(argv[0]);
  
  net = read_pep_net(llnet);

  nc_static_checks(net,stoptr_name);
  nc_create_trans_pool(net);
  /* creating transitions pool that are enforced by restrictions */
  if(verbose)
  {
    /* Reset set of the transitions */
    printf("Reset set of the transitions\n");
    trans_t *trr = net->transitions;
    while (trr){
      printf("%s:\n", trr->name);
      nodelist_t *ptr = trr->reset;
      while (ptr){
        printf("  %s\n",((place_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      trr = trr->next;
    }
    printf("\n");

    /* Context set of the transitions */
    printf("Context set of the transitions\n");
    trr = net->transitions;
    while (trr){
      printf("%s:\n", trr->name);
      nodelist_t *ptr = trr->ctxset;
      while (ptr){
        printf("  %s\n",((place_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      trr = trr->next;
    }
    printf("\n");

    /* Preset of the transitions */
    printf("Preset of the transitions\n");
    trr = net->transitions;
    while (trr){
      printf("%s:\n", trr->name);
      nodelist_t *ptr = trr->preset;
      while (ptr){
        printf("  %s\n",((place_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      trr = trr->next;
    }
    printf("\n");

    /* Postset of the transitions */
    printf("Postset of the transitions\n");
    trr = net->transitions;
    while (trr){
      printf("%s:\n", trr->name);
      nodelist_t *ptr = trr->postset;
      while (ptr){
        printf("  %s\n",((place_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      trr = trr->next;
    }
    printf("\n");

    /* Reset set of the places */

    printf("Reset set of the places\n");
    place_t *pll = net->places;
    while (pll){
      printf("%s:\n", pll->name);
      nodelist_t *ptr = pll->reset;
      while (ptr){
        printf("  %s\n",((trans_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      pll = pll->next;
    }
    printf("\n");

    /* Context set of the places */

    printf("Context set of the places\n");
    pll = net->places;
    while (pll){
      printf("%s:\n", pll->name);
      nodelist_t *ptr = pll->ctxset;
      while (ptr){
        printf("  %s\n",((trans_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      pll = pll->next;
    }
    printf("\n");

    /* Preset  of the places */
    printf("Preset  of the places\n");
    pll = net->places;
    while (pll){
      printf("%s:\n", pll->name);
      nodelist_t *ptr = pll->preset;
      while (ptr){
        printf("  %s\n",((trans_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      pll = pll->next;
    }
    printf("\n");

    /* Postset set of the places */
    printf("Postset set of the places\n");
    pll = net->places;
    while (pll){
      printf("%s:\n", pll->name);
      nodelist_t *ptr = pll->postset;
      while (ptr){
        printf("  %s\n",((trans_t*)(ptr->node))->name);
        ptr = ptr->next;
      }
      printf("\n");
      pll = pll->next;
    }
    printf("\n");

    /* Restrictions of net's unfolding */
    printf("Restrictions of net's unfolding\n");
    restr_t *rtt = net->restrictions;
    while (rtt){
      printf("  %s\n", rtt->name);
      rtt = rtt->next;
    }
    printf("\n");
  }

  unfold();
  nc_create_ignored_trans(net);

  if(verbose)
  {  
    printf("Restricted transitions to fire: %s\n", net->rt_trans);
    printf("Fired transitions in the unfolding: %s\n", net->unf_trans);
    printf("Transitions without restrictions but never enabled: %s\n", net->ign_trans);
  }
  if (compressed)
    write_mci_file_cpr(mcifile);
  else
    write_mci_file(mcifile);
  
  
  if(badunf && freechk) printf("%d", exitcode);
  return 0;
}
