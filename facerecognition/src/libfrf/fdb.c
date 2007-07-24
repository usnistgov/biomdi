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
/*                                                                            */
/* Routines that read, write, and validate a Facial Data Block that complies  */
/* with Face Recognition Format for Data Interchange (ANSI/INCITS 385-2004)   */
/* record format.                                                             */
/*                                                                            */
/******************************************************************************/
#include <sys/queue.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <frf.h>
#include <biomdi.h>
#include <biomdimacro.h>

int
new_fdb(struct facial_data_block **fdb)
{
	struct facial_data_block *lfdb;
                
	lfdb = (struct facial_data_block *)malloc(
			sizeof(struct facial_data_block));
	if (lfdb == NULL) {
		perror("Failed to allocate Facial Block");
		return -1;
	}       
	memset((void *)lfdb, 0, sizeof(struct facial_data_block));
	TAILQ_INIT(&lfdb->feature_points);

	*fdb = lfdb;

	return 0;
}

void
free_fdb(struct facial_data_block *fdb)
{
	struct feature_point_block *fpb;

	while (!TAILQ_EMPTY(&fdb->feature_points)) {
		fpb = TAILQ_FIRST(&fdb->feature_points);
		TAILQ_REMOVE(&fdb->feature_points, fpb, list);
		free_fpb(fpb);
	}
	free(fdb);
}

int
read_fdb(FILE *fp, struct facial_data_block *fdb)
{
	unsigned int i;
	int ret;
	struct feature_point_block *fpb;
	unsigned int lval;
	long long llval;

	// Read the Facial Information Block first
	// Block Length
	LREAD(&fdb->block_length, fp);

	// Number of Feature Points
	SREAD(&fdb->num_feature_points, fp);

	// Gender
	CREAD(&fdb->gender, fp);

	// Eye Color
	CREAD(&fdb->eye_color, fp);

	// Hair Color
	CREAD(&fdb->hair_color, fp);

	// Feature Mask; need to shift the bits around because the length
	// is less than what will fit in a long integer
	lval = 0;
	OREAD(&lval, 1, FEATURE_MASK_LEN, fp);
	lval = ntohl(lval);
	fdb->feature_mask = lval >> 8;

	// Expression
	SREAD(&fdb->expression, fp);

	// Pose Angles
	CREAD(&fdb->pose_angle_yaw, fp);
	CREAD(&fdb->pose_angle_pitch, fp);
	CREAD(&fdb->pose_angle_roll, fp);

	// Pose Angle Uncertainties
	CREAD(&fdb->pose_angle_uncertainty_yaw, fp);
	CREAD(&fdb->pose_angle_uncertainty_pitch, fp);
	CREAD(&fdb->pose_angle_uncertainty_roll, fp);

	// Read the Feature Points(s)
	for (i = 1; i <= fdb->num_feature_points; i++) {
		if (new_fpb(&fpb) < 0) {
			fprintf(stderr, "error allocating FPB %u\n", i);
			goto err_out;
		}
		ret = read_fpb(fp, fpb);
		if (ret == READ_OK)
			add_fpb_to_fdb(fpb, fdb);
		else if (ret == READ_EOF)
			goto eof_out;
		else
			goto err_out;

	}

	// Read the Image Information
	// Face Image Type
	CREAD(&fdb->face_image_type, fp);

	// Image Data Type
	CREAD(&fdb->image_data_type, fp);

	// Width
	SREAD(&fdb->width, fp);

	// Height
	SREAD(&fdb->height, fp);

	// Image Color Space
	CREAD(&fdb->image_color_space, fp);

	// Source Type
	CREAD(&fdb->source_type, fp);

	// Device Type
	SREAD(&fdb->device_type, fp);

	// Quality
	SREAD(&fdb->quality, fp);

	// Read the optional Image Data; the length of the image data is 
	// equal to the Facial Image Block Length minus the sum of the
	// Facial Information Block, Feature Points, and Image Information.

	llval = fdb->block_length - FRF_FIB_LENGTH - 
		    (fdb->num_feature_points * FRF_FPB_LENGTH) - FRF_IIB_LENGTH;

	if (llval < 0)
		ERR_OUT("Block length too short to account for image");

	lval = (unsigned int)llval;
	fdb->image_data = malloc(lval);
	if (fdb->image_data == NULL)
		ERR_OUT("Allocating image data\n");

	OREAD(fdb->image_data, 1, lval, fp);
	fdb->image_len = lval;

        return READ_OK;

eof_out:
        return READ_EOF;

err_out:
        return READ_ERROR;
}

