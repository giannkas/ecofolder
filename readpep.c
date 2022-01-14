/*****************************************************************************/
/* readPEP.c                                                                 */
/*****************************************************************************/
/*                                  Stefan Roemer,  06.02.1995 - 06.02.1995  */
/*                                  Stefan Schwoon, April 1997		     */
/*****************************************************************************/
/* The function read_pep_net at the end reads a PEP LL net into memory.	     */
/*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "readlib.h"
#include "netconv.h"
#include "common.h"

/*****************************************************************************/

enum { FT_STRING = 1, FT_COORDS = 2, FT_NUMBER = 3, FT_FLAG = 4 };
enum { TB_MANDATORY = 0, TB_OPTIONAL = 1 };
enum { TB_BLOCK = 0, TB_LINE = 1 };

#define MAX_LINE_SIZE 500
#define MAX_READ_PLACES 50

/* used for specifying where data fields of interest should be stored. */
typedef struct
{
	char c;		/* Identifying character. */
	void *ptr;	/* Memory location.	  */
} t_dest;

/* what to do with the data found in a block with matching name */
typedef struct
{
	char *name;		/* Name of the block.			     */
	int  (*hookfunc)();	/* called any time an entity has been read   */
	char **restptr;		/* Where to store additional data fields.    */
	t_dest *destarray;	/* Where to store which data field.	     */
} t_blockdest;

typedef struct { int x,y; } t_coords;	/* Simple struct for coordinates. */

/* File and block identifier. */
char *filetype;
char *blocktype;

typedef struct
{
	char c;
	int type;
} t_fieldinfo;

typedef struct
{
	char *name;
	int  optional;
	int  line;
	t_fieldinfo *field;
} t_blockinfo;

typedef struct
{
	int type;
	void *ptr;
} t_lookup;

/*****************************************************************************/
/* read_PEP_file							     */
/* This is a function for reading a file in PEP's general layout, i.e. it's  */
/* not restricted to nets (though in practice that's the only thing we'll    */
/* use it for). blocks is a data structure telling us how the layout of a    */
/* file should look like, and dest tells us what we should do with the data  */
/* we find (what and where to store it).				     */

