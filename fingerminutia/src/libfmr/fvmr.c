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
/* Manipulate Finger View records. The FV record is not explicitly part of    */
/* the ANSI-INCITS spec. This package combines the Finger View record         */
/* processing with the Finger View Minutiae Record.                           */
/*                                                                            */
/******************************************************************************/
#include <sys/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <biomdi.h>
#include <biomdimacro.h>
#include <fmr.h>

/******************************************************************************/
/* Implement the interface for allocating and freeing Finger View Minutiae    */
/* Records                                                                    */
/******************************************************************************/
int
new_fvmr(unsigned int format_std, struct finger_view_minutiae_record **fvmr)
{
	struct finger_view_minutiae_record *lfvmr;

	lfvmr = (struct finger_view_minutiae_record *)malloc(
			sizeof(struct finger_view_minutiae_record));
	if (lfvmr == NULL) {
		perror("Failed to allocate Finger View Minutiae Record");
		return -1;
	}
	memset((void *)lfvmr, 0, sizeof(struct finger_view_minutiae_record));
	lfvmr->format_std = format_std;
	lfvmr->extended = NULL;
	lfvmr->partial = FALSE;
	TAILQ_INIT(&lfvmr->minutiae_data);
	*fvmr = lfvmr;
	return 0;
}

void
free_fvmr(struct finger_view_minutiae_record *fvmr)
{
	struct finger_minutiae_data *fmd;

	// Free the Finger Minutiae Records contained within the FVMR
	while (!TAILQ_EMPTY(&fvmr->minutiae_data)) {
		fmd = TAILQ_FIRST(&fvmr->minutiae_data);
		TAILQ_REMOVE(&fvmr->minutiae_data, fmd, list);
		free_fmd(fmd);
	}
	if (fvmr->extended != NULL) {
		free_fedb(fvmr->extended);
	}

	// Free the FVMR itself
	free(fvmr);
}

/******************************************************************************/
/* Implement the interface for reading and writing Finger View Minutiae       */
/* records.                                                                   */
/******************************************************************************/
static int
internal_read_fvmr(FILE *fp, BDB *fmdb,
    struct finger_view_minutiae_record *fvmr)
{
	unsigned char cval;
	unsigned short sval;
	int i;
	struct finger_minutiae_data *fmd;
	struct finger_extended_data_block *fedb;
	int ret;

	/* ISO normal and compact card formats don't have a finger view
	 * header, so we will just read the minutiae data directly.
	 */
	if ((fvmr->format_std == FMR_STD_ISO_NORMAL_CARD) ||
	    (fvmr->format_std == FMR_STD_ISO_COMPACT_CARD)) {
		ret = READ_OK;
		i = 1;
		while (ret == READ_OK) {
			if (new_fmd(fvmr->format_std, &fmd, i) < 0)
				ERR_OUT("Could not allocate FMD %d", i);

			if (fp != NULL)
				ret = read_fmd(fp, fmd);
			else
				ret = scan_fmd(fmdb, fmd);
			if (ret == READ_OK) {
				add_fmd_to_fvmr(fmd, fvmr);
				i++;
				fvmr->number_of_minutiae++;
			} else if (ret == READ_EOF)
				return READ_OK;
			else 
				ERR_OUT("Could not read FMD %d", i);
		}
	}

	// Finger number
	CGET(&cval, fp, fmdb);
	fvmr->finger_number = cval;

	// View number/impression type
	CGET(&cval, fp, fmdb);
	fvmr->view_number = (cval & FVMR_VIEW_NUMBER_MASK) >> 
				FVMR_VIEW_NUMBER_SHIFT;
	fvmr->impression_type = (unsigned short)(cval & FVMR_IMPRESSION_MASK);

	// Finger quality
	CGET(&cval, fp, fmdb);
	fvmr->finger_quality = (unsigned short)cval;

	// Number of minutiae
	CGET(&cval, fp, fmdb);
	fvmr->number_of_minutiae = cval;

	// Finger minutiae data
	for (i = 0; i < fvmr->number_of_minutiae; i++) {
		if (new_fmd(fvmr->format_std, &fmd, i) < 0)
			ERR_OUT("Could not allocate FMD %d", i);

		if (fp != NULL)
			ret = read_fmd(fp, fmd);
		else
			ret = scan_fmd(fmdb, fmd);
		if (ret == READ_OK)
			add_fmd_to_fvmr(fmd, fvmr);
		else if (ret == READ_EOF)
			goto eof_out;
		else 
			ERR_OUT("Could not read FMD %d", i);
	}

