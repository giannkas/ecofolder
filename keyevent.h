/****************************************************************************/
/* keyevent.h                                                                */
/*                                                                          */
/* Definitions for keyevent.c		    */
/****************************************************************************/

#ifndef __KEYEVENT_H__
#define __KEYEVENT_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PROGRAM_NAME "mci2dot_ev"

/****************************************************************************/


typedef struct keyevent
{
    char *data;
    struct keyevent* next;
} keyevent;

extern keyevent* keyevent_init(char*);
extern int keyevent_find (keyevent*, char*);
extern keyevent* keyevent_push (keyevent**, char*);
extern void keyevent_print (keyevent*);

#endif