void read_PEP_file(char *filename, char **types,
		   t_blockinfo *blocks, t_blockdest *dest)
{
	FILE *infile;
	t_lookup *tbl;
	t_fieldinfo *fld;
	t_dest *dst;
	t_blockdest *sdest = dest;
	int i, len = 0, ch, num, num2, ralloc;
	char *restptr, *rtmp;

	HLinput_file = filename;
	HLinput_line = 1;

	/* Open the file, read the header. */
	if (!(infile = fopen(filename, "r")))
		nc_error("could not open file for reading");

	ReadCmdToken(infile);
	if (strcmp(sbuf, "PEP")) nc_error("keyword `PEP' expected");

	/* Check if the file's type (second line of file) is one of those
	   that are allowed. */
	ReadNewline(infile);
	ReadCmdToken(infile);
	for (; *types && strcmp(sbuf,*types); types++);
	if (!*types) nc_error("unexpected format identifier '%s'",sbuf);
	filetype = MYstrdup(sbuf);

	ReadNewline(infile);
	ReadCmdToken(infile);
	if (strncmp(sbuf, "FORMAT_N", 8))
		nc_error("keyword 'FORMAT_N' or 'FORMAT_N2' expected");

	tbl = (t_lookup*) MYmalloc(sizeof(t_lookup)*(128-' ')) - ' ';
	ReadNewline(infile);

	while (!feof(infile))
	{
		/* Read next block id. */
		ReadCmdToken(infile);

		blocktype = MYstrdup(sbuf);

		/* Identify block. */
		for (; blocks->name && strcmp(blocks->name,sbuf); blocks++)
		    if (!blocks->optional)
			nc_error("keyword '%s' expected",blocks->name);

		if (!blocks->name) nc_error("unknown keyword '%s'",sbuf);

		for (dest = sdest; dest->name; dest++)
			if (!strcmp(blocks->name,dest->name)) break;

		/* Set up tbl. */
		for (i=' '; i<128; tbl[i++].type = 0);
		for (i=' '; i<128; tbl[i++].ptr = NULL);

		for (fld = blocks->field; fld->c; fld++)
			tbl[(int)fld->c].type = fld->type;

		if (!blocks->line) ReadNewline(infile);

		/* Read lines until next block begins. Every line yields one
		   new entity of the type determined by the current block. */
		for(;;)
		{
		    if (isupper(ch = ReadCharComment(infile)))
		    {
			char ch2 = getc(infile);
			ungetc(ch2,infile);
			/* We assume that uppercase letters at the start of
			   a line always indicates a new block. */
			if (isupper((int)ch2)) break;
		    }

		    if (feof(infile)) break;
		    if (ch == '\n') continue;

		    *(rtmp = restptr = MYmalloc(ralloc = 64)) = '\0';

		    /* If information about this block is wanted, take
		       the information where to store data from dest. */
		    if (dest->name)
			for (dst = dest->destarray; dst->c; dst++)
			{
			    t_lookup *l = tbl + dst->c;
			    l->ptr = dst->ptr;
			    switch(l->type)
			    {
				case FT_STRING:
				case FT_COORDS:
					*(char**)(l->ptr) = NULL;
					break;
				case FT_NUMBER:
				case FT_FLAG:
					*(int*)(l->ptr) = 0;
					break;
				default:
					nc_error("internal error: don't "
					"know the type of field '%c'",dst->c);
				break;
			    }
			}

		    /* Parse until end of line. We assume that all information
		       about an entity (place, transition etc.) is stored in
		       one single line. */
		    while (ch != '\n')
		    {
			if (isdigit(ch) || ch == '-')
			{
				/* Numbers are treated specially.
				   x@y gets stored in the '@' field, and
				   plain numbers are stored in the '0' field. */
				ungetc(ch,infile);
				ReadNumber(infile,&num);
				if ((ch=ReadWhiteSpace(infile)) == '@'
						  || ch == '<' || ch == '>')
				{
					ch = '@';
					ReadNumber(infile,&num2);
					len = 24;
				}
				else
				{
					ungetc(ch,infile);
					ch = '0';
					len = 12;
				}
			}
			else
			{
				if (ch == '\'' || ch == '"') ungetc(ch,infile);
				switch(tbl[ch].type)
				{
				    case FT_STRING:
					ReadEnclString(infile);
					len = 3+strlen(sbuf);
					break;
				    case FT_NUMBER:
					ReadNumber(infile,&num);
					len = 12;
					break;
				    case FT_COORDS:
					ReadCoordinates(infile, &num, &num2);
					len = 24;
					break;
				    case FT_FLAG:
					len = 1;
					break;
				    default:
					nc_error("unknown token '%c'",ch);
					break;
				}
			}

			/* Store data in the memory locations specified by
			   the dest array. If no location is given for a
			   particular field, its contents are written to
			   a 'rest' string. */
			while (!tbl[ch].ptr && rtmp-restptr+len >= ralloc)
			{
				restptr = MYrealloc(restptr, ralloc += 64);
				rtmp = restptr + strlen(restptr);
			}
			switch(tbl[ch].type)
			{
			    case FT_STRING:
				if (tbl[ch].ptr)
					*(char**)(tbl[ch].ptr) = MYstrdup(sbuf);
				else if (strchr("'\"",ch))
					sprintf(rtmp,"\"%s\"",sbuf);
				else
					sprintf(rtmp,"%c\"%s\"",ch,sbuf);
				break;
			    case FT_NUMBER:
				if (tbl[ch].ptr)
					*(int*)(tbl[ch].ptr) = num;
				else if (ch == '0')
					sprintf(rtmp,"%d ",num);
				else
					sprintf(rtmp,"%c%d",ch,num);
				break;
			    case FT_COORDS:
				if (tbl[ch].ptr)
				{
				    *((t_coords**)(tbl[ch].ptr)) =
						MYmalloc(sizeof(t_coords));
				    (*(t_coords**)(tbl[ch].ptr))->x = num;
				    (*(t_coords**)(tbl[ch].ptr))->y = num2;
				}
				else if (ch == '@')
					sprintf(rtmp,"%d@%d ",num,num2);
				else
					sprintf(rtmp,"%c%d@%d",ch,num,num2);
				break;
			    case FT_FLAG:
				if (tbl[ch].ptr)
					(*(int*)(tbl[ch].ptr))++;
				else
					sprintf(rtmp,"%c",ch);
				break;
			}
			if (!tbl[ch].ptr) rtmp += strlen(rtmp);
			ch = ReadCharComment(infile);
		    } /* end of while */

		    /* When the whole line has been read, see if information
		       about the current block is wanted - if so, call the
		       corresponding hook function. */
		    if (dest->name)
		    {
			if (dest->restptr) *(dest->restptr) = restptr;
			if (dest->hookfunc()) nc_error("read aborted");
		    }
		    else
			free(restptr);

		    HLinput_line++;
		}

		free(blocktype);
		if (!feof(infile)) ungetc(ch,infile);
		blocks++;
	}

	/* Check for mandatory blocks that didn't occur in the file. */
	for (; blocks->name; blocks++)
	    if (!blocks->optional)
		nc_error("section '%s' not found",blocks->name);

	free(tbl+' ');
	fclose(infile);
}

