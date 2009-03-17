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
/* Representation of a Facial Block to contain the facial image information   */
/* contained within a ANSI INCITS 385-2004 data record. The Facial Block      */
/* represents the Facial Header and 0 or more Facial Data blocks.             */
/*                                                                            */
/* This package implements the routines to read and write a Facial Image      */
/* Record, and print that information in human-readable form.                 */
/*                                                                            */
/* Routines are provided to manage memory for Facial Block representations.   */
/*                                                                            */
/******************************************************************************/
#include <sys/queue.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <frf.h>
#include <biomdimacro.h>

int
new_fb(struct facial_block **fb)
{
	struct facial_block *lfb;

	lfb = (struct facial_block *)malloc(sizeof(struct facial_block));
	if (lfb == NULL) {
		perror("Failed to allocate Facial Block");
		return -1;
	}
	memset((void *)lfb, 0, sizeof(struct facial_block));
	TAILQ_INIT(&lfb->facial_data);

	*fb = lfb;
	return 0;
}

void
free_fb(struct facial_block *fb)
{
	struct facial_data_block *fdb;

	// Free the Facial Data Blocks contained within the Facial Block
	while (!TAILQ_EMPTY(&fb->facial_data)) {
		fdb = TAILQ_FIRST(&fb->facial_data);
		TAILQ_REMOVE(&fb->facial_data, fdb, list);
		free_fdb(fdb);
	}
	free(fb);
}

int
read_fb(FILE *fp, struct facial_block *fb)
{
	unsigned int i;
	int ret;
	struct facial_data_block *fdb;

	// Format ID
	OREAD(fb->format_id, 1, FRF_FORMAT_ID_LENGTH, fp);

	// Version Number
	OREAD(fb->version_num, 1, FRF_VERSION_NUM_LENGTH, fp);

	// Length of Record
	LREAD(&fb->record_length, fp);

	// Number of Faces
	SREAD(&fb->num_faces, fp);

	// Read the Facial Data Blocks
	for (i = 1; i <= fb->num_faces; i++) {
		if (new_fdb(&fdb) < 0) {
			fprintf(stderr, "error allocating FDB %d\n", i);
			goto err_out;
		}
		ret = read_fdb(fp, fdb);
		if (ret == READ_OK)
			add_fdb_to_fb(fdb, fb);
		else if (ret == READ_EOF)
			goto eof_out;
		else
			goto err_out;
	}

	return READ_OK;

eof_out:
	return READ_EOF;
err_out:
	return READ_ERROR;
}

int
write_fb(FILE *fp, struct facial_block *fb)
{
	int ret;
	struct facial_data_block *fdb;

	OWRITE(fb->format_id, 1, FRF_FORMAT_ID_LENGTH, fp);

	OWRITE(fb->version_num, 1, FRF_VERSION_NUM_LENGTH, fp);

	LWRITE(fb->record_length, fp);

	SWRITE(fb->num_faces, fp);

	TAILQ_FOREACH(fdb, &fb->facial_data, list) {
		ret = write_fdb(fp, fdb);
		if (ret != WRITE_OK)
			goto err_out;
	}

	return WRITE_OK;

err_out:
	return WRITE_ERROR;
}

int
print_fb(FILE *fp, struct facial_block *fb)
{
	int ret;
	struct facial_data_block *fdb;

	// Facial Header
	FPRINTF(fp, "Format ID\t\t: %s\nSpec Version\t\t: %s\n",
		fb->format_id, fb->version_num);

	FPRINTF(fp, "Record Length\t\t: %u\n", fb->record_length);

	FPRINTF(fp, "Number of Faces\t\t: %u\n", fb->num_faces);

	// Print the Facial Data Blocks
	TAILQ_FOREACH(fdb, &fb->facial_data, list) {
		ret = print_fdb(fp, fdb);
		if (ret != PRINT_OK)
			goto err_out;
	}

	return PRINT_OK;

err_out:
	return PRINT_ERROR;
}

void
add_fdb_to_fb(struct facial_data_block *fdb, struct facial_block *fb)
{
	fdb->fb = fb;
	TAILQ_INSERT_TAIL(&fb->facial_data, fdb, list);
}

