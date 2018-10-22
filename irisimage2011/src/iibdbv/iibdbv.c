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
/* This program uses the Iris Image Biometric Data Block library to validate  */
/* the contents of a file containing iris image records according to the      */
/* ISO/IEC 19794-6:2005 standard. The record can be optionally validated.     */
/* The file may contain more than one image biometric data block.             */
/******************************************************************************/
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <biomdi.h>
#include <biomdimacro.h>
#include <iid.h>

static void validate_irh_in_iibd(IIBDB *iibdb)
{
	IRH **irh;
	unsigned int num, n;
	/*
	 * Validate the image headers.
	 */
	num = get_irh_count(iibdb);
	if (num == 0) {
		printf("\tThere are no image record headers.\n");
	} else {
		irh = (IRH **)malloc(num * sizeof(IRH *));
		if (irh == NULL)
			ALLOC_ERR_OUT("\tIris image header");
		if (get_irhs(iibdb, irh) != num) {
			ERRP("\tCould not get image headers");
		} else {
			for (n = 0; n < num; n++) {
				printf("\tImage header %d ", n + 1);
				if (validate_irh(irh[n]) != VALIDATE_OK)
					printf("is NOT valid.\n");
				else
					printf("is valid.\n");
			}
		}
	}
err_out:
	return;
}

int main(int argc, char *argv[])
{
	char *usage = "usage: iibdbv <datafile>\n";
	FILE *fp;
	struct stat sb;
	IIBDB *iibdb;
	int ret;
	unsigned int total_length;
	unsigned int count;
	int status;

	if (argc != 2) {
		printf("%s\n", usage);
		exit (EXIT_FAILURE);
	}
				
	fp = fopen(argv[1], "rb");
	if (fp == NULL)
		ERR_EXIT("Open of %s failed: %s\n", argv[1],
		    strerror(errno));

	if (new_iibdb(&iibdb) < 0)
		ALLOC_ERR_EXIT("Iris Image Biometric Data Block");

	if (stat(argv[1], &sb) < 0)
		ERR_EXIT("Could not get stats on input file");

	total_length = 0;
	count = 0;
	status = EXIT_SUCCESS;
	while (total_length < sb.st_size) {
		ret = read_iibdb(fp, iibdb);
		if (ret != READ_OK) {
			status = EXIT_FAILURE;
			break;
		}
		count++;
		printf("Iris Image Data Record %u ", count);
		if (iibdb->general_header.record_length == 0) {
			printf("Record length for record %u is 0.\n", count);
			status = EXIT_FAILURE;
		} else {
			total_length += iibdb->general_header.record_length;
		}

		if (validate_iibdb(iibdb) != VALIDATE_OK) {
			printf("is NOT valid.\n");
			printf("\nValidating components:\n");
			validate_irh_in_iibd(iibdb);
			status = EXIT_FAILURE;
		} else {
			printf("is valid.\n");
			
		}
		free_iibdb(iibdb);
		if (new_iibdb(&iibdb) < 0)
			ALLOC_ERR_EXIT("Iris Image Biometric Data Block");
	}
	exit (status);
}
