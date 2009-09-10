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
/* This file implements the routines to validate ISO 19794-4 Finger Image     */
/* Records according to ISO/IEC 29109-4 conformance testing.                  */
/******************************************************************************/

int
validate_fir(struct finger_image_record *fir)
{
	int ret = VALIDATE_OK;
	int hdr_len;

	if (strncmp(fir->format_id, FIR_FORMAT_ID, FIR_FORMAT_ID_LEN) != 0) {
		ERRP("Header format ID is [%s], should be [%s]",
		    fir->format_id, FIR_FORMAT_ID);
		ret = VALIDATE_ERROR;
	}
	if (strncmp(fir->spec_version, FIR_SPEC_VERSION, FIR_SPEC_VERSION_LEN) != 0) {
		ERRP("Header spec version is [%s], should be [%s]",
		    fir->spec_version, FIR_SPEC_VERSION);
		ret = VALIDATE_ERROR;
	}
	if (fir->format_std == FIR_STD_ANSI)
		hdr_len = FIR_ANSI_HEADER_LENGTH;
	else
		hdr_len = FIR_ISO_HEADER_LENGTH;
	if (fir->record_length < hdr_len +
	    (fir->num_fingers_or_palm_images * FIVR_HEADER_LENGTH)) {
		ERRP("Record length is too short, minimum is %d", hdr_len);
		ret = VALIDATE_ERROR;
	}
	if (fir->format_std == FIR_STD_ANSI) {
		if (fir->product_identifier_owner == 0) {
			ERRP("Product ID Owner is zero");
			ret = VALIDATE_ERROR;
		}
	}
	if ((fir->image_acquisition_level != 10) &&
	    (fir->image_acquisition_level != 20) &&
	    (fir->image_acquisition_level != 30) &&
	    (fir->image_acquisition_level != 31) &&
	    (fir->image_acquisition_level != 40) &&
	    (fir->image_acquisition_level != 41)) {
		ERRP("Image acquisition level is invalid");
		ret = VALIDATE_ERROR;
	}
	if (fir->num_fingers_or_palm_images == 0) {
		ERRP("Number of fingers/palms is zero");
		ret = VALIDATE_ERROR;
	}
	if ((fir->scale_units != FIR_SCALE_UNITS_CM) &&
	    (fir->scale_units != FIR_SCALE_UNITS_INCH)) {
		ERRP("Scale units is invalid");
		ret = VALIDATE_ERROR;
	}
	if (fir->x_scan_resolution > FIR_MAX_SCAN_RESOLUTION) {
		ERRP("X scan resolution too large");
		ret = VALIDATE_ERROR;
	}
	if (fir->y_scan_resolution > FIR_MAX_SCAN_RESOLUTION) {
		ERRP("Y scan resolution too large");
		ret = VALIDATE_ERROR;
	}
	if (fir->x_image_resolution > fir->x_scan_resolution) {
		ERRP("X image resolution greater than X scan resolution");
		ret = VALIDATE_ERROR;
	}
	if (fir->y_image_resolution > fir->y_scan_resolution) {
		ERRP("Y image resolution greater than Y scan resolution");
		ret = VALIDATE_ERROR;
	}
	if ((fir->pixel_depth < FIR_MIN_PIXEL_DEPTH) ||
	    (fir->pixel_depth > FIR_MAX_PIXEL_DEPTH)) {
		ERRP("Pixel depth is invalid");
		ret = VALIDATE_ERROR;
	}
	if ((fir->image_compression_algorithm !=
		COMPRESSION_ALGORITHM_UNCOMPRESSED_NO_BIT_PACKED) &&
	    (fir->image_compression_algorithm !=
		COMPRESSION_ALGORITHM_UNCOMPRESSED_BIT_PACKED) &&
	    (fir->image_compression_algorithm !=
		COMPRESSION_ALGORITHM_COMPRESSED_WSQ) &&
	    (fir->image_compression_algorithm !=
		COMPRESSION_ALGORITHM_COMPRESSED_JPEG) &&
	    (fir->image_compression_algorithm !=
		COMPRESSION_ALGORITHM_COMPRESSED_JPEG2000) &&
	    (fir->image_compression_algorithm !=
		COMPRESSION_ALGORITHM_COMPRESSED_PNG)) {
		ERRP("Image compression algorithm is invalid");
		ret = VALIDATE_ERROR;
	}
	if (fir->reserved != 0) {
		ERRP("Reserved field is not zero");
		ret = VALIDATE_ERROR;
	}

	return (ret);
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
	int ret;
	int status = VALIDATE_OK;

	if (fivr->length < FIVR_HEADER_LENGTH) {
		ERRP("Record length is less than minimum");
		status = VALIDATE_ERROR;
	}
	ret = validate_finger_palm_position(fivr->finger_palm_position);
	if (ret != VALIDATE_OK)
		status = VALIDATE_ERROR;

	if ((fivr->count_of_views < FIR_MIN_VIEW_COUNT) ||
	    (fivr->count_of_views > FIR_MAX_VIEW_COUNT)) {
		ERRP("Count of views is invalid");
		status = VALIDATE_ERROR;
	}
	if ((fivr->view_number < FIR_MIN_VIEW_COUNT) ||
	    (fivr->view_number > FIR_MAX_VIEW_COUNT)) {
		ERRP("View number is invalid");
		status = VALIDATE_ERROR;
	}
	if (fivr->quality != UNDEFINED_IMAGE_QUALITY) {
		ERRP("Quality is invalid");
		status = VALIDATE_ERROR;
	}
	ret = validate_impression_type(fivr->impression_type);
	if (ret != VALIDATE_OK)
		status = VALIDATE_ERROR;
	
	if (fivr->reserved != 0) {
		ERRP("Reserved is not 0");
		status = VALIDATE_ERROR;
	}
	return (status);
}