/*****************************************************************************/
/* Define the layout of a PEP net file.					     */

/* Allowable types of nets. */
char *type_llnet[] = { "PetriBox", "PTNet", NULL };

/* Defaults for blocks/places/transitions. */
t_fieldinfo nodedefs[] =
	{ { 'n', FT_COORDS },		/* rel pos of name	  */
	  { 'a', FT_COORDS },		/* rel pos of meaning	  */
	  { 's', FT_NUMBER },		/* object size		  */
	  { 't', FT_NUMBER },		/* line weight		  */
	  {  0 ,  0 } };

/* Defaults for arcs. */
t_fieldinfo arcdefs[] =
	{ { 'n', FT_COORDS },		/* rel pos of name	  */
	  { 't', FT_NUMBER },		/* line weight		  */
	  { 'w', FT_NUMBER },		/* default weight	  */
	  {  0 ,  0 } };

/* Data fields for blocks. */
t_fieldinfo blockfields[] =
	{ { '\'',FT_STRING },		/* identifier		  */
	  { '"', FT_STRING },		/*     "		  */
	  { '@', FT_COORDS },		/* coordinates		  */
	  { '0', FT_NUMBER },		/* numeric identifier	  */
	  { 'n', FT_COORDS },		/* rel pos of name	  */
	  { 'N', FT_COORDS },		/* abs. Pos of name	  */
	  { 'b', FT_STRING },		/* Bedeutung		  */
	  { 'a', FT_COORDS },		/* rel pos of meaning	  */
	  { 'A', FT_COORDS },		/* abs pos of meaning	  */
	  { 'R', FT_STRING },		/* reference		  */
	  { 'T', FT_STRING },		/* BPN->HL ref (obsolete) */
	  { 'u', FT_STRING },		/* block list		  */
	  {  0 ,  0 } };
	  
/* Data fields for places. */
t_fieldinfo placefields[] =
	{ { '\'',FT_STRING },		/* identifier		  */
	  { '"', FT_STRING },		/*     "		  */
	  { '@', FT_COORDS },		/* coordinates		  */
	  { '0', FT_NUMBER },		/* numeric identifier	  */
	  { 'n', FT_COORDS },		/* rel pos of name	  */
	  { 'N', FT_COORDS },		/* abs pos of name	  */
	  { 'b', FT_STRING },		/* meaning		  */
	  { 'a', FT_COORDS },		/* rel pos of meaning	  */
	  { 'A', FT_COORDS },		/* abs pos of meaning	  */
	  { 'M', FT_NUMBER },		/* initial marking (int)  */
	  { 'm', FT_NUMBER },		/* current marking (int) */
	  { 'u', FT_STRING },		/* block list		  */
	  { 'Z', FT_STRING },		/* type			  */
	  { 'B', FT_COORDS },		/* rel pos of type	  */
	  { 'z', FT_STRING },		/* initial marking (str)  */
	  { 'y', FT_STRING },		/* current marking (str)  */
	  { 'R', FT_STRING },		/* reference		  */
	  { 'k', FT_NUMBER },		/* capacity		  */
	  { 'e', FT_FLAG   },		/* entry place		  */
	  { 'x', FT_FLAG   },		/* exit place		  */
	  { 'v', FT_NUMBER },		/* flags		  */
	  { 's', FT_NUMBER },		/* object size		  */
	  { 't', FT_NUMBER },		/* line weight		  */
	  { 'c', FT_NUMBER },		/* colour		  */
	  { 'T', FT_STRING },		/* BPN->HL ref (obsolete) */
	  {  0 ,  0 } };
  
