/*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
*/

#include <sys/queue.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <biomdi.h>
#include <biomdimacro.h>
#include <iid.h>

/******************************************************************************/
/* This file implements the routines to validate ISO 19794-6 Iris Image       */
/* Records according to ISO/IEC 29109-6 conformance testing.                  */
/******************************************************************************/

static biomdiIntSet capture_device_tech_id = {
	.is_size   = 2,
	.is_values = {
	    IID_CAPTURE_DEVICE_TECHNOLOGY_UNSPEC,
	    IID_CAPTURE_DEVICE_TECHNOLOGY_CMOSCCD
	}
};
static biomdiIntSet eye_labels = {
	.is_size   = 3,
	.is_values = {
	    IID_SUBJECT_EYE_UNDEF,
	    IID_SUBJECT_EYE_RIGHT,
	    IID_SUBJECT_EYE_LEFT
	}
};
static biomdiIntSet type_of_imagery = {
	.is_size   = 4,
	.is_values = {
	    IID_TYPE_UNCROPPED,
	    IID_TYPE_VGA,
	    IID_TYPE_CROPPED,
	    IID_TYPE_CROPPED_AND_MASKED
	}
};
static biomdiIntSet image_formats = {
	.is_size   = 3,
	.is_values = {
	    IID_IMAGEFORMAT_MONO_RAW,
	    IID_IMAGEFORMAT_MONO_JPEG2000,
	    IID_IMAGEFORMAT_MONO_PNG
	}
};
static biomdiIntSet horz_orientations = {
	.is_size   = 3,
	.is_values = {
	    IID_ORIENTATION_UNDEF,
	    IID_ORIENTATION_BASE,
	    IID_ORIENTATION_FLIPPED
	}
};
static biomdiIntSet vert_orientations = {
	.is_size   = 3,
	.is_values = {
	    IID_ORIENTATION_UNDEF,
	    IID_ORIENTATION_BASE,
	    IID_ORIENTATION_FLIPPED
	}
};
static biomdiIntSet compression_history = {
	.is_size   = 3,
	.is_values = {
	    IID_PREV_COMPRESSION_UNDEF,
	    IID_PREV_COMPRESSION_LOSSLESS_NONE,
	    IID_PREV_COMPRESSION_LOSSY
	}
};

int
validate_irh(IRH *irh)
{
	int ret = VALIDATE_OK;
	int i;

	if (encoded_date_check(irh->capture_date) == VALIDATE_ERROR) {
		ERRP("Capture Date invalid");
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(capture_device_tech_id, irh->capture_device_tech_id)) {
		ERRP("Capture device technology ID 0x%02hhX invalid",
		    irh->capture_device_tech_id);
		ret = VALIDATE_ERROR;
	}
	for (i = 0; i < irh->num_quality_blocks; i++) {
		switch (irh->quality_block[i].score) {
		case IID_IMAGE_QUAL_FAILED:
			break;
		default:
			CRSR(irh->quality_block[i].score,
			    IID_IMAGE_QUAL_MIN_SCORE,
			    IID_IMAGE_QUAL_MAX_SCORE, "Quality score");
			break;
		}
	}
	if (irh->representation_number == 0) {
		ERRP("Representation number is 0");
		ret = VALIDATE_ERROR;
	}
	if (irh->representation_number >
	    irh->iibdb->general_header.num_irises) {
		ERRP("Representation number %hu greater greater than "
		    "total of %hu",
		    irh->representation_number,
		    irh->iibdb->general_header.num_irises);
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(eye_labels, irh->eye_label)) {
		ERRP("Eye Label 0x%02hhX invalid", irh->eye_label);
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(type_of_imagery, irh->image_type)) {
		ERRP("Kind 0x%02hhX invalid", irh->image_type);
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(image_formats, irh->image_format)) {
		ERRP("Image format 0x%02hhX invalid", irh->image_format);
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(horz_orientations, irh->horz_orientation)) {
		ERRP("Horizontal orientation 0x%02hhX invalid",
		    irh->horz_orientation);
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(vert_orientations, irh->vert_orientation)) {
		ERRP("Vertical orientation 0x%02hhX invalid",
		    irh->vert_orientation);
		ret = VALIDATE_ERROR;
	}
	if (irh->image_width == 0) {
		ERRP("Image width is 0");
		ret = VALIDATE_ERROR;
	}
	if (irh->image_height == 0) {
		ERRP("Image height is 0");
		ret = VALIDATE_ERROR;
	}
	if (irh->bit_depth < IID_IMAGE_BIT_DEPTH_MIN) {
		ERRP("Image bit depth is less than %u",
		    IID_IMAGE_BIT_DEPTH_MIN);
		ret = VALIDATE_ERROR;
	}
	if (!inIntSet(compression_history, irh->compression_history)) {
		ERRP("Compression history 0x%02hhX invalid",
		    irh->compression_history);
		ret = VALIDATE_ERROR;
	}
	if (irh->roll_angle != IID_ROLL_ANGLE_UNDEF)
		CRSR(irh->roll_angle, IID_ROLL_ANGLE_MIN,
		    IID_ROLL_ANGLE_MAX, "Roll angle");
	if (irh->roll_angle_uncertainty != IID_ROLL_ANGLE_UNDEF)
		CRSR(irh->roll_angle_uncertainty,
		    IID_ROLL_ANGLE_UNCERTAINTY_MIN,
		    IID_ROLL_ANGLE_UNCERTAINTY_MAX, "Roll angle uncertainty");

	return (ret);
}

int
validate_iibdb(IIBDB *iibdb)
{
	int ret = VALIDATE_OK;
	int i;
	IGH rh = iibdb->general_header;
	IRH *irh;
	int error;

	if (rh.format_id[IID_FORMAT_ID_LEN - 1] != 0) {
		ERRP("Header format ID is not NULL-terminated.");
		ret = VALIDATE_ERROR;
	} else {
		if (strncmp(rh.format_id, IID_FORMAT_ID,
		    IID_FORMAT_ID_LEN) != 0) {
			ERRP("Header format ID is [%s], should be [%s]",
			    rh.format_id, IID_FORMAT_ID);
			ret = VALIDATE_ERROR;
		}
	}
	if (rh.format_version[IID_FORMAT_VERSION_LEN - 1] != '\0') {
		ERRP("Header format version is not NULL-terminated.");
		ret = VALIDATE_ERROR;
	}
	for (i = 0; i < IID_FORMAT_VERSION_LEN - 1; i++) {
		if (!(isdigit(rh.format_version[i]))) {
			ERRP("Header format version is non-numeric.");
			ret = VALIDATE_ERROR;
			break;
		}
	}
	if (strcmp(rh.format_version, IID_ISO_FORMAT_VERSION) != 0) {
		ERRP("Header format version is not %s.",
		    IID_ISO_FORMAT_VERSION);
		ret = VALIDATE_ERROR;
	}

	CRSR(rh.num_irises, IID_MIN_IRISES, IID_MAX_IRISES, "Number of Irises");
	CRSR(rh.num_eyes, IID_MIN_EYES, IID_MAX_EYES, "Number of Eyes");
	
	/* Validate the image headers */
	TAILQ_FOREACH(irh, &iibdb->image_headers, list) {
		error = validate_irh(irh);
		if (error != VALIDATE_OK)
			ret = VALIDATE_ERROR;
	}

	return (ret);
}
