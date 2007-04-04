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
/* Implementation of the Finger Minutiae Data record processing interface.    */
/*                                                                            */
/******************************************************************************/
#include <sys/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fmr.h>
#include <biomdimacro.h>

int
new_fmd(unsigned int format_std, struct finger_minutiae_data **fmd)
{
	struct finger_minutiae_data *lfmd;
	lfmd = (struct finger_minutiae_data *)malloc(
		sizeof(struct finger_minutiae_data));
	if (lfmd == NULL) {
		perror("Failed to allocate Finger Minutiae Data record");
		return (-1);
	}
	memset((void *)lfmd, 0, sizeof(struct finger_minutiae_data));
	lfmd->format_std = format_std;
	*fmd = lfmd;
	return 0;
}

void
free_fmd(struct finger_minutiae_data *fmd)
{
	free(fmd);
}

int
read_fmd(FILE *fp, struct finger_minutiae_data *fmd)
{
	unsigned short sval;
	unsigned char cval;

	// Type/X Coord
	SREAD(&sval, fp);
	fmd->type = (unsigned char)
			((sval & FMD_MINUTIA_TYPE_MASK) >> 
				FMD_MINUTIA_TYPE_SHIFT);
	fmd->x_coord = sval & FMD_X_COORD_MASK;

	// Y Coord
	SREAD(&sval, fp);

	// We save the reserved field for conformance checking
	fmd->reserved = (unsigned char)
			((sval & FMD_RESERVED_MASK) >> 
				FMD_RESERVED_SHIFT);
	fmd->y_coord = sval & FMD_Y_COORD_MASK;

	// Minutia angle
	CREAD(&cval, fp);
	fmd->angle = cval;

	// Minutia quality
	CREAD(&cval, fp);
	fmd->quality = cval;

	return READ_OK;

eof_out:
	ERRP("EOF encountered in %s", __FUNCTION__);
	return READ_EOF;
err_out:
	return READ_ERROR;
}

int
write_fmd(FILE *fp, struct finger_minutiae_data *fmd)
{
	unsigned short sval;
	unsigned char cval;

	// Type/X Coord
	sval = (unsigned short)(fmd->type << FMD_MINUTIA_TYPE_SHIFT);
	sval = sval | (fmd->x_coord  & FMD_X_COORD_MASK);
	SWRITE(&sval, fp);

	// Y Coord/Reserved
	sval = fmd->y_coord & FMD_Y_COORD_MASK;
	SWRITE(&sval, fp);

	// Minutia angle
	cval = fmd->angle;
	CWRITE(&cval, fp);

	// Minutia quality
	cval = fmd->quality;
	CWRITE(&cval, fp);

	return WRITE_OK;

err_out:
	return WRITE_ERROR;
}

int
print_fmd(FILE *fp, struct finger_minutiae_data *fmd)
{
	fprintf(fp, "Finger Minutiae Data:\n");
	fprintf(fp, "\tType\t\t: 0x%01x\n", fmd->type);
	fprintf(fp, "\tCoordinate\t: (%u,%u)\n", fmd->x_coord, fmd->y_coord);
	fprintf(fp, "\tAngle\t\t: %u\n", fmd->angle);
	fprintf(fp, "\tQuality\t\t: %u\n", fmd->quality);
	return 0;
}

int
validate_fmd(struct finger_minutiae_data *fmd)
{
	unsigned short coord;
	int ret = VALIDATE_OK;

	// The coordinates must lie within the scanned image
	coord = fmd->fvmr->fmr->x_image_size - 1;
	if (fmd->x_coord > coord) {
	  ERRP("X-coordinate (%u) of Finger Minutia Data lies outside image",
		fmd->x_coord);
		ret = VALIDATE_ERROR;
	}
	coord = fmd->fvmr->fmr->y_image_size - 1;
	if (fmd->y_coord > coord) {
	  ERRP("Y-coordinate (%u) of Finger Minutia Data lies outside image",
		fmd->y_coord);
		ret = VALIDATE_ERROR;
	}
	
	// Minutia type is one of these values
	if ((fmd->type != MINUTIA_TYPE_OTHER) &
	    (fmd->type != MINUTIA_TYPE_RIDGE_ENDING) &
	    (fmd->type != MINUTIA_TYPE_BIFURCATION)) {
		ERRP("Minutia Type %u is not valid", fmd->type);
		ret = VALIDATE_ERROR;
	}

	// Reserved field must be '00'
	if (fmd->reserved != 0) {
		ERRP("Minutia Reserved is %u, should be '00'",
			fmd->quality, MIN_MINUTIA_ANGLE, MAX_MINUTIA_ANGLE);
		ret = VALIDATE_ERROR;
	}

	// Position; XXX should this be reasonably related to the 
	// size/resolution?

	// Angle
	if ((fmd->angle < MIN_MINUTIA_ANGLE) |
	    (fmd->angle > MAX_MINUTIA_ANGLE)) {
		ERRP("Minutia angle %u is out of range %u-%u",
			fmd->angle, MIN_MINUTIA_ANGLE, MAX_MINUTIA_ANGLE);
		ret = VALIDATE_ERROR;
	}

	// Quality
	if ((fmd->quality < MIN_MINUTIA_QUALITY) |
	    (fmd->quality > MAX_MINUTIA_QUALITY)) {
		ERRP("Minutia quality %u is out of range %u-%u",
			fmd->quality, MIN_MINUTIA_QUALITY, MAX_MINUTIA_QUALITY);
		ret = VALIDATE_ERROR;
	}

	return ret;
}

/******************************************************************************/
/* Find the center-of-mass for a set of minutiae.                             */
/******************************************************************************/
int find_center_of_minutiae_mass(FMD **fmds, int mcount, int *x, int *y)
{
	int lx, ly, i;

	lx = ly = 0;
	for (i = 0; i < mcount; i++) {
		lx += fmds[i]->x_coord;
		ly += fmds[i]->y_coord;
	}
	*x = lx / mcount;
	*y = ly / mcount; 
}
