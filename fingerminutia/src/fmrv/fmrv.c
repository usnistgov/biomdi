/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
/******************************************************************************/
/* This program uses the ANSI/INCITS Finger Minutiae Record library to        */
/* validate the contents of a file containing minutiae records.               */
/*                                                                            */
/* Return values:                                                             */
/*    0 - File contents are valid                                             */
/*    1 - File contents are invalid                                           */
/*   -1 - Other error occurred                                                */
/******************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/queue.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fmr.h>
#include <m1io.h>

int main(int argc, char *argv[])
{
	char *usage = "usage: fmrv <datafile>\n";
	FILE *fp;
	struct stat sb;
	struct finger_minutiae_record *fmr;
	int ch;
	int ret;
	unsigned int total_length;

	if (argc != 2) {
		printf("%s", usage);
		exit (EXIT_FAILURE);
	}

	if (argv[1] == NULL) {
		printf("%s", usage);
		exit (EXIT_FAILURE);
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL)
		OPEN_ERR_EXIT(argv[optind]);

	if (fstat(fileno(fp), &sb) < 0) {
		fprintf(stderr, "Could not get stats on input file.\n");
		exit (EXIT_FAILURE);
	}

	if (new_fmr(&fmr) < 0)
		ALLOC_ERR_EXIT("Could not allocate FMR\n");

	total_length = 0;
	while (total_length < sb.st_size) {
		ret = read_fmr(fp, fmr);
		if (ret != READ_OK)
			break;
		total_length += fmr->record_length;

		// Validate the FMR
		if (validate_fmr(fmr) != VALIDATE_OK)
			exit (EXIT_FAILURE);

		free_fmr(fmr);

		if (new_fmr(&fmr) < 0)
			ALLOC_ERR_EXIT("Could not allocate FMR\n");
	}
	if (ret != READ_OK)
		exit (EXIT_FAILURE);

	exit (EXIT_SUCCESS);
}