/* Data fields for transitions. */
t_fieldinfo transfields[] =
	{ { 'S', FT_FLAG   },		/* synch transition	  */
	  { '\'',FT_STRING },		/* identifier		  */
	  { '"', FT_STRING },		/*     "		  */
	  { '@', FT_COORDS },		/* coordinates		  */
	  { '0', FT_NUMBER },		/* numeric identifier	  */
	  { 'n', FT_COORDS },		/* rel pos of name	  */
	  { 'N', FT_COORDS },		/* abs pos of name	  */
	  { 'b', FT_STRING },		/* meaning		  */
	  { 'a', FT_COORDS },		/* rel pos of meaning	  */
	  { 'A', FT_COORDS },		/* abs pos of meaning	  */
	  { 'v', FT_NUMBER },		/* flags		  */
	  { 'u', FT_STRING },		/* block list		  */
	  { 'P', FT_STRING },		/* list of phantom trs	  */
	  { 'g', FT_STRING },		/* value term		  */
	  { 'h', FT_COORDS },		/* rel pos of value term  */
	  { 'H', FT_COORDS },		/* rel pos of value term  */
	  { 'R', FT_STRING },		/* reference		  */
	  { 'i', FT_STRING },		/* action terms		  */
	  { 'j', FT_COORDS },		/* rel pos action terms   */
	  { 's', FT_NUMBER },		/* object size		  */
	  { 't', FT_NUMBER },		/* line weight		  */
	  { 'c', FT_NUMBER },		/* colour		  */
	  { 'T', FT_STRING },		/* BPN->HL ref (obsolete) */
	  { 'r', FT_FLAG   },		/* refined flag		  */
	  {  0 ,  0 } };
  
/* Data fields for arcs. */
t_fieldinfo arcfields[] =
	{ { '@', FT_COORDS },		/* source/dest		  */
	  { 'J', FT_COORDS },		/* coordinates		  */
	  { ',', FT_COORDS },		/* more coordinates	  */
	  { 'w', FT_NUMBER },		/* weight		  */
	  { 'n', FT_COORDS },		/* rel pos of weight	  */
	  { 'N', FT_COORDS },		/* abs pos of weight	  */
	  { 'p', FT_STRING },		/* inscription		  */
	  { 'q', FT_COORDS },		/* rel pos of inscription */
	  { 'Q', FT_COORDS },		/* abs pos of inscription */
	  { 'v', FT_NUMBER },		/* visibility		  */
	  { 't', FT_NUMBER },		/* line weight		  */
	  { 'c', FT_NUMBER },		/* colour		  */
	  {  0 ,  0 } };

/* Data fields for texts. */
t_fieldinfo textfields[] =
	{ { '\'',FT_STRING },		/* text			  */
	  { '"', FT_STRING },		/*   "			  */
	  { 'N', FT_COORDS },		/* absolute position	  */
	  {  0 ,  0 } };

/* All the data blocks that may occur in the file. */

t_blockinfo netblocks[] =
	{ { "DBL", TB_OPTIONAL,  TB_LINE,  nodedefs },
	  { "DPL", TB_OPTIONAL,  TB_LINE,  nodedefs },
	  { "DTR", TB_OPTIONAL,  TB_LINE,  nodedefs },
	  { "DPT", TB_OPTIONAL,  TB_LINE,  arcdefs },
	  { "BL",  TB_OPTIONAL,  TB_BLOCK, blockfields },
	  { "PL",  TB_MANDATORY, TB_BLOCK, placefields },
	  { "TR",  TB_MANDATORY, TB_BLOCK, transfields },
	  { "PTR", TB_OPTIONAL,  TB_BLOCK, transfields+1 },
	  { "RD",  TB_OPTIONAL, TB_BLOCK, arcfields },		//*** NEW  ***//
	  { "TP",  TB_MANDATORY, TB_BLOCK, arcfields },
	  { "PT",  TB_MANDATORY, TB_BLOCK, arcfields },
	  { "RS",  TB_OPTIONAL, TB_BLOCK, arcfields },		//*** NEW  ***//
	  { "PTP", TB_OPTIONAL,  TB_BLOCK, arcfields },
	  { "PPT", TB_OPTIONAL,  TB_BLOCK, arcfields },
	  { "TX",  TB_OPTIONAL,  TB_BLOCK, textfields },
	  { NULL, 0, 0, NULL } };

/*****************************************************************************/

#define MBSIZE 4096
#define NAMES_START 2000
#define NAMES_OFFSET 1000

net_t	 *rd_net;
t_coords *rd_co;
place_t	**PlArray;
trans_t **TrArray;
int  AnzPlNamen, MaxPlNamen, AnzTrNamen, MaxTrNamen;
int  placecount, transcount, rd_ident, rd_marked;
char autonumbering, *rd_name;

