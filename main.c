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
		"%s -- low level net unfolder\n\n"
		"Usage: %s [options] <LLnetfile> [FileOptions]\n\n"

	"     Options:\n"
	"      -T <name>      stop when transition <name> is inserted\n"
	"      -d <depth>     unfold up to given depth\n"
	"      -i             interactive mode\n\n"

	"     FileOptions:\n"
	"      -m <filename>  file to store the unfolding in\n\n"

	"Unless specified otherwise, all filenames will default to\n"
	"the basename of <LLnetfile> plus appropriate extensions.\n\n"

	"Version 1.1.0 (28.04.2014)\n", myname, myname);

	exit(1);
}

/*****************************************************************************/

int main (int argc, char **argv)
{
	int	 i;
	char    *llnet = NULL, *mcifile;
	char    **dptr = &llnet;
	char	*tmpname, *idx;
	char    *stoptr_name = NULL;

	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i],"-m"))
			dptr = &mcifile;
		else if (!strcmp(argv[i],"-T"))
			stoptr_name = argv[++i];
		else if (!strcmp(argv[i],"-d"))
			unfold_depth = atoi(argv[++i]);
		else if (!strcmp(argv[i],"-i"))
			interactive = 1;
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
				strcpy(idx,".mci");
				mcifile = MYstrdup(tmpname);
			}

			dptr = NULL;
		}

	if (!llnet) usage(argv[0]);

	net = read_pep_net(llnet);
	nc_static_checks(net,stoptr_name);
	unfold();
	write_mci_file(mcifile);

	return exitcode;
}
