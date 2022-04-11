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
    //keyevent_print(list);
    //"list->data: %s\n", list->data);
    //printf("data: %s\n", data);
	while (list && strcmp(list->data, data)){
        list = list->next;
    }
	
	return (list && !strcmp(list->data, data)) ? 1 : 0;
}

/*****************************************************************************/
/* add a keyevent to the end of a list					     */

/* keyevent* keyevent_push (keyevent *list, char *data)
{
    keyevent *newkey = calloc(sizeof(keyevent), 1);
    newkey->data = data;
    newkey->next = NULL;

	keyevent *head = calloc(sizeof(keyevent), 1);
	head->data = list->data;
    head->next = list->next;

    while(list)
        list = list->next;

    list->next = newkey;
	
	return head ? head : NULL;
} */

keyevent* keyevent_push (keyevent **list, char *data)
{
    keyevent *newkey = malloc(sizeof(keyevent));
    newkey->data = strdup(data);
    newkey->next = *list;
    return *list = newkey;
    /* printf("list->data: %s\n", list->data);
    printf("newkey->data: %s\n", newkey->data);
	keyevent *head = calloc(sizeof(keyevent), 1);
	head->data = list->data;
    keyevent *tail = head;
    list = list->next;
    while(list && list->next){
        tail->next = malloc(sizeof(keyevent));
        tail = tail->next;
        tail->data = list->data;
        list = list->next;
    }

    tail->next = newkey;
	
	return head ? head : NULL; */
}

/*****************************************************************************/
/* Print keyevent list					     */

void keyevent_print(keyevent* list){

    while(list){
        printf("%s,", list->data);
        list = list->next;
    }
}