/*****************************************************************************/
/* insert_{place,trans,arc}						     */
/* These functions are called from read_PEP_net whenever a place, transition */
/* or an arc has been parsed in the net. The tables place_dest, trans_dest   */
/* and arc_dest in read_HLnet determine where read_PEP_net should store	     */
/* contents of certain fields prior to calling these functions.		     */

int insert_place()
{
	placecount++;
	if (rd_ident && rd_ident != placecount) autonumbering = 0;
	if (!rd_ident && autonumbering) rd_ident = placecount;
	if (!rd_ident) nc_error("missing place identifier");
		
	if (rd_ident > AnzPlNamen)
		AnzPlNamen = rd_ident;
	else if (PlArray[rd_ident])
		nc_error("place identifier %d used twice", rd_ident);

	while (AnzPlNamen >= MaxPlNamen)
	{
		int count;
		PlArray = MYrealloc((char*) PlArray,
			(MaxPlNamen += NAMES_OFFSET) * sizeof(place_t*));
		for (count = MaxPlNamen - NAMES_OFFSET; count < MaxPlNamen;)
			PlArray[count++] = NULL;
	}

	PlArray[rd_ident] = nc_create_place(rd_net);
	PlArray[rd_ident]->name = rd_name? MYstrdup(rd_name) : NULL;
	if (rd_marked > 1) nc_error("place %s has more than one token",rd_name);
	PlArray[rd_ident]->marked = !!rd_marked;
	return 0;
}

int insert_trans()
{
	if (!transcount++) autonumbering = 1;
	if (rd_ident && rd_ident != transcount) autonumbering = 0;
	if (!rd_ident && autonumbering) rd_ident = transcount;
	if (!rd_ident) nc_error("missing transition identifier");

	if (rd_ident > AnzTrNamen)
		AnzTrNamen = rd_ident;
	else if (TrArray[rd_ident])
		nc_error("transition identifier %d used twice",rd_ident);

	while (AnzTrNamen >= MaxTrNamen)
	{
		int count;
		TrArray = MYrealloc((char*) TrArray,
			  (MaxTrNamen += NAMES_OFFSET) * sizeof(trans_t*));
		for (count = MaxTrNamen - NAMES_OFFSET; count < MaxTrNamen;)
			TrArray[count++] = NULL;
	}

	TrArray[rd_ident] = nc_create_transition(rd_net);
	TrArray[rd_ident]->name = rd_name? MYstrdup(rd_name) : NULL;
	return 0;
}

int insert_arc()
{
	static int tp, rs = 0, rd = 0;  /* tp = 1 means Place->Trans, 0 is Trans->Place; 
							rs = 1 means reset arc Place->Trans 
							rd = 1 means read arc Place->Trans */
	int pl, tr;

	if (*blocktype)
	{
		rs = strcmp(blocktype,"RS");
		if (rs == 0)
			tp = -1;
		else{
			rd = strcmp(blocktype, "RD");
			tp = rd == 0 ? -2 : strcmp(blocktype,"PT");
		}
		*blocktype = '\0';
	}

	if (tp == -1)
		tp = rs;
	else if(tp == -2)
		tp = rd;
	pl = tp? rd_co->y : rd_co->x;
	tr = tp? rd_co->x : rd_co->y;
	
	//printf("pl %d and tr %d\n", pl, tr);

	if (!tr || (tr > AnzTrNamen) || !TrArray[tr])
		nc_error("arc: incorrect transition identifier");
	if (!pl || (pl > AnzPlNamen) || !PlArray[pl] )
		nc_error("arc: incorrect place identifier");

	if (rs == 0)
		nc_create_arc(&(TrArray[tr]->reset),&(PlArray[pl]->reset),
			  TrArray[tr],PlArray[pl]);
	else if (rd == 0){
		nc_create_arc(&(TrArray[tr]->ctxset),&(PlArray[pl]->ctxset),
			  TrArray[tr],PlArray[pl]);
	}
	else if (tp != 0)
		nc_create_arc(&(TrArray[tr]->postset),&(PlArray[pl]->preset),
			  TrArray[tr],PlArray[pl]);
	else{
		nc_create_arc(&(PlArray[pl]->postset),&(TrArray[tr]->preset),
			  PlArray[pl],TrArray[tr]);
	}
	return 0;
}

/*****************************************************************************/
/* The main function of this file: read a PEP file into a net_t structure.   */

