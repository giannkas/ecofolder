/****************************************************************************/
/* netconv.h                                                                */
/*                                                                          */
/* Definitions for netconv.c, readpep.c, nodelist.c, mci.c		    */
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
	struct nodelist_t *reset; /* unordered list of reset transitions            */                  //*** NEW  ***//
	struct nodelist_t *conds;   /* conditions derived from this place   */
	struct nodelist_t *ctxset; /* unordered list of context transitions            */                  //*** NEW  ***//
	char marked;		    /* non-zero if place is marked	    */
} place_t;

typedef struct trans_t
{
	struct trans_t *next;
	char  *name;		    /* short name			    */
	int    num;		    /* number				    */
	struct nodelist_t *preset;  /* unordered list of preset		    */
	struct nodelist_t *postset; /* unordered list of postset	    */
	struct nodelist_t *reset; /* unordered list of reset	    */			//*** NEW  ***//
	struct nodelist_t *ctxset; /* unordered list of context transitions            */                  //*** NEW  ***//
	short  preset_size, postset_size, reset_size, 
		prereset_size, postreset_size, ctxset_size;							//*** NEW  ***//
} trans_t;

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
	short  foata_level;
	short  preset_size, postset_size;
} event_t;

/****************************************************************************/
/* structures for net and unfolding					    */

typedef struct
{
	place_t *places;	/* pointer to first place		*/
	trans_t *transitions;	/* pointer to first transition		*/ 
	int numpl, numtr;	/* number of places/transitions in net	*/
	int maxpre, maxpost, maxres, maxctx;	/* maximal size of a t-pre/postset,reset and ctxset	*/
} net_t;

typedef struct
{
	cond_t *conditions;	/* pointer to first condition		*/
	event_t *events;	/* pointer to first event		*/ 
	int numco, numev;	/* number of conditions/events in net	*/
	struct nodelist_t *m0;	/* list of minimal conditions		*/
	struct nodelist_t *m0_unmarked;	/* list of minimal unmarked conditions		*/
} unf_t;

/****************************************************************************/

extern net_t* nc_create_net ();
extern unf_t* nc_create_unfolding ();
extern place_t* nc_create_place (net_t*);
extern trans_t* nc_create_transition (net_t*);
extern void nc_create_arc (struct nodelist_t**,struct nodelist_t**,void*,void*);
extern void nc_compute_sizes (net_t*);
extern void nc_static_checks (net_t*,char*);
extern int nc_same_condition (cond_t*, int);
extern int nc_check_unmarked_initial_marking (cond_t*, char*);

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
extern char nodelist_compare (nodelist_t*, nodelist_t*);
extern int sizeList(nodelist_t *);
extern nodelist_t* nodelist_concatenate(nodelist_t *list1, nodelist_t *list2);			//*** NEW  ***//
extern int nodelist_find (nodelist_t *list, void *);
extern int nodelist_common(cond_t*, nodelist_t*, cond_t*, trans_t*, place_t*);								//*** NEW  ***//

/****************************************************************************/
/* declarations for readpep.c						    */

extern net_t* read_pep_net(char*);
extern net_t* reset_complement(net_t*);
extern net_t* pr_encoding(net_t*);

/****************************************************************************/
/* declarations for mci.c						    */

extern void write_mci_file (char*);

#endif
