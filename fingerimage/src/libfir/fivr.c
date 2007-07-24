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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <biomdi.h>
#include <biomdimacro.h>
#include <fir.h>

/******************************************************************************/
/* Implement the interface for allocating and freeing Image View records      */
/******************************************************************************/
int
new_fivr(struct finger_image_view_record **fivr)
{
	struct finger_image_view_record *lfivr;

	lfivr = (struct finger_image_view_record *)malloc(
	    sizeof(struct finger_image_view_record));
	if (lfivr == NULL) {
		perror("Failed allocating memory for FIVR");
		return (-1);
	}
	memset((void *)lfivr, 0, sizeof(struct finger_image_view_record));
	*fivr = lfivr;
	return (0);
}

void
free_fivr(struct finger_image_view_record *fivr)
{
	if (fivr->image_data != NULL)
		free(fivr->image_data);
	free (fivr);
}

void
add_image_to_fivr(char *image, struct finger_image_view_record *fivr)
{
	fivr->image_data = image;
}

/******************************************************************************/
/* Implement the interface for reading and writing Finger Image View records  */
/******************************************************************************/

int
read_fivr(FILE *fp, struct finger_image_view_record *fivr)
{
	LREAD(&fivr->length, fp);
	CREAD(&fivr->finger_palm_position, fp);
	CREAD(&fivr->count_of_views, fp);
	CREAD(&fivr->view_number, fp);
	CREAD(&fivr->quality, fp);
	CREAD(&fivr->impression_type, fp);
	SREAD(&fivr->horizontal_line_length, fp);
	SREAD(&fivr->vertical_line_length, fp);
	CREAD(&fivr->reserved, fp);
	// XXX Need stronger constraints here on length
	if (fivr->length > FIVR_HEADER_LENGTH) {
		fivr->image_data = (char *)malloc(
		    fivr->length - FIVR_HEADER_LENGTH);
		if (fivr->image_data == NULL)
			ERR_OUT("Could not allocate memory for image data");
		else
			OREAD(fivr->image_data, 1,
			    fivr->length - FIVR_HEADER_LENGTH, fp);
	}
	return (READ_OK);
eof_out:
	ERRP("EOF during read of FIVR encountered in %s", __FUNCTION__);
	return (READ_EOF);
err_out:
	return (READ_ERROR);
}

int
write_fivr(FILE *fp, struct finger_image_view_record *fivr)
{
	LWRITE(&fivr->length, fp);
	CWRITE(&fivr->finger_palm_position, fp);
	CWRITE(&fivr->count_of_views, fp);
	CWRITE(&fivr->view_number, fp);
	CWRITE(&fivr->quality, fp);
	CWRITE(&fivr->impression_type, fp);
	SWRITE(&fivr->horizontal_line_length, fp);
	SWRITE(&fivr->vertical_line_length, fp);
	CWRITE(&fivr->reserved, fp);
	if (fivr->image_data != NULL) {
		OWRITE(fivr->image_data, sizeof(char),
			    fivr->length - FIVR_HEADER_LENGTH, fp);
	}
	return (WRITE_OK);
err_out:
	return (WRITE_ERROR);
}

int
print_fivr(FILE *fp, struct finger_image_view_record *fivr)
{
	FPRINTF(fp, "--- Finger Image View Record ---\n");
	FPRINTF(fp, "Length\t\t\t: %u\n", fivr->length);
	FPRINTF(fp, "Position\t\t: %u\n", fivr->finger_palm_position);
	FPRINTF(fp, "Count of views\t\t: %u\n", fivr->count_of_views);
	FPRINTF(fp, "View number\t\t: %u\n", fivr->view_number);
	FPRINTF(fp, "Quality\t\t\t: %u\n", fivr->quality);
	FPRINTF(fp, "Impression type\t\t: %u\n", fivr->impression_type);
	FPRINTF(fp, "Image size\t\t: %u X %u\n", fivr->horizontal_line_length,
	    fivr->vertical_line_length);
	FPRINTF(fp, "Reserved\t\t: %u\n", fivr->reserved);
	return (PRINT_OK);
err_out:
	return (PRINT_ERROR);
}