net_t* read_pep_net(char *PEPfilename)
{
	/* These tables instruct read_PEP_net where contents 
	   of certain fields should be stored.		    */
	
	t_dest place_dest[] =
		{ { '\'',&rd_name },	/* identifier		*/
		  { '"', &rd_name },	/*     "		*/
		  { '0', &rd_ident },	/* numeric name		*/
		  { 'M', &rd_marked },	/* initial marking	*/
		  {  0 ,  0 } };
	  
	t_dest trans_dest[] =
		{ { '\'',&rd_name },	/* identifier		*/
		  { '"', &rd_name },	/*     "		*/
		  { '0', &rd_ident },	/* numeric name		*/
		  {  0 ,  0 } };
	  
	t_dest arc_dest[] =
		{ { '@', &rd_co },	/* source/destination	*/
		  {  0 ,  0 } };

	/* This table is passed to read_PEP_net and instructs   */
	/* it which "hook" functions (see below) should be	*/
	/* called whenever a place, transition etc. is found.	*/

	t_blockdest netdest[] =
		{ { "PL",  insert_place, NULL, place_dest },
		  { "TR",  insert_trans, NULL, trans_dest },
		  { "RD",  insert_arc,   NULL, arc_dest},			//*** NEW  ***//
		  { "TP",  insert_arc,   NULL, arc_dest },
		  { "PT",  insert_arc,   NULL, arc_dest },
		  { "RS",  insert_arc,   NULL, arc_dest},			//*** NEW  ***//
		  { NULL, NULL, NULL, NULL } };

	int count;

	/* Set up tables. */
	PlArray = MYmalloc((count = MaxPlNamen = MaxTrNamen = NAMES_START)
				* sizeof(place_t*));
	TrArray = MYmalloc(count * sizeof(trans_t*));
	AnzPlNamen = AnzTrNamen = 0;
	while (--count)
		PlArray[count] = NULL, TrArray[count] = NULL;

	/* Initialize net */
	rd_net = nc_create_net();

	placecount = transcount = 0;
	autonumbering = 1;

	/* Read the net */
	read_PEP_file(PEPfilename, type_llnet, netblocks, netdest);

	free(PlArray);
	free(TrArray);

	nc_compute_sizes(rd_net);

	return rd_net;
}

net_t* reset_complement(net_t *net){

	place_t *pl, *pl2;
	nodelist_t *list;
	nodelist_t *rsttr = NULL;	
	
	for (pl = net->places; pl; pl=pl->next)
	{
		rsttr = pl->reset;
		if (rsttr){			
			pl2 = nc_create_place(net);
			pl2->name = strdup(pl->name);
			strcat(pl2->name, "_rs");
			pl2->marked = pl->marked ? 0 : 1;

			for (list = pl->preset; list; list = list->next)
				if (!nodelist_find(pl->postset, list->node))
					nc_create_arc(&(pl2->postset),&(((trans_t*)(list->node))->preset),
						pl2,((trans_t*)(list->node)));
			for (list = pl->postset; list; list = list->next)
				if (!nodelist_find(pl->preset, list->node))
					nc_create_arc(&(((trans_t*)(list->node))->postset), &(pl2->preset),
						((trans_t*)(list->node)),pl2);
			for (list = pl->reset; list; list = list->next){
				nc_create_arc(&(((trans_t*)(list->node))->reset), &(pl2->reset),
					((trans_t*)(list->node)),pl2);
				nc_create_arc(&(((trans_t*)(list->node))->postset), &(pl2->preset),
					((trans_t*)(list->node)),pl2);
			}
		}
	}

	return net;
}

/* net_t* pr_encoding(net_t *net){

	place_t *pl, *pl2;
	nodelist_t *list;
	nodelist_t *ctxtr = NULL;
	int n_ctxtr = 0;
	char buffer[50];

	for (pl = net->places; pl; pl=pl->next)
	{
		for (ctxtr = pl->ctxset; ctxtr; ctxtr = ctxtr->next){
			n_ctxtr++;
			if(n_ctxtr > 1){
				pl2 = nc_create_place(net);
				sprintf(buffer, "%s_%d", pl->name, n_ctxtr);
				pl2->name = MYstrdup(buffer);
				pl2->marked = pl->marked ? 1 : 0;
				for (list = pl->preset; list; list = list->next)
					nc_create_arc(&(pl2->preset),&(((trans_t*)(list->node))->postset),
			  			pl2,((trans_t*)(list->node)));
				for (list = pl->postset; list; list = list->next)
					nc_create_arc(&(((trans_t*)(list->node))->preset), &(pl2->postset),
						((trans_t*)(list->node)),pl2);
				for (list = pl->reset; list; list = list->next)
					nc_create_arc(&(((trans_t*)(list->node))->reset), &(pl2->reset),
						((trans_t*)(list->node)),pl2);				
				nc_create_arc(&(((trans_t*)(ctxtr->node))->postset),&(pl2->preset),
			  		((trans_t*)(ctxtr->node)),pl2);
				nc_create_arc(&(pl2->postset),&(((trans_t*)(ctxtr->node))->preset),
					pl2,((trans_t*)(ctxtr->node)));
			}
		}
		if(pl->ctxset){
			pl->name = n_ctxtr > 1 ? strcat(pl->name, "_1") : pl->name;
			nc_create_arc(&(((trans_t*)(pl->ctxset->node))->postset),&(pl->preset),
			  ((trans_t*)(pl->ctxset->node)),pl);
			nc_create_arc(&(pl->postset),&(((trans_t*)(pl->ctxset->node))->preset),
				pl,((trans_t*)(pl->ctxset->node)));
		}

		n_ctxtr = 0;
	}

	nc_compute_sizes(net);
	
	return net;
} */