	// Read the extended data block, if it exists
	if (new_fedb(fvmr->format_std, &fedb) < 0)
		ERR_OUT("Could not allocate extended data block");

	if (fp != NULL)
		ret = read_fedb(fp, fedb);
	else
		ret = scan_fedb(fmdb, fedb);
	if (ret == READ_ERROR)
		ERR_OUT("Could not read extended data block");

	// EOF is OK as we may have read a partial block
	if (fedb->partial)
		fvmr->partial = TRUE;
	if (fedb->block_length != 0)
		add_fedb_to_fvmr(fedb, fvmr);
	else
		free_fedb(fedb);

	return ret;

eof_out:
	ERRP("EOF while reading Finger View Minutiae Record");
	return READ_EOF;
err_out:
	return READ_ERROR;
}

int
read_fvmr(FILE *fp, struct finger_view_minutiae_record *fvmr)
{
	return (internal_read_fvmr(fp, NULL, fvmr));
}

int
scan_fvmr(BDB *fmdb, struct finger_view_minutiae_record *fvmr)
{
	return (internal_read_fvmr(NULL, fmdb, fvmr));
}

static int
internal_write_fvmr(FILE *fp, BDB *fmdb,
    struct finger_view_minutiae_record *fvmr)
{
	struct finger_minutiae_data *fmd;
	unsigned char cval;
	unsigned short sval;
	int ret;

	/* ISO normal and compact card formats don't have a finger view
	 * header, so we will just write the minutiae data directly.
	 */
	if ((fvmr->format_std == FMR_STD_ISO_NORMAL_CARD) ||
	    (fvmr->format_std == FMR_STD_ISO_COMPACT_CARD)) {
		TAILQ_FOREACH(fmd, &fvmr->minutiae_data, list) {
			if (write_fmd(fp, fmd) != WRITE_OK)
				ERR_OUT("Could not write minutiae data");
		}
		return WRITE_OK;
	}

	// Finger number
	CPUT(&fvmr->finger_number, fp, fmdb);

	// View number/impression type
	cval = (fvmr->view_number << FVMR_VIEW_NUMBER_SHIFT) | 
		fvmr->impression_type;
	CPUT(&cval, fp, fmdb);

	// Finger quality
	CPUT(&fvmr->finger_quality, fp, fmdb);

	// Number of minutiae
	CPUT(&fvmr->number_of_minutiae, fp, fmdb);

	// Write each Finger Minutiae Data record
	TAILQ_FOREACH(fmd, &fvmr->minutiae_data, list) {
		if (fp != NULL)
			ret = write_fmd(fp, fmd);
		else
			ret = push_fmd(fmdb, fmd);
		if (ret != WRITE_OK)
			ERR_OUT("Could not write minutiae data");
	}

	// Write the extended data
	if (fp != NULL)
		ret = write_fedb(fp, fvmr->extended);
	else
		ret = push_fedb(fmdb, fvmr->extended);
	if (ret != WRITE_OK)
		ERR_OUT("Could not write extended data block");

	return WRITE_OK;

err_out:
	return WRITE_ERROR;
}

int
write_fvmr(FILE *fp, struct finger_view_minutiae_record *fvmr)
{
	return (internal_write_fvmr(fp, NULL, fvmr));
}

int
push_fvmr(BDB *fmdb, struct finger_view_minutiae_record *fvmr)
{
	return (internal_write_fvmr(NULL, fmdb, fvmr));
}

