/****************************************************************************/
/* unfold.h								    */
/*                                                                          */
/* Definitions for unfold.c, marking.c, pe.c, order.c			    */
/****************************************************************************/

#ifndef __UNFOLD_H__
#define __UNFOLD_H__

#include <stdlib.h>
#include "netconv.h"

/****************************************************************************/
/* declarations from unfold.c						    */

extern net_t *net;	/* stores the net	*/
extern unf_t *unf;	/* stores the unfolding */
extern hashcell_t **hash; /* stores the hash table for markings */

extern int conditions_size, events_size;
extern int ev_mark;		  	   /* needed for various procedures */
extern int hash_buckets;
extern event_t **events;

extern int exitcode;		/* proposed exit code of the program         */
extern trans_t *stoptr;		/* the transition mentioned by the -T switch */
extern int unfold_depth;	/* argument of the -d parameter */
extern int interactive;		/* whether the -i switch has been supplied */
extern int compressed;		/* whether the -c switch has been supplied */
extern int mcmillan;		/* whether the -mcmillan switch is enabled */
extern int m_repeat;		/* whether the -r switch has been supplied */

extern nodelist_t *cutoff_list, *corr_list;  /* cut-off/corresponding list */

extern void unfold();	/* the unfolding procedure */
extern cond_t* insert_condition(place_t*, event_t*, int);  /* insert a condition to the unfolding */
extern coa_t alloc_coarray (int);	/* Create an empty coarray with 'size' allocated events. */
extern void print_conditions (cond_t*);
extern void print_events (event_t*);

/***************************************************************************/
/* declarations for marking.c						   */

extern void marking_init ();
extern int repeated_marking(nodelist_t*);
extern int find_marking (nodelist_t*, int);
extern int add_marking (nodelist_t*,event_t*);
extern nodelist_t* marking_initial ();
extern nodelist_t* format_marking_query ();
extern void print_marking_pl (nodelist_t*);
extern void print_marking_co (nodelist_t*);


/***************************************************************************/
/* declarations for pe.c						   */

/* An entry in the queue of possible extensions. Contains information
   relevant to the chosen adequate total ordering.			*/
typedef struct pe_queue_t
{
	int lc_size;			/* size of local configuration	*/
        struct parikh_t *p_vector;	/* Parikh vector		*/
	trans_t	 *trans;		/* transition       		*/
	cond_t  **conds;		/* input conditions 		*/
	nodelist_t *marking;		/* marking associated with [e]  */
	int id;
} pe_queue_t;

extern cond_t **pe_conds;	/* array of conditions */

extern void pe_init (nodelist_t*);
extern pe_queue_t* pe_pop (int);
extern void pe_free (pe_queue_t*);
extern void pe_finish ();
extern void pe (cond_t*);

extern int pe_qsize;
extern pe_queue_t **pe_queue;

/***************************************************************************/
/* declarations for order.c						   */

typedef struct parikh_t
{
	unsigned short int tr_num;
	unsigned short int appearances;
} parikh_t;

extern void parikh_init ();
extern void parikh_finish ();
extern pe_queue_t* create_queue_entry (trans_t*);
extern int pe_compare (pe_queue_t*,pe_queue_t*);
extern int find_foata_level (pe_queue_t*);

#endif
