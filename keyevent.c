/****************************************************************************/
/* keyevent.c                                                                */
/*                                                                          */
/* Functions for initializing and modifying events		    */
/****************************************************************************/

#include "keyevent.h"

/*****************************************************************************/
/* create a keyevent list					     */

keyevent* keyevent_init(char* data){
    keyevent *newkey = calloc(sizeof(keyevent), 1);
    newkey->data = strdup(data);
    newkey->next = NULL;
    return newkey;
}

/*****************************************************************************/
/* find an keyevent; return 0 if keyevent is not found, 1 otherwise    */

int keyevent_find (keyevent *list, char* data)
{    
	while (list && strcmp(list->data, data)){
        list = list->next;
    }
	return (list && !strcmp(list->data, data)) ? 1 : 0;
}

/*****************************************************************************/
/* add a keyevent to the beginning of a list					     */

keyevent* keyevent_push (keyevent **list, char *data)
{
    keyevent *newkey = malloc(sizeof(keyevent));
    newkey->data = strdup(data);
    newkey->next = *list;
    return *list = newkey;
}

/*****************************************************************************/
/* Print keyevent list					     */

void keyevent_print(keyevent* list){

    while(list){
        printf("%s,", list->data);
        list = list->next;
    }
}