int
write_fdb(FILE *fp, struct facial_data_block *fdb)
{
	int ret;
	struct feature_point_block *fpb;
	unsigned int lval;

	LWRITE(&fdb->block_length, fp);
	SWRITE(&fdb->num_feature_points, fp);
	CWRITE(&fdb->gender, fp);
	CWRITE(&fdb->eye_color, fp);
	CWRITE(&fdb->hair_color, fp);

	// Convert the feature mask, stored as a value longer than
	// what is needed.
	lval = fdb->feature_mask << 8;
	lval = htonl(lval);
	OWRITE(&lval, 1, FEATURE_MASK_LEN, fp);

	SWRITE(&fdb->expression, fp);

	CWRITE(&fdb->pose_angle_yaw, fp);
	CWRITE(&fdb->pose_angle_pitch, fp);
	CWRITE(&fdb->pose_angle_roll, fp);
	CWRITE(&fdb->pose_angle_uncertainty_yaw, fp);
	CWRITE(&fdb->pose_angle_uncertainty_pitch, fp);
	CWRITE(&fdb->pose_angle_uncertainty_roll, fp);

	// Write the Feature Point Blocks
	TAILQ_FOREACH(fpb, &fdb->feature_points, list) {
		ret = write_fpb(fp, fpb);
		if (ret != WRITE_OK)
			goto err_out;
	}

	// Write the Image Information block
	CWRITE(&fdb->face_image_type, fp);
	CWRITE(&fdb->image_data_type, fp);
	SWRITE(&fdb->width, fp);
	SWRITE(&fdb->height, fp);
	CWRITE(&fdb->image_color_space, fp);
	CWRITE(&fdb->source_type, fp);
	SWRITE(&fdb->device_type, fp);
	SWRITE(&fdb->quality, fp);

	// Write the image data
	if (fdb->image_data != NULL)
		OWRITE(fdb->image_data, 1, fdb->image_len, fp);

        return WRITE_OK;

err_out:
        return WRITE_ERROR;
}

int
print_fdb(FILE *fp, struct facial_data_block *fdb)
{
	struct feature_point_block *fpb;
	int ret = PRINT_OK;
	int error;

	FPRINTF(fp, "Facial Data Block\n");
	FPRINTF(fp, "\tBlock Length is %u\n", fdb->block_length);
	FPRINTF(fp, "\tNumber of Feature Points = %u\n", 
			fdb->num_feature_points);
	FPRINTF(fp, "\tGender is %u, Eye Color is %u, Hair Color is %u\n",
			fdb->gender, fdb->eye_color, fdb->hair_color);
	FPRINTF(fp, "\tFeature Mask is 0x%06x\n", fdb->feature_mask);
	FPRINTF(fp, "\tExpression is 0x%02x\n", fdb->expression);
	FPRINTF(fp, "\tPose Angles: Yaw = %u, Pitch = %u, Roll = %u\n",
			fdb->pose_angle_yaw, fdb->pose_angle_pitch,
			fdb->pose_angle_roll);
	FPRINTF(fp, "\tPose Angle Uncertainties: "
			"Yaw = %u, Pitch = %u, Roll = %u\n",
			fdb->pose_angle_uncertainty_yaw, 
			fdb->pose_angle_uncertainty_pitch,
			fdb->pose_angle_uncertainty_roll);

	// Print out the Feature Point Blocks
	TAILQ_FOREACH(fpb, &fdb->feature_points, list) {
		error = print_fpb(fp, fpb);
		if (error != PRINT_OK)
			ret = PRINT_ERROR;
	}

	// Print out the Image Information Block
	FPRINTF(fp, "Image Information Block\n");
	FPRINTF(fp, "\tFace Image: Type is %u, Image Data Type is %u\n",
			fdb->face_image_type, fdb->image_data_type);
	FPRINTF(fp, "\tSize is %ux%u\n", fdb->width, fdb->height);

	FPRINTF(fp, "\tColor Space is %u, Source Type is %u, "
			"Device Type is %u\n",
			fdb->image_color_space, fdb->source_type,
			fdb->device_type);
	FPRINTF(fp, "\tQuality is %u\n", fdb->quality);
	FPRINTF(fp, "\tImage length is %u\n", fdb->image_len);

        return ret;

err_out:
        return PRINT_ERROR;
}

