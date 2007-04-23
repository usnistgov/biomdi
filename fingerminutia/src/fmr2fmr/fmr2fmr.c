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
/* This program will transform certain values from an INCITS M1 record to     */
/* ISO compact form, then back again. The purpose of this transformation is   */
/* to simulate a fingerprint minutiae record that is converted from M1        */
/* format to ISO compact, as when an FMR is placed on a smart card, then      */
/* converted back for matching, etc.                                          */
/******************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fmr.h>
#include <biomdimacro.h>
#include "ansi2iso.h"

/* Keep track of the entire FMR length in a global. */
static unsigned int fmr_length;

/******************************************************************************/
/* Print a how-to-use the program message.                                    */
/******************************************************************************/
static void
usage()
{
	fprintf(stderr, 
	    "usage:\n"
	    "\tfmr2fmr -i <m1file> -o <outfile>\n"
	    "\twhere:\n"
	    "\t   -i:  Specifies the input FMR file\n"
	    "\t   -o:  Specifies the output FMR file\n");
}

/* Global file pointers */
static FILE *in_fp;	// the FMR input file
static FILE *out_fp;	// for the output file

/******************************************************************************/
/* Close all open files.                                                      */
/******************************************************************************/
static void
close_files()
{
	if (in_fp != NULL)
		(void)fclose(in_fp);
	in_fp = NULL;
	if (out_fp != NULL)
		(void)fclose(out_fp);
	out_fp = NULL;
}

/******************************************************************************/
/* Process the command line options, and set the global option indicators     */
/* based on those options.  This function will force an exit of the program   */
/* on error.                                                                  */
/******************************************************************************/
static void
get_options(int argc, char *argv[])
{
	int ch, i_opt, o_opt;
	char *out_file;
	struct stat sb;

	i_opt = o_opt = 0;
	while ((ch = getopt(argc, argv, "i:o:")) != -1) {
		switch (ch) {
		    case 'i':
			if ((in_fp = fopen(optarg, "rb")) == NULL)
				OPEN_ERR_EXIT(optarg);
			i_opt++;
			break;

		    case 'o':
			if (o_opt == 1)
				goto err_usage_out;
			if (stat(optarg, &sb) == 0) {
		    	    ERR_OUT(
				"File '%s' exists, remove it first.", optarg);
			}
			if ((out_fp = fopen(optarg, "wb")) == NULL)
				OPEN_ERR_EXIT(optarg);
			out_file = optarg;
			o_opt++;
			break;
				
		    default:
			goto err_usage_out;
		}
	}

	if ((i_opt != 1) || (o_opt != 1))
		goto err_usage_out;

	return;

err_usage_out:
	usage();
err_out:
	close_files();

	/* If we created the output file, remove it. */
	if (o_opt == 1)
		(void)unlink(out_file);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	FMR *ifmr, *ofmr = NULL;
	FVMR **ifvmrs = NULL;
	FVMR *ofvmr;
	int r, rcount, fvmrl;

	get_options(argc, argv);

	// Allocate the input/output FMR records in memory
	if (new_fmr(FMR_STD_ANSI, &ifmr) < 0)
		ALLOC_ERR_OUT("Input FMR");
	if (new_fmr(FMR_STD_ISO, &ofmr) < 0)
		ALLOC_ERR_OUT("Output FMR");

	// Read the input FMR
	if (read_fmr(in_fp, ifmr) != READ_OK) {
		fprintf(stderr, "Could not read FMR from file.\n");
		goto err_out;
	}
	COPY_FMR(ifmr, ofmr);
	fmr_length = FMR_ISO_HEADER_LENGTH;

	// Get all of the finger view records
	rcount = get_fvmr_count(ifmr);
	if (rcount > 0) {
		ifvmrs = (FVMR **) malloc(rcount * sizeof(FVMR *));
		if (ifvmrs == NULL)
			ALLOC_ERR_OUT("FVMR Array");
		if (get_fvmrs(ifmr, ifvmrs) != rcount)
			ERR_OUT("getting FVMRs from FMR");

		for (r = 0; r < rcount; r++) {
			if (new_fvmr(FMR_STD_ISO, &ofvmr) < 0)
	                        ALLOC_ERR_RETURN("Output FVMR");
			COPY_FVMR(ifvmrs[r], ofvmr);
			if (ansi2iso_fvmr_theta(ifvmrs[r], ofvmr, &fvmrl) < 0)
				ERR_OUT("Modifying FVMR");
			fmr_length += fvmrl;
			ofvmr->extended = NULL;
			add_fvmr_to_fmr(ofvmr, ofmr);
			// XXX Copy the FEDB to the output fmr
			fmr_length += FEDB_HEADER_LENGTH;
		}

	} else {
		if (rcount == 0)
			ERR_OUT("there are no FVMRs in the input FMR");
		else
			ERR_OUT("retrieving FVMRs from input FMR");
	}

	ofmr->record_length = fmr_length;
	(void)write_fmr(out_fp, ofmr);
	free_fmr(ifmr);
	free_fmr(ofmr);

	close_files();

	exit(EXIT_SUCCESS);

err_out:
	if (ifmr != NULL)
		free_fmr(ifmr);
	if (ofmr != NULL)
		free_fmr(ofmr);

	close_files();

	exit(EXIT_FAILURE);
}