int
print_fvmr(FILE *fp, struct finger_view_minutiae_record *fvmr)
{
	struct finger_minutiae_data *fmd;
	int i;

	if ((fvmr->format_std == FMR_STD_ANSI) ||
	    (fvmr->format_std == FMR_STD_ISO)) {

		fprintf(fp, "----------------------------------------------------\n");
		fprintf(fp, "Finger View Minutia Record:\n");
		fprintf(fp, "\tFinger Number\t\t: %u\n", fvmr->finger_number);
		fprintf(fp, "\tView Number\t\t: %u\n", fvmr->view_number);
		fprintf(fp, "\tImpression Type\t\t: %u\n",
		    fvmr->impression_type);
		fprintf(fp, "\tFinger Quality\t\t: %u\n", fvmr->finger_quality);
		fprintf(fp, "\tNumber of Minutiae\t: %u\n",
		    fvmr->number_of_minutiae);
		fprintf(fp, "\n");
	}

	i = 1;
	TAILQ_FOREACH(fmd, &fvmr->minutiae_data, list) {
		fprintf(fp, "(%03d) ", fmd->index);
		if (print_fmd(fp, fmd) != PRINT_OK)
			ERR_OUT("Could not print minutiae data");
	}
	if (fvmr->extended != NULL)
		if (print_fedb(fp, fvmr->extended) != PRINT_OK)
			ERR_OUT("Could not write extended data block");

	fprintf(fp, "----------------------------------------------------\n");
	return PRINT_OK;

err_out:
	return PRINT_ERROR;
}

int
validate_fvmr(struct finger_view_minutiae_record *fvmr)
{
	struct finger_minutiae_data *fmd;
	int ret = VALIDATE_OK;
	int valid;
	struct finger_minutiae_record *fmr = fvmr->fmr;

	if ((fvmr->format_std == FMR_STD_ANSI) ||
	    (fvmr->format_std == FMR_STD_ISO)) {

		// Validate the finger number, view number, etc.

		// Note that the finger number check assumes that the range of
		// finger numbers is continuous
		if ((fvmr->finger_number < MIN_FINGER_CODE) || 
		    (fvmr->finger_number > FMR_MAX_FINGER_CODE)) {
			ERRP("Finger number of %u is out of range %u-%u",
			    fvmr->finger_number, MIN_FINGER_CODE,
			    MAX_FINGER_CODE);
			ret = VALIDATE_ERROR;
	}
		// View number
		// The view numbers must increase, starting with 0. The expected
		// minimum finger number is stored in the FMR.
		if ((fmr->next_min_view[fvmr->finger_number] == 0) && 
		    (fvmr->view_number != 0)) {
			ERRP("First view number for finger position %u is %u; "
			    "must start with 0", fvmr->finger_number,
			    fvmr->view_number);
			ret = VALIDATE_ERROR;
		} else if (fvmr->view_number < 
			    fmr->next_min_view[fvmr->finger_number]) {
			ERRP("View number of %u for finger position %u is out"
			    " of sync, expecting minimum value of %u",
			    fvmr->view_number, fvmr->finger_number,
			    fmr->next_min_view[fvmr->finger_number]);
			ret = VALIDATE_ERROR;
		} else {
			fmr->next_min_view[fvmr->finger_number] =
			    fvmr->view_number + 1;
		}

		// Validate impression type code
		if ((fvmr->impression_type != LIVE_SCAN_PLAIN) &&
		    (fvmr->impression_type != LIVE_SCAN_ROLLED) &&
		    (fvmr->impression_type != NONLIVE_SCAN_PLAIN) &&
		    (fvmr->impression_type != NONLIVE_SCAN_ROLLED) &&
		    (fvmr->impression_type != SWIPE) &&
		    (fvmr->impression_type != LIVE_SCAN_CONTACTLESS)) {
			ERRP("Impression Type %u is invalid",
				fvmr->impression_type);
			ret = VALIDATE_ERROR;
		}

		// Finger Quality
		if ((fvmr->finger_quality < FMR_MIN_FINGER_QUALITY) ||
		    (fvmr->finger_quality > FMR_MAX_FINGER_QUALITY)) {
			ERRP("Finger Quality %u is out of range %u-%u",
				fvmr->finger_quality, FMR_MIN_FINGER_QUALITY, 
				FMR_MAX_FINGER_QUALITY);
			ret = VALIDATE_ERROR;
		}

	}

	// Number of Minutiae is not constrained by the spec
	// Validate each minutuia data record
	TAILQ_FOREACH(fmd, &fvmr->minutiae_data, list) {
		valid = validate_fmd(fmd);
		if (valid != VALIDATE_OK) {
			ret = VALIDATE_ERROR;
		}
	}

	// Validate the extended data, if present
	if (fvmr->extended != NULL) {
		valid = validate_fedb(fvmr->extended);
		if (valid != VALIDATE_OK) {
			ret = VALIDATE_ERROR;
		}
	}

	return ret;
}