int
validate_fdb(struct facial_data_block *fdb)
{
	int ret = VALIDATE_OK;
	int error;
	struct feature_point_block *fpb;

	// Gender
	if ((fdb->gender != GENDER_UNSPECIFIED) &&
	    (fdb->gender != GENDER_MALE) &&
	    (fdb->gender != GENDER_FEMALE) &&
	    (fdb->gender != GENDER_UNKNOWN)) {
		fprintf(stderr, "Gender is invalid.\n");
                ret = VALIDATE_ERROR;
	}

	// Eye color
	if ((fdb->eye_color != EYE_COLOR_UNSPECIFIED) &&
	    (fdb->eye_color != EYE_COLOR_BLUE) &&
	    (fdb->eye_color != EYE_COLOR_BROWN) &&
	    (fdb->eye_color != EYE_COLOR_GREEN) &&
	    (fdb->eye_color != EYE_COLOR_HAZEL) &&
	    (fdb->eye_color != EYE_COLOR_MAROON) &&
	    (fdb->eye_color != EYE_COLOR_MULTI) &&
	    (fdb->eye_color != EYE_COLOR_PINK) &&
	    (fdb->eye_color != EYE_COLOR_UNKNOWN)) {
		fprintf(stderr, "Eye color is invalid.\n");
                ret = VALIDATE_ERROR;
	}

	// Hair color
	if ((fdb->hair_color != HAIR_COLOR_UNSPECIFIED) &&
	    (fdb->hair_color != HAIR_COLOR_BALD) &&
	    (fdb->hair_color != HAIR_COLOR_BLACK) &&
	    (fdb->hair_color != HAIR_COLOR_BLONDE) &&
	    (fdb->hair_color != HAIR_COLOR_BROWN) &&
	    (fdb->hair_color != HAIR_COLOR_GRAY) &&
	    (fdb->hair_color != HAIR_COLOR_RED) &&
	    (fdb->hair_color != HAIR_COLOR_BLUE) &&
	    (fdb->hair_color != HAIR_COLOR_GREEN) &&
	    (fdb->hair_color != HAIR_COLOR_ORANGE) &&
	    (fdb->hair_color != HAIR_COLOR_PINK) &&
	    (fdb->hair_color != HAIR_COLOR_SANDY) &&
	    (fdb->hair_color != HAIR_COLOR_AUBURN) &&
	    (fdb->hair_color != HAIR_COLOR_WHITE) &&
	    (fdb->hair_color != HAIR_COLOR_STRAWBERRY) &&
	    (fdb->hair_color != HAIR_COLOR_UNKNOWN)) {
		fprintf(stderr, "Hair color is invalid.\n");
                ret = VALIDATE_ERROR;
	}

	// Feature Mask
	// This check is commented out for now because it is debatable
	// whether using the reserved bits violates conformance. So
	// we ignore them for now.
	//if (fdb->feature_mask & FEATURE_MASK_RESERVED) {
	//	fprintf(stderr, "Feature Mask is using reserved flags.\n");
	//	ret = VALIDATE_ERROR;
	//}

	// Expression
	if ((fdb->expression >= EXPRESSION_RESERVED_LOW) && 
	    (fdb->expression <= EXPRESSION_RESERVED_HIGH)) {
		fprintf(stderr, "Expresssion is in reserved range.\n");
                ret = VALIDATE_ERROR;
	}

	// Pose Angles
	if ((fdb->pose_angle_yaw != POSE_ANGLE_UNSPECIFIED) &&
	    ((fdb->pose_angle_yaw < POSE_ANGLE_MIN) ||
	    (fdb->pose_angle_yaw > POSE_ANGLE_MAX))) {
		fprintf(stderr, "Pose Angle Yaw is invalid.\n");
                ret = VALIDATE_ERROR;
	}
	if ((fdb->pose_angle_pitch != POSE_ANGLE_UNSPECIFIED) &&
	    ((fdb->pose_angle_pitch < POSE_ANGLE_MIN) ||
	    (fdb->pose_angle_pitch > POSE_ANGLE_MAX))) {
		fprintf(stderr, "Pose Angle Pitch is invalid.\n");
                ret = VALIDATE_ERROR;
	}
	if ((fdb->pose_angle_roll != POSE_ANGLE_UNSPECIFIED) &&
	    ((fdb->pose_angle_roll < POSE_ANGLE_MIN) ||
	    (fdb->pose_angle_roll > POSE_ANGLE_MAX))) {
		fprintf(stderr, "Pose Angle Roll is invalid.\n");
                ret = VALIDATE_ERROR;
	}

	// Pose Angle Uncertainties
	if ((fdb->pose_angle_uncertainty_yaw != 
		POSE_ANGLE_UNCERTAINTY_UNSPECIFIED) &&
	    ((fdb->pose_angle_uncertainty_yaw < POSE_ANGLE_UNCERTAINTY_MIN) ||
	    (fdb->pose_angle_uncertainty_yaw > POSE_ANGLE_UNCERTAINTY_MAX))) {
		fprintf(stderr, "Pose Angle Uncertainty Yaw is invalid.\n");
                ret = VALIDATE_ERROR;
	}
	if ((fdb->pose_angle_uncertainty_pitch != 
		POSE_ANGLE_UNCERTAINTY_UNSPECIFIED) &&
	    ((fdb->pose_angle_uncertainty_pitch < POSE_ANGLE_UNCERTAINTY_MIN) ||
	    (fdb->pose_angle_uncertainty_pitch > POSE_ANGLE_UNCERTAINTY_MAX))) {
		fprintf(stderr, "Pose Angle Uncertainty Pitch is invalid.\n");
                ret = VALIDATE_ERROR;
	}
	if ((fdb->pose_angle_uncertainty_roll != 
		POSE_ANGLE_UNCERTAINTY_UNSPECIFIED) &&
	    ((fdb->pose_angle_uncertainty_roll < POSE_ANGLE_UNCERTAINTY_MIN) ||
	    (fdb->pose_angle_uncertainty_roll > POSE_ANGLE_UNCERTAINTY_MAX))) {
		fprintf(stderr, "Pose Angle Uncertainty Roll is invalid.\n");
                ret = VALIDATE_ERROR;
	}

	// Validate the Feature Point Blocks
	TAILQ_FOREACH(fpb, &fdb->feature_points, list) {
		error = validate_fpb(fpb);
		if (error != VALIDATE_OK)
			ret = VALIDATE_ERROR;
	}

	// Image Information Block
	// Facial Image Type
	if ((fdb->face_image_type != FACE_IMAGE_TYPE_BASIC) &&
	    (fdb->face_image_type != FACE_IMAGE_TYPE_FULL_FRONTAL) &&
	    (fdb->face_image_type != FACE_IMAGE_TYPE_TOKEN_FRONTAL) &&
	    (fdb->face_image_type != FACE_IMAGE_TYPE_OTHER)) {
		fprintf(stderr, "Image Type is invalid.\n");
		ret = VALIDATE_ERROR;
	}

	// Image Data Type
	if ((fdb->image_data_type != IMAGE_DATA_JPEG) &&
	    (fdb->image_data_type != IMAGE_DATA_JPEG2000))
	{
		fprintf(stderr, "Image Data Type is invalid.\n");
		ret = VALIDATE_ERROR;
	}

	// Image Color Space
	if ((fdb->image_color_space >= COLOR_SPACE_TYPE_RESERVED_MIN) &&
	    (fdb->image_color_space <= COLOR_SPACE_TYPE_RESERVED_MAX)) {
		fprintf(stderr, "Image Color Space is in reserved range.\n");
		ret = VALIDATE_ERROR;
	}

	// Source Type
	if ((fdb->source_type >= SOURCE_TYPE_RESERVED_MIN) &&
	    (fdb->source_type <= SOURCE_TYPE_RESERVED_MAX)) {
		fprintf(stderr, "Image Source Type is in reserved range.\n");
		ret = VALIDATE_ERROR;
	}

	// There is no validation for Device Type

	// Quality
	if (fdb->quality != FRF_IMAGE_QUALITY_UNSPECIFIED) {
		fprintf(stderr, "Image Quality is invalid.\n");
		ret = VALIDATE_ERROR;
	}

	// XXX Verify the Image data?
	
        return ret;

}

void
add_fpb_to_fdb(struct feature_point_block *fpb, struct facial_data_block *fdb)
{
        fpb->fdb = fdb;
        TAILQ_INSERT_TAIL(&fdb->feature_points, fpb, list);
}

int
add_image_to_fdb(char *filename, struct facial_data_block *fdb)
{
	struct stat sb;
	FILE *fp;

	if (stat(filename, &sb) != 0)
		ERR_OUT("File '%s' not accessible.\n", filename);

        if ((fp = fopen(filename, "rb")) == NULL) 
                ERR_OUT("Could not open '%s'", filename);

	fdb->image_data = malloc(sb.st_size);
	if (fdb->image_data == NULL)
		ERR_OUT("Allocating image data\n");

	OREAD(fdb->image_data, 1, sb.st_size, fp);

	fdb->image_len = sb.st_size;
	fdb->block_length += sb.st_size;

        return READ_OK;

eof_out:
        return READ_EOF;

err_out:
        return READ_ERROR;
}
