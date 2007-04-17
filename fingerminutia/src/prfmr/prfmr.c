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
/* This program uses the ANSI/INCITS Finger Minutiae Record library to print  */
/* the contents of a file containing minutiae records.                        */
/*                                                                            */
/*                                                                            */
/*                                                                            */
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
#include <biomdimacro.h>

static int std_types[4] = {FMR_STD_ANSI, FMR_STD_ISO, FMR_STD_ISO_NORMAL_CARD,
    FMR_STD_ISO_COMPACT_CARD};
static char *std_names[4] = {"ANSI", "ISO", "ISO Normal Card", "ISO Compact Card"};
#define NUM_STD_TYPES	4
int main(int argc, char *argv[])
{
	char *usage = "usage: prfmr [-v] <datafile>\n"
			"\t -v Validate the record";
	FILE *fp;
	struct stat sb;
	struct finger_minutiae_record *fmr;
	int vflag = 0;
	int ch;
	int ret;
	int std;
	unsigned int total_length;

	if ((argc < 2) || (argc > 3)) {
		printf("%s\n", usage);
		exit (EXIT_FAILURE);
	}

	while ((ch = getopt(argc, argv, "v")) != -1) {
		switch (ch) {
			case 'v' :
				vflag = 1;
				break;
			default :
				printf("%s\n", usage);
				exit (EXIT_FAILURE);
				break;
		}
	}
				
	if (argv[optind] == NULL) {
		printf("%s\n", usage);
		exit (EXIT_FAILURE);
	}

	fp = fopen(argv[optind], "rb");
	if (fp == NULL) {
		fprintf(stderr, "open of %s failed: %s\n",
			argv[optind], strerror(errno));
		exit (EXIT_FAILURE);
	}


	if (fstat(fileno(fp), &sb) < 0) {
		fprintf(stdout, "Could not get stats on input file.\n");
		exit (EXIT_FAILURE);
	}

	total_length = 0;
	std = 0;
	while (total_length < sb.st_size) {
		if (new_fmr(std_types[std], &fmr) < 0) {
			fprintf(stderr, "could not allocate FMR\n");
			exit (EXIT_FAILURE);
		}
		printf("================================================\n");
		printf("Attempting read conforming to %s:\n", std_names[std]);
		ret = read_fmr(fp, fmr);
		/* Try other standard formats */
		if (ret != READ_OK) {
			std++;
			if (std < NUM_STD_TYPES) {
				free_fmr(fmr);
				rewind(fp);
				continue;
			} else {
				break;
			}
		}
		total_length += fmr->record_length;

		// Validate the FMR
		if (vflag) {
			if (validate_fmr(fmr) != VALIDATE_OK) {
				fprintf(stdout, 
				    "Finger Minutiae Record is invalid.\n");
				exit (EXIT_FAILURE);
			} else {
				fprintf(stdout, 
				    "Finger Minutiae Record is valid.\n");
			}
		}
		print_fmr(stdout, fmr);
		free_fmr(fmr);
	}
	if (ret != READ_OK) {
		fprintf(stderr, "Could not read entire record; Contents:\n");
		print_fmr(stderr, fmr);
		free_fmr(fmr);
		exit (EXIT_FAILURE);
	}

	if (vflag) {
		// Check the record length info against file reality
		if (sb.st_size != total_length) {
			fprintf(stdout, "WARNING: "
			    "File size does not match FMR record length(s).\n");
		} 
	}

	exit (EXIT_SUCCESS);
}