void
add_fmd_to_fvmr(struct finger_minutiae_data *fmd,
                struct finger_view_minutiae_record *fvmr)
{
	fmd->fvmr = fvmr;
	TAILQ_INSERT_TAIL(&fvmr->minutiae_data, fmd, list);
}

void
add_fedb_to_fvmr(struct finger_extended_data_block *fedb,
		 struct finger_view_minutiae_record *fvmr)
{
		fvmr->extended = fedb;
		fedb->fvmr = fvmr;
}

/******************************************************************************/
/* Implementation of the higher level access routines.                        */
/******************************************************************************/
int
get_minutiae_count(struct finger_view_minutiae_record *fvmr)
{
	return (int)fvmr->number_of_minutiae;
}

int
get_minutiae(struct finger_view_minutiae_record *fvmr,
	     struct finger_minutiae_data *fmds[])
{
	int count = 0;
	struct finger_minutiae_data *fmd;

	TAILQ_FOREACH(fmd, &fvmr->minutiae_data, list) {
		fmds[count] = fmd;
		count++;
	}

	return count;
}

int
get_ridge_record_count(struct finger_view_minutiae_record *fvmr)
{
	struct finger_extended_data *fed;
	struct ridge_count_data *rcd;
	int count = 0;

	if (fvmr->extended == NULL)
		return 0;

	// Because there is no count in the FMR for the number of Ridge Count
	// records, we calculate the total here
	TAILQ_FOREACH(fed, &fvmr->extended->extended_data, list) {
		if (fed->type_id == FED_RIDGE_COUNT)
			TAILQ_FOREACH(rcd, &fed->rcdb->ridge_counts, list)
				count++; 
	}

	return count;
}

int
get_ridge_records(struct finger_view_minutiae_record *fvmr,
	 struct ridge_count_data *rcds[])
{
	struct finger_extended_data *fed;
	struct ridge_count_data *rcd;
	int count = 0;

	TAILQ_FOREACH(fed, &fvmr->extended->extended_data, list) {
		if (fed->type_id == FED_RIDGE_COUNT)
			TAILQ_FOREACH(rcd, &fed->rcdb->ridge_counts, list) {
				rcds[count] = rcd;
				count++; 
		}
	}
	return count;
}

int
get_core_record_count(struct finger_view_minutiae_record *fvmr)
{
	struct finger_extended_data *fed;
	int count = 0;

	if (fvmr->extended == NULL)
		return 0;

	// Add up the core count from each Core-Delta data block because
	// there my be more than one CDDB in the extended data
	TAILQ_FOREACH(fed, &fvmr->extended->extended_data, list) {
		if (fed->type_id == FED_CORE_AND_DELTA)
			count += fed->cddb->num_cores;
	}
	return count;
}

int
get_core_records(struct finger_view_minutiae_record *fvmr,
		 struct core_data *cores[])
{
	struct finger_extended_data *fed;
	struct core_data *core;
	int count = 0;

	TAILQ_FOREACH(fed, &fvmr->extended->extended_data, list) {
		if (fed->type_id == FED_CORE_AND_DELTA)
			TAILQ_FOREACH(core, &fed->cddb->cores, list) {
				cores[count] = core;
				count++;
			}
	}
	return count;
}

int
get_delta_record_count(struct finger_view_minutiae_record *fvmr)
{
	struct finger_extended_data *fed;
	int count = 0;

	if (fvmr->extended == NULL)
		return 0;

	// Add up the delta count from each Core-Delta data block because
	// there my be more than one CDDB in the extended data
	TAILQ_FOREACH(fed, &fvmr->extended->extended_data, list) {
		if (fed->type_id == FED_CORE_AND_DELTA)
			count += fed->cddb->num_deltas;
	}

	return count;
}

int
get_delta_records(struct finger_view_minutiae_record *fvmr,
	   struct delta_data *deltas[])
{
	struct finger_extended_data *fed;
	struct delta_data *delta;
	int count = 0;

	TAILQ_FOREACH(fed, &fvmr->extended->extended_data, list) {
		if (fed->type_id == FED_CORE_AND_DELTA)
			TAILQ_FOREACH(delta, &fed->cddb->deltas, list) {
				deltas[count] = delta;
				count++;
			}
	}
	return count;
}