char* pr_encoding(char* in_file){
    // Declare the file pointer
    FILE *r_pointer ;
    FILE *w_pointer = NULL;
     
    // Variables that come along
    char d_read[50];
    char *token, *tmp, *tmp1, *token2, *out_file = NULL;    
    int header = 0, places = 0, trans = 0, new_places = 0, 
        counter_pl = 0, num_tmp, rd_arcs = 0;

    // Open the existing file using fopen()
    // in read mode using "r" attribute
    r_pointer = fopen(in_file, "r") ;
     
    // Check if this r_pointer is null
    // which maybe if the file does not exist
    if ( r_pointer == NULL )
    {
        printf( "%s file failed to open.", in_file ) ;
    }
    else
    {        
        /* Read up to RD label and get number of transitions and places*/
        while(fgets(d_read, 50, r_pointer) != NULL && d_read[0] != 'R' && d_read[1] != 'D'){
            if (d_read[0] == 'P' && d_read[1] == 'L');
            else if (places == header) header++;
            if (d_read[0] == 'T' && d_read[1] == 'R');
            else if (trans == places) places++;
            trans++;
        }
        /* Compute header lines, number of places and transitions  */
        header++;
        places = places - header;
        header++;
        trans = trans - places - header;
        
        if(d_read[0] == 'R' && d_read[1] == 'D'){
			out_file = ftokstr(in_file, 0, '.');
			strcat(out_file, "_pr.ll_net");
    		w_pointer = fopen(out_file, "w"); // if we have reset arcs then create a new file.
            int n_ctxtr = 1;
            char buf_arcs[MAX_READ_PLACES];
            int read_places[MAX_READ_PLACES] = {0};
            char buffer_pl[MAX_LINE_SIZE] = {0};
            char buffer_rd[MAX_LINE_SIZE] = {0};            

            /* Check for those places that have more than one read arc and make its respective
             replication in buffer_pl which will be pasted into the output file */
            while(fgets(d_read, 50, r_pointer) != NULL && isdigit(d_read[0])){
                strcat(buffer_rd, d_read);
                token = strtok(d_read, ">");
                if(n_ctxtr > 1 && !strcmp(token, tmp)){
                    sprintf(buf_arcs, "\"%s_%d\",\n", token, n_ctxtr);
                    strcat(buffer_pl, buf_arcs);
                    new_places++;
                    read_places[strtoint(token)] = n_ctxtr;
                }
                else{
                    tmp = strdup(token);
                    n_ctxtr = 1;
                }
                n_ctxtr++;
            }            

            /* Print into the output file the places that come from the input file plus
            those that were replicated (print the content from buffer_pl) */
            fseek( r_pointer, 0, SEEK_SET );
            while(fgets(d_read, 50, r_pointer) != NULL && d_read[0] != 'T' && d_read[1] != 'R'){
                token = ftokstr(d_read, 1, '\"');
                if(token){                    
                    num_tmp = strtoint(token);
                    if(read_places[num_tmp] > 1){
                        sprintf(buf_arcs, "\"%s_%d\"\n", token, 1);
                        fprintf(w_pointer, "%s", buf_arcs);
                    }
                    else
                        fprintf(w_pointer, "%s", d_read);
                }
                else
                    fprintf(w_pointer, "%s", d_read);
            }
            token = strtok(buffer_pl, ",");
            while (token != NULL){
                fprintf(w_pointer, "%s", token);
                token = strtok(NULL, ",");
            }

            /* TP SECTION */
            /* Any transition t producing p in N produces p_i in N', i.e., p_i in postset(t)*/
            new_places = places;
            if(d_read[0] == 'T' && d_read[1] == 'R'){
                fprintf(w_pointer, "%s", d_read);
                while(fgets(d_read, 50, r_pointer) != NULL && d_read[0] != 'P' && d_read[1] != 'T'){
                    if( strlen(d_read) > 2 && d_read[1] == '<' ){
                        tmp1 = ltokstr(d_read, '<');
						printf("d_read is: %s", d_read);
						printf("tmp1 is: %s\n", tmp1);
                        num_tmp = strtoint(tmp1);
                        counter_pl = read_places[num_tmp]-1;
                        for(int i = 1; i <= counter_pl && counter_pl > 0; i++){
                            num_tmp = new_places + 1;
                            sprintf(buf_arcs, "%s<%d", ftokstr(d_read, 0, '<'), num_tmp);
                            fprintf(w_pointer, "%s\n", buf_arcs);
                            new_places = new_places + i;
                        }            
                    }
                    fprintf(w_pointer, "%s", d_read);
                }
            }
            /* Transition t_i produces place p_i, i.e., p_i in postset(t)*/
            new_places = places;
            if(d_read[0] == 'P' && d_read[1] == 'T'){
                rd_arcs = 0;
                token = ftokstr(buffer_rd, rd_arcs, '\n'); tmp = "";
                while (strlen(token) != 0){
                    token2 = strdup(token);
                    tmp1 = ftokstr(token, 0, '>');
                    if(strcmp(tmp1, tmp) != 0){
                        revstr(token2);
                        flip(token2, '>', '<');
                        fprintf(w_pointer, "%s\n", token2);
                        counter_pl = 0;
                    }else{
                        counter_pl++;
                        num_tmp = new_places + 1;
                        tmp1 = ltokstr(token, '>');
                        sprintf(buf_arcs, "%s<%d", tmp1, num_tmp);
                        fprintf(w_pointer, "%s\n", buf_arcs);
                        new_places = new_places + counter_pl;
                    }
                    tmp = ftokstr(token, 0, '>');
                    rd_arcs++;
                    token = ftokstr(buffer_rd, rd_arcs,'\n');
                }
            }

            /* PT SECTION */
            /* Any transition t consuming p in N consumes p_i in N', i.e., p_i in preset(t)*/
            new_places = places;
            if(d_read[0] == 'P' && d_read[1] == 'T'){
                fprintf(w_pointer, "%s", d_read);
                while(fgets(d_read, 50, r_pointer) != NULL){
                    if( strlen(d_read) > 2 && d_read[1] == '>' ){
                        tmp1 = ftokstr(d_read, 0, '>');
                        num_tmp = strtoint(tmp1);
                        counter_pl = read_places[num_tmp]-1;
                        for(int i = 1; i <= counter_pl && counter_pl > 0; i++){
                            num_tmp = new_places + 1;
                            sprintf(buf_arcs, "%d>%s", num_tmp, ltokstr(d_read, '>'));
                            fprintf(w_pointer, "%s\n", buf_arcs);
                            new_places = new_places + i;
                        }        
                    }
                    fprintf(w_pointer, "%s", d_read);
                }
            }
            /* Transition t_i consumes place p_i, i.e., p_i in preset(t)*/
            new_places = places;
            if(fgets(d_read, 50, r_pointer) == NULL){
                fprintf(w_pointer, "\n");
                rd_arcs = 0;
                token = ftokstr(buffer_rd, rd_arcs, '\n'); tmp = "";
                while (strlen(token) != 0){
                    tmp1 = ftokstr(token, 0, '>');
                    if (strcmp(tmp1, tmp) != 0)
                        fprintf(w_pointer, "%s\n", token);
                    else{
                        num_tmp = strtoint(tmp1);
                        counter_pl = read_places[num_tmp]-1;
                        if (counter_pl > 0){
                            num_tmp = new_places + 1;
                            sprintf(buf_arcs, "%d>%s", num_tmp, ltokstr(token, '>'));
                            fprintf(w_pointer, "%s\n", buf_arcs);
                            new_places++;
                        }
                    }
                    tmp = ftokstr(token, 0, '>');
                    rd_arcs++;
                    token = ftokstr(buffer_rd, rd_arcs,'\n');
                }
            }
        }        

        fclose(r_pointer) ;
        fclose(w_pointer) ;
    }
    return out_file? out_file : in_file;
}