/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#include <sys/queue.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fmr.h>
#include <biomdimacro.h>

// Test program to exercise some of the FMR library functions.

int main(int argc, char *argv[])
{
	char *usage = "usage: test_fmr <infile>";
	FILE *infp;
	struct finger_minutiae_record *fmr;
	struct finger_view_minutiae_record **fvmrs;
	int count = 0;
	int i;

	if (argc != 2) {
		printf("%s\n", usage);
		exit(EXIT_FAILURE);
	}

	infp = fopen(argv[1], "r");
	if (infp == NULL) {
		fprintf(stderr, "open of %s failed: %s\n",
			argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (new_fmr(FMR_STD_ANSI, &fmr) < 0) {
		fprintf(stderr, "could not allocate input FMR\n");
		exit(EXIT_FAILURE);
	}
	while (read_fmr(infp, fmr) == READ_OK) {
		count = get_fvmr_count(fmr);
		printf("FVMR count is %d\n", count);
		fvmrs = (struct finger_view_minutiae_record **) malloc(
			 count * sizeof(struct finger_view_minutiae_record **));
		if (fvmrs == NULL) {
			fprintf(stderr, "Memory allocation error.\n");
			exit (EXIT_FAILURE);
		}
		get_fvmrs(fmr, fvmrs);
		for (i = 0; i < count; i++) {
			printf("FVMR %d has %d minutiae.\n", 
				i, get_minutiae_count(fvmrs[i]));
			printf("FVMR %d has %d core records.\n", 
				i, get_core_record_count(fvmrs[i]));
			printf("FVMR %d has %d delta records.\n", 
				i, get_delta_record_count(fvmrs[i]));
			printf("FVMR %d has %d ridge data records.\n", 
				i, get_ridge_record_count(fvmrs[i]));
		}
	}

	exit (EXIT_SUCCESS);
}
