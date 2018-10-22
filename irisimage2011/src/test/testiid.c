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
/* This program uses the Iris Image Biometric Data Block library to print     */
/* the contents of a file containing iris image records according to the      */
/* ISO/IEC 19794-6:2011 standard. The record can be optionally validated.     */
/* The file may contain more than one image biometric data block.             */
/******************************************************************************/
#define _XOPEN_SOURCE	1
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

static void
print_iibdb_stats(IIBDB *iibdb)
{
	IGH *igh;

	igh = &iibdb->general_header;

	printf("IRH count is %d; ", get_irh_count(iibdb));
	printf("Header info: ");
	printf("\t%s %s %u %hu %hhu %hhu\n",
	    igh->format_id, igh->format_version, igh->record_length,
	     igh->num_irises, igh->cert_flag, igh->num_eyes);
}

int main(int argc, char *argv[])
{
	char *usage = "usage: testiid <datafile>\n";
	FILE *infp, *outfp;
	struct stat sb;
	IIBDB *iibdb;
	BDB bdb;
	uint8_t *buf;
	int ret;

	if (argc != 2) {
		printf("%s\n", usage);
		exit (EXIT_FAILURE);
	}

	if (argv[1] == NULL) {
		printf("%s\n", usage);
		exit (EXIT_FAILURE);
	}

	infp = fopen(argv[1], "rb");
	if (infp == NULL)
		ERR_EXIT("Open of %s failed: %s\n", argv[1],
		    strerror(errno));

	if (new_iibdb(&iibdb) < 0)
		ALLOC_ERR_EXIT("Iris Image Biometric Data Block");

	if (fstat(fileno(infp), &sb) < 0)
		ERR_EXIT("Could not get stats on input file");

	/*
	 * Test the read functions.
	 */
	printf("Testing the read functions...\n");
	if (read_iibdb(infp, iibdb) != READ_OK)
		ERR_EXIT("Library read of %s failed: %s\n", argv[1],
		    strerror(errno));
	print_iibdb_stats(iibdb);
	fclose(infp);

	/*
	 * Test the push functions by writing the data to a file from the
	 * memory blob, then read the file using the library again.
	 */
	printf("Testing the push functions...\n");
	buf = (uint8_t *)malloc(iibdb->general_header.record_length);
        if (buf == NULL)
		ALLOC_ERR_EXIT("Temporary buffer");
	INIT_BDB(&bdb, buf, iibdb->general_header.record_length);
	if (push_iibdb(&bdb, iibdb) != WRITE_OK) {
		fprintf(stderr, "could not push Iris Image record\n");
		exit (EXIT_FAILURE);
	}
	outfp = tmpfile();
	if (outfp == NULL)
		OPEN_ERR_EXIT("Temporary file");
	if (fwrite(bdb.bdb_start, 1, bdb.bdb_size, outfp) != bdb.bdb_size) {
		fprintf(stderr, "Write of temp file failed: %s\n",
		    strerror(errno));
		exit (EXIT_FAILURE);
	}
	free_iibdb(iibdb);
	new_iibdb(&iibdb);
	rewind(outfp);
	if (read_iibdb(outfp, iibdb) != READ_OK)
		ERR_EXIT("Library read of temporary file %s failed: %s\n",
		    argv[1], strerror(errno));
	print_iibdb_stats(iibdb);
	fclose(outfp);
	
	/*
	 * Test the write functions.
	 */
	printf("Testing the write functions...\n");
	outfp = tmpfile();
	if (outfp == NULL)
		OPEN_ERR_EXIT("Temporary file");
	if (write_iibdb(outfp, iibdb) != WRITE_OK) {
		fprintf(stderr, "Could not write Iris Image record\n");
		exit (EXIT_FAILURE);
	}

	/*
	 * Test the scan functions by reading in the copy of the output file
	 * into memory, then scanning that memory.
	 */
	printf("Testing the scan functions...\n");
	rewind(outfp);
	fread(buf, 1, iibdb->general_header.record_length, outfp);
	INIT_BDB(&bdb, buf, iibdb->general_header.record_length);
	free_iibdb(iibdb);
	new_iibdb(&iibdb);
	ret = scan_iibdb(&bdb, iibdb);
	if (ret != READ_OK)
		ERR_OUT("Could not scan record.");
	print_iibdb_stats(iibdb);
	fclose(outfp);

err_out:
	free_iibdb(iibdb);
	free(buf);

	exit (EXIT_SUCCESS);
}