static int
validate_finger_palm_position(unsigned char code)
{
	switch (code) {
	case UNKNOWN_FINGER:
	case RIGHT_THUMB:
	case RIGHT_INDEX:
	case RIGHT_MIDDLE:
	case RIGHT_RING:
	case RIGHT_LITTLE:
	case LEFT_THUMB:
	case LEFT_INDEX:
	case LEFT_MIDDLE:
	case LEFT_RING:
	case LEFT_LITTLE:
	case PLAIN_RIGHT_FOUR:
	case PLAIN_LEFT_FOUR:
	case PLAIN_THUMBS:
	case UNKNOWN_PALM:
	case RIGHT_FULL_PALM:
	case RIGHT_WRITERS_PALM:
	case LEFT_FULL_PALM:
	case LEFT_WRITERS_PALM:
	case RIGHT_LOWER_PALM:
	case RIGHT_UPPER_PALM:
	case LEFT_LOWER_PALM:
	case LEFT_UPPER_PALM:
	case RIGHT_OTHER_PALM:
	case LEFT_OTHER_PALM:
	case RIGHT_INTERDIGITAL_PALM:
	case RIGHT_THENAR_PALM:
	case RIGHT_HYPOTHENAR_PALM:
	case LEFT_INTERDIGITAL_PALM:
	case LEFT_THENAR_PALM:
	case LEFT_HYPOTHENAR_PALM:
		return (VALIDATE_OK);
		break;
	default:
		return (VALIDATE_ERROR);
		break;
	}
}

static int
validate_impression_type(unsigned char code)
{
	switch (code) {
	case LIVE_SCAN_PLAIN:
	case LIVE_SCAN_ROLLED:
	case NONLIVE_SCAN_PLAIN:
	case NONLIVE_SCAN_ROLLED:
	case LATENT:
	case SWIPE:
	case LIVE_SCAN_CONTACTLESS:
		return (VALIDATE_OK);
		break;
	default:
		return (VALIDATE_ERROR);
		break;
	}
}

int
validate_fivr(struct finger_image_view_record *fivr)
{
	int ret = VALIDATE_OK;

	if (fivr->length < FIVR_HEADER_LENGTH) {
		ERRP("Record length is less than minimum");
		ret = VALIDATE_ERROR;
	}
	ret = validate_finger_palm_position(fivr->finger_palm_position);

	if ((fivr->count_of_views < FIR_MIN_VIEW_COUNT) ||
	    (fivr->count_of_views > FIR_MAX_VIEW_COUNT)) {
		ERRP("Count of views is invalid");
		ret = VALIDATE_ERROR;
	}
	if ((fivr->view_number < FIR_MIN_VIEW_COUNT) ||
	    (fivr->view_number > FIR_MAX_VIEW_COUNT)) {
		ERRP("View number is invalid");
		ret = VALIDATE_ERROR;
	}
	if (fivr->quality != UNDEFINED_IMAGE_QUALITY) {
		ERRP("Quality is invalid");
		ret = VALIDATE_ERROR;
	}
	ret = validate_impression_type(fivr->impression_type);
	if (fivr->reserved != 0) {
		ERRP("Reserved is not 0");
		ret = VALIDATE_ERROR;
	}
	return (ret);
}

void
copy_fivr(struct finger_image_view_record *src, 
    struct finger_image_view_record *dst)
{

	dst->length = src->length;
	dst->finger_palm_position = src->finger_palm_position;
	dst->count_of_views = src->count_of_views;
	dst->view_number = src->view_number;
	dst->quality = src->quality;
	dst->impression_type = src->impression_type;
	dst->horizontal_line_length = src->horizontal_line_length;
	dst->vertical_line_length = src->vertical_line_length;
	dst->reserved = src->reserved;
}
