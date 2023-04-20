/****************************************************************************/
/* netconv.h                                                                */
/*                                                                          */
/* Definitions for netconv.c, readpep.c, nodelist.c, mci.c, mci_cpr.c    */
/****************************************************************************/

#ifndef __NETCONV_H__
#define __NETCONV_H__

/****************************************************************************/

typedef unsigned char uchar;

/****************************************************************************/
/* structures for places, transitions, conditions, events		    */

/* If you change these structures, do keep the "next" field at the top;
   reverse_list() depends on it.					    */

typedef struct place_t
{
  struct place_t *next;
  char  *name;		    /* short name			    */
  int    num;		    /* number				    */
  struct nodelist_t *preset;  /* unordered list of preset		    */
  struct nodelist_t *postset; /* unordered list of postset	    */
  struct nodelist_t *reset; /* unordered list of reset transitions */
  struct nodelist_t *conds;   /* conditions derived from this place   */
  struct nodelist_t *ctxset; /* unordered list of context transitions */
  char marked;		    /* non-zero if place is marked	    */
  int queried;      /* non-zero if a place is queried */
} place_t;

typedef struct trans_t
{
  struct trans_t *next;
  char  *name;		    /* short name			    */
  int    num;		    /* number				    */
  struct nodelist_t *preset;  /* unordered list of preset		    */
  struct nodelist_t *postset; /* unordered list of postset	    */
  struct nodelist_t *reset; /* unordered list of reset */
  struct nodelist_t *ctxset; /* unordered list of context transitions */
  short  preset_size, postset_size, reset_size, 
    prereset_size, postreset_size, ctxset_size;
  char blocked;
} trans_t;

typedef struct restr_t
{
  struct restr_t *next;
  char  *name;		    /* short name	which should correspond 
                        to a least one place name*/
  int    num;		    /* number				    */
} restr_t;

typedef struct coa_t {
  int size;
  int inuse;
  struct cond_t **conds;
} coa_t;

typedef struct cond_t
{
  struct cond_t *next;
  struct event_t *pre_ev;	    /* the single event in the preset	    */
  struct nodelist_t *postset; /* unordered list of postset	    */
  struct place_t *origin;	    /* associated place			    */
  int    num;		    /* number (needed by co_relation)	    */
  int    mark;		    /* used by marking_of		    */
  int token; 				/* used by conditions to represent firings */
  int flag;         /* used to format view when compressing conditions */
  int queried;		    /* non-zero if cond-cut is queried	    */
  coa_t  co_common;	    /* list of co-conditions		    */
  coa_t  co_private;	    /* list of co-conditions		    */
} cond_t;

typedef struct event_t
{
  struct event_t *next;
  struct cond_t **preset;	    /* array of preset/postset conditions   */
  struct cond_t **postset;    /* size fixed by sizes of origin	    */
  struct trans_t *origin;	    /* associated transition		    */
  int    mark;		    /* used by marking_of		    */
  coa_t  coarray;
  int    id;
  int queried;		    /* non-zero if the event has a causal relation with a queried marking	    */
  short  foata_level;
  short  preset_size, postset_size;
} event_t;

/****************************************************************************/
/* structures for net and unfolding					    */

typedef struct
{
  place_t *places;	/* pointer to first place		*/
  trans_t *transitions;	/* pointer to first transition		*/
  restr_t *restrictions; /* constraints to restrict net's unfolding */
  int numpl, numtr, numrt, numqr;	/* number of places/transitions in net	*/
  int maxpre, maxpost, maxres, maxctx;	/* maximal size of a t-pre/postset, reset and ctxset	*/
  int maxplname, maxtrname; /* maximal size of place and transition names */
  char *rt_trans;		    /* word of transitions restricted to fire according to restrictions */
  char *unf_trans; /* word of fired transitions when unfolding */
  char *ign_trans; /* word of ignored transitions because they were never enable */
} net_t;

typedef struct
{
  cond_t *conditions;	/* pointer to first condition		*/
  event_t *events;	/* pointer to first event		*/ 
  int numco, numev;	/* number of conditions/events in net	*/
  struct nodelist_t *m0;	/* list of minimal conditions		*/
  struct nodelist_t *m0_unmarked;	/* list of minimal unmarked conditions		*/
} unf_t;

typedef struct hashcell_t
{
  struct nodelist_t *marking;
  struct nodelist_t *pre_evs;
  int repeat;
  struct hashcell_t *next;
} hashcell_t;

typedef struct querycell_t
{
  struct cond_t *cut;
  struct querycell_t *next;  
} querycell_t;


/****************************************************************************/

extern net_t* nc_create_net ();
extern unf_t* nc_create_unfolding ();
extern place_t* nc_create_place (net_t*);
extern trans_t* nc_create_transition (net_t*);
extern restr_t* nc_create_restriction(net_t*);
extern void nc_create_arc (struct nodelist_t**,struct nodelist_t**,void*,void*);
extern void nc_compute_sizes (net_t*);
extern void nc_static_checks (net_t*,char*);
extern void nc_create_trans_pool (net_t*);
extern void nc_create_ignored_trans (net_t*);


/*****************************************************************************/
/* declarations from nodelist.c						     */

typedef struct nodelist_t
{
  void   *node;
  struct nodelist_t* next;
} nodelist_t;

extern nodelist_t* nodelist_alloc ();
extern nodelist_t* nodelist_push (nodelist_t**list,void*);
extern nodelist_t* nodelist_insert (nodelist_t**,void*e);
extern void nodelist_delete (nodelist_t*);
extern int nodelist_compare (nodelist_t*, nodelist_t*);
extern nodelist_t* nodelist_concatenate(nodelist_t *, nodelist_t *);
extern int nodelist_find (nodelist_t *, void *);
extern int nodelist_size (nodelist_t *);

/****************************************************************************/
/* declarations for readpep.c						    */

extern net_t* read_pep_net(char*);
extern net_t* reset_complement(net_t*);
extern char* pr_encoding(char*);

/****************************************************************************/
/* declarations for mci.c and mci_cpr.c						    */

extern void write_mci_file (char*);
extern void write_mci_file_cpr (char*);
extern void* reverse_list (void *);

#endif
