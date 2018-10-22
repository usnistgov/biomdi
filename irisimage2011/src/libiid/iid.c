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
#include <strings.h>

#include <biomdi.h>
#include <biomdimacro.h>
#include <iid.h>

int
encoded_date_check(uint8_t *date)
{
	int ret;

	ret = VALIDATE_OK;	/* May be changed by macros below */
	/* No check on the year */
	CRSR(date[2], 00, 12, "Month");
	CRSR(date[3], 00, 31, "Day");
	CRSR(date[4], 00, 23, "Hour");
	if ((date[5] > 59) && (date[5] != 99)) {
		ERRP("Minute value");
		ret = VALIDATE_ERROR;
	}
	if ((date[6] > 59) && (date[6] != 99)) {
		ERRP("Second");
		ret = VALIDATE_ERROR;
	}
	return (ret);
}

static void
encoded_date_to_str(uint8_t *src, char *dst)
{
	int ret;

	ret = encoded_date_check(src);
	if (ret == VALIDATE_ERROR) {
		snprintf(dst, IID_CAPTURE_DATE_STRING_LEN+1, "Undefined");
		return;
	}
	uint16_t year;
	BDB bdb;
	INIT_BDB(&bdb, src, IID_CAPTURE_DATE_STRING_LEN);
	SSCAN(&year, &bdb); 
	/* Print the date string, even if invalid */
	snprintf(dst, IID_CAPTURE_DATE_STRING_LEN+1,
	    "%04u/%02u/%02u %02u:%02u:%02u",
	    year, src[2], src[3], src[4], src[5], src[6]);
	return;
eof_out:
	return;
}

char *
iid_code_to_str(int category, int code)
{
	switch(category) {
	case IID_CODE_CATEGORY_NUM_EYES:
		switch(code) {
			case IID_NUM_EYES_UNKNOWN:
				return ("Unknown");
			case IID_NUM_EYES_LEFT_OR_RIGHT:
				return ("Left or Right Present");
			case IID_NUM_EYES_LEFT_AND_RIGHT:
				return ("Left and Right Present");
			default : return("Invalid code");
		}

	case IID_CODE_CATEGORY_ORIENTATION :
		switch(code) {
			case IID_ORIENTATION_UNDEF:
				return("Undefined");
			case IID_ORIENTATION_BASE:
				return("Base");
			case IID_ORIENTATION_FLIPPED:
				return("Flipped");
			default:
				return("Invalid code");
		}

	case IID_CODE_CATEGORY_IMAGE_FORMAT :
		switch(code) {
			case IID_IMAGEFORMAT_MONO_RAW :
			    return ("Mono Raw");
			case IID_IMAGEFORMAT_MONO_JPEG2000 :
			    return ("Mono JPEG 2000");
			case IID_IMAGEFORMAT_MONO_PNG :
			    return ("Mono PNG");
			default : return ("Invalid code");
		}
	case IID_CODE_CATEGORY_EYE_LABEL :
		switch(code) {
			case IID_SUBJECT_EYE_UNDEF : return ("Undefined");
			case IID_SUBJECT_EYE_RIGHT : return ("Right Eye");
			case IID_SUBJECT_EYE_LEFT : return ("Left Eye");
			default : return ("Invalid code");
		}
	case IID_CODE_CATEGORY_IMAGE_TYPE :
		switch(code) {
			case IID_TYPE_UNCROPPED :
			    return ("Uncropped rectlinear");
			case IID_TYPE_VGA :
			    return ("Rectlinear VGA");
			case IID_TYPE_CROPPED :
			    return ("Cropped and centered");
			case IID_TYPE_CROPPED_AND_MASKED :
			    return ("Cropped, ROI masked and centered");
			default : return ("Invalid code");
		}
	case IID_CODE_CATEGORY_COMPRESSION_HISTORY:
		switch(code) {
			case IID_PREV_COMPRESSION_UNDEF:
				return("Undefined");
			case IID_PREV_COMPRESSION_LOSSLESS_NONE:
				return("Lossless or None");
			case IID_PREV_COMPRESSION_LOSSY:
				return("Lossy");
		}
	default : return("Invalid category");
	}
}

/******************************************************************************/
/* Implement the interface for allocating and freeing iris image data blocks. */
/******************************************************************************/
int
new_irh(IRH **irh)
{
	IRH *lirh;

	lirh = (IRH *)malloc(sizeof(IRH));
	if (lirh == NULL)
		return (-1);
	memset((void *)lirh, 0, sizeof(IRH));
	*irh = lirh;
	return (0);
}

void
free_irh(IRH *irh)
{
	if (irh->image_data != NULL)
		free(irh->image_data);
	free(irh);
}

int
new_iibdb(IIBDB **iibdb)
{
	IIBDB *liibdb;

	liibdb = (IIBDB *)malloc(sizeof(IIBDB));
	if (liibdb == NULL)
		return (-1);
	memset((void *)liibdb, 0, sizeof(IIBDB));
	TAILQ_INIT(&liibdb->image_headers);
	*iibdb = liibdb;
	return (0);
}

void
free_iibdb(IIBDB *iibdb)
{
	IRH *irh;

	while (!TAILQ_EMPTY(&iibdb->image_headers)) {
		irh = TAILQ_FIRST(&iibdb->image_headers);
		TAILQ_REMOVE(&iibdb->image_headers, irh, list);
		free_irh(irh);
	}
	free(iibdb);
}

void
add_irh_to_iibdb(IRH *irh, IIBDB *iibdb)
{
	irh->iibdb = iibdb;
	TAILQ_INSERT_TAIL(&iibdb->image_headers, irh, list);
}

/******************************************************************************/
/* Implement the interface for reading/writing/verifying iris representation  */
/* headers and associated image data.                                         */
/******************************************************************************/
static int
internal_read_irh(FILE *fp, BDB *bdb, IRH *irh)
{
	uint8_t cval;
	int i;

	LGET(&irh->representation_length, fp, bdb);
	OGET(irh->capture_date, 1, IID_CAPTURE_DATE_LEN, fp, bdb);
	CGET(&irh->capture_device_tech_id, fp, bdb);
	SGET(&irh->capture_device_vendor_id, fp, bdb);
	SGET(&irh->capture_device_type_id, fp, bdb);
	CGET(&irh->num_quality_blocks, fp, bdb);
	for (i = 0; i < irh->num_quality_blocks; i++) {
		CGET(&irh->quality_block[i].score, fp, bdb);
		SGET(&irh->quality_block[i].algorithm_vendor_id, fp, bdb);
		SGET(&irh->quality_block[i].algorithm_id, fp, bdb);
	}
	SGET(&irh->representation_number, fp, bdb);
	CGET(&irh->eye_label, fp, bdb);
	CGET(&irh->image_type, fp, bdb);
	CGET(&irh->image_format, fp, bdb);

	/* Iris image properties bit field */
	CGET(&cval, fp, bdb);
	irh->horz_orientation = (cval & IID_HORZ_ORIENTATION_MASK)
	    >> IID_HORZ_ORIENTATION_SHIFT;
	irh->vert_orientation = (cval & IID_VERT_ORIENTATION_MASK)
	    >> IID_VERT_ORIENTATION_SHIFT;
	irh->compression_history =
	    (cval & IID_PREV_COMPRESSION_MASK) >> IID_PREV_COMPRESSION_SHIFT;

	SGET(&irh->image_width, fp, bdb);
	SGET(&irh->image_height, fp, bdb);
	CGET(&irh->bit_depth, fp, bdb);
	SGET(&irh->range, fp, bdb);
	SGET(&irh->roll_angle, fp, bdb);
	SGET(&irh->roll_angle_uncertainty, fp, bdb);

	SGET(&irh->iris_center_smallest_x, fp, bdb);
	SGET(&irh->iris_center_largest_x, fp, bdb);
	SGET(&irh->iris_center_smallest_y, fp, bdb);
	SGET(&irh->iris_center_largest_y, fp, bdb);
	SGET(&irh->iris_diameter_smallest, fp, bdb);
	SGET(&irh->iris_diameter_largest, fp, bdb);

	LGET(&irh->image_length, fp, bdb);
	if (irh->image_length != 0) {
		irh->image_data = (uint8_t *)malloc(irh->image_length);
		if (irh->image_data == NULL)
			ALLOC_ERR_OUT("Image data buffer");
		OGET(irh->image_data, 1, irh->image_length, fp, bdb);
	}
	return (READ_OK);
eof_out:
	return (READ_EOF);
err_out:
	return (READ_ERROR);
}

int
read_irh(FILE *fp, IRH *irh)
{
	return (internal_read_irh(fp, NULL, irh));
}

int
scan_irh(BDB *bdb, IRH *irh)
{
	return (internal_read_irh(NULL, bdb, irh));
}

static int
internal_read_iibdb(FILE *fp, BDB *bdb, IIBDB *iibdb)
{
	int i;
	int ret;
	IGH *hdr;
	IRH *irh;

	/* Read the Iris general header */
	hdr = &iibdb->general_header;
	OGET(hdr->format_id, 1, IID_FORMAT_ID_LEN, fp, bdb);
	OGET(hdr->format_version, 1, IID_FORMAT_VERSION_LEN, fp, bdb);
	LGET(&hdr->record_length, fp, bdb);
	SGET(&hdr->num_irises, fp, bdb);
	CGET(&hdr->cert_flag, fp, bdb);
	CGET(&hdr->num_eyes, fp, bdb);

	/* Read the iris representation headers and image data */
	for (i = 0; i < iibdb->general_header.num_irises; i++) {
		ret = new_irh(&irh);
		if (ret < 0)
			ALLOC_ERR_OUT("image header");
		irh->iibdb = iibdb;
		if (fp != NULL)
			ret = read_irh(fp, irh);
		else
			ret = scan_irh(bdb, irh);
		if (ret == READ_OK)
			add_irh_to_iibdb(irh, iibdb);
		else if (ret == READ_EOF)
			// XXX Handle a partial read?
			return (READ_EOF);
		else
			READ_ERR_OUT("Iris image header %d", i);
	}
	return (READ_OK);

eof_out:
	return (READ_EOF);
err_out:
	return (READ_ERROR);
}

int
read_iibdb(FILE *fp, IIBDB *iibdb)
{
	return (internal_read_iibdb(fp, NULL, iibdb));
}

int
scan_iibdb(BDB *bdb, IIBDB *iibdb)
{
	return (internal_read_iibdb(NULL, bdb, iibdb));
}

static int
internal_write_irh(FILE *fp, BDB *bdb, IRH *irh)
{
	int i;

	LPUT(irh->representation_length, fp, bdb);
	OPUT(irh->capture_date, 1, IID_CAPTURE_DATE_LEN, fp, bdb);
	CPUT(irh->capture_device_tech_id, fp, bdb);
	SPUT(irh->capture_device_vendor_id, fp, bdb);
	SPUT(irh->capture_device_type_id, fp, bdb);
	CPUT(irh->num_quality_blocks, fp, bdb);
	for (i = 0; i < irh->num_quality_blocks; i++) {
		CPUT(irh->quality_block[i].score, fp, bdb);
		SPUT(irh->quality_block[i].algorithm_vendor_id, fp, bdb);
		SPUT(irh->quality_block[i].algorithm_id, fp, bdb);
	}
	SPUT(irh->representation_number, fp, bdb);
	CPUT(irh->eye_label, fp, bdb);
	CPUT(irh->image_type, fp, bdb);
	CPUT(irh->image_format, fp, bdb);

	/* Iris image properties bit field */
	uint8_t cval = 0;
	cval = (irh->horz_orientation << IID_HORZ_ORIENTATION_SHIFT) |
	    (irh->vert_orientation << IID_VERT_ORIENTATION_SHIFT) |
	    (irh->compression_history << IID_PREV_COMPRESSION_SHIFT);
	CPUT(cval, fp, bdb);

	LPUT(irh->image_length, fp, bdb);

	SPUT(irh->image_width, fp, bdb);
	SPUT(irh->image_height, fp, bdb);
	CPUT(irh->bit_depth, fp, bdb);
	SPUT(irh->range, fp, bdb);
	SPUT(irh->roll_angle, fp, bdb);
	SPUT(irh->roll_angle_uncertainty, fp, bdb);

	SPUT(irh->iris_center_smallest_x, fp, bdb);
	SPUT(irh->iris_center_largest_x, fp, bdb);
	SPUT(irh->iris_center_smallest_y, fp, bdb);
	SPUT(irh->iris_center_largest_y, fp, bdb);
	SPUT(irh->iris_diameter_smallest, fp, bdb);
	SPUT(irh->iris_diameter_largest, fp, bdb);

	if (irh->image_data != NULL)
		OPUT(irh->image_data, 1, irh->image_length, fp, bdb);
	return (WRITE_OK);
err_out:
	return (WRITE_ERROR);
}

int
write_irh(FILE *fp, IRH *irh)
{
	return (internal_write_irh(fp, NULL, irh));
}

int
push_irh(BDB *bdb, IRH *irh)
{
	return (internal_write_irh(NULL, bdb, irh));
}

static int
internal_write_iibdb(FILE *fp, BDB *bdb, IIBDB *iibdb)
{
	int ret;
	IGH *hdr;
	IRH *irh;

	/* Write the Iris record header */
	hdr = &iibdb->general_header;

	OPUT(hdr->format_id, 1, IID_FORMAT_ID_LEN, fp, bdb);
	OPUT(hdr->format_version, 1, IID_FORMAT_VERSION_LEN, fp, bdb);
	LPUT(hdr->record_length, fp, bdb);
	SPUT(hdr->num_irises, fp, bdb);
	CPUT(hdr->cert_flag, fp, bdb);
	CPUT(hdr->num_eyes, fp, bdb);

	/* Write the image headers and data */
	TAILQ_FOREACH(irh, &iibdb->image_headers, list) {
		if (fp != NULL)
			ret = write_irh(fp, irh);
		else
			ret = push_irh(bdb, irh);
		if (ret != WRITE_OK)
			WRITE_ERR_OUT("Iris Representation Header");
	}

	return (WRITE_OK);
err_out:
	return (WRITE_ERROR);
}

int
write_iibdb(FILE *fp, IIBDB *iibdb)
{
	return (internal_write_iibdb(fp, NULL, iibdb));
}

int
push_iibdb(BDB *bdb, IIBDB *iibdb)
{
	return (internal_write_iibdb(NULL, bdb, iibdb));
}

int
print_irh(FILE *fp, IRH *irh)
{
	int i;
	char date[IID_CAPTURE_DATE_STRING_LEN + 1];

	encoded_date_to_str(irh->capture_date, date);
	FPRINTF(fp, "\tCapture Date\t\t\t: %s\n", date);

	FPRINTF(fp, "\tCapture Device Technology ID\t: ");
	switch (irh->capture_device_tech_id) {
		case IID_CAPTURE_DEVICE_TECHNOLOGY_UNSPEC:
 			FPRINTF(fp, "Unknown/Unspecified\n");
			break;
		case IID_CAPTURE_DEVICE_TECHNOLOGY_CMOSCCD:
 			FPRINTF(fp, "CMOS/CCD\n");
			break;
		default:
 			FPRINTF(fp, "Invalid: 0x%02hhX\n",
			    irh->capture_device_tech_id);
			break;
	}
	FPRINTF(fp, "\tCapture Device Vendor ID\t: ");
	if (irh->capture_device_vendor_id == IID_CAPTURE_DEVICE_UNSPEC)
 		FPRINTF(fp, "Unspecified\n");
	else
 		FPRINTF(fp, "0x%04hX\n", irh->capture_device_vendor_id);
	FPRINTF(fp, "\tCapture Device Type ID\t\t: ");
	if (irh->capture_device_type_id == IID_CAPTURE_DEVICE_UNSPEC)
 		FPRINTF(fp, "Unspecified\n");
	else
 		FPRINTF(fp, "0x%04hX\n", irh->capture_device_type_id);

	FPRINTF(fp, "\tNumber of Quality Blocks\t: %hhu\n",
	    irh->num_quality_blocks);
	if (irh->num_quality_blocks > 0) {
		FPRINTF(fp, "\t\tQuality Blocks:\n");
		FPRINTF(fp, "\t\t\tScore\t\tAlg Vendor\tAlg ID\n");
		for (i = 0; i < irh->num_quality_blocks; i++) {
			if (irh->quality_block[i].score ==
			    IID_IMAGE_QUAL_FAILED) {
				FPRINTF(fp, "\t\t\tFailed    ");
			} else {
				FPRINTF(fp, "\t\t\t%-10hhu",
				    irh->quality_block[i].score);
			}
			if (irh->quality_block[i].algorithm_vendor_id == 0)
 				FPRINTF(fp, "\tUnreported");
			else
 				FPRINTF(fp, "\t0x%04hX",
				    irh->quality_block[i].algorithm_vendor_id);

			if (irh->quality_block[i].algorithm_id == 0)
 				FPRINTF(fp, "\tUnreported");
			else
 				FPRINTF(fp, "\t\t0x%04hX",
				    irh->quality_block[i].algorithm_id);

			FPRINTF(fp, "\n");
		}
	}
	FPRINTF(fp, "\tRepresentation Number\t\t: %hu\n",
	    irh->representation_number);
	FPRINTF(fp, "\tEye Label\t\t\t: %hhu (%s)\n", irh->eye_label,
	    iid_code_to_str(IID_CODE_CATEGORY_EYE_LABEL, irh->eye_label));
	FPRINTF(fp, "\tImage Type\t\t\t: %hhu (%s)\n", irh->image_type,
	    iid_code_to_str(IID_CODE_CATEGORY_IMAGE_TYPE, irh->image_type));
	FPRINTF(fp, "\tImage Format\t\t\t: %hhu (%s)\n", irh->image_format,
	    iid_code_to_str(IID_CODE_CATEGORY_IMAGE_FORMAT, irh->image_format));

	FPRINTF(fp, "\tHorizontal Orientation\t\t: %hhu (%s)\n",
	    irh->horz_orientation,
	    iid_code_to_str(IID_CODE_CATEGORY_ORIENTATION,
		irh->horz_orientation));
	FPRINTF(fp, "\tVertical Orientation\t\t: %hhu (%s)\n",
	    irh->vert_orientation,
	    iid_code_to_str(IID_CODE_CATEGORY_ORIENTATION,
		irh->vert_orientation));
	FPRINTF(fp, "\tCompression History\t\t: %hhu (%s)\n",
	    irh->compression_history,
	    iid_code_to_str(IID_CODE_CATEGORY_COMPRESSION_HISTORY,
	     irh->compression_history));

	FPRINTF(fp, "\tImage Size\t\t\t: ");
	FPRINTF(fp, "%hu X ", irh->image_width);
	FPRINTF(fp, "%hu\n", irh->image_height);
	FPRINTF(fp, "\tIntensity Depth\t\t\t: ");
	FPRINTF(fp, "%hhu\n", irh->bit_depth);

	FPRINTF(fp, "\tRange\t\t\t\t: ");
	switch (irh->range) {
		case IID_RANGE_UNASSIGNED:
			FPRINTF(fp, "Unassigned\n");
			break;
		case IID_RANGE_FAILED:
			FPRINTF(fp, "Failed\n");
			break;
		case IID_RANGE_OVERFLOW:
			FPRINTF(fp, "Overflow\n");
			break;
		default:
			FPRINTF(fp, "%hu\n", irh->range);
			break;
	}

	FPRINTF(fp, "\tRoll Angle\t\t\t: ");
	if (irh->roll_angle == IID_ROLL_ANGLE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->roll_angle);
	FPRINTF(fp, "\tRotation Uncertaintity\t\t: ");
	if (irh->roll_angle_uncertainty == IID_ROLL_ANGLE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->roll_angle_uncertainty);

	FPRINTF(fp, "\tIris center, smallest X\t\t: ");
	if (irh->iris_center_smallest_x == IID_COORDINATE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->iris_center_smallest_x);

	FPRINTF(fp, "\tIris center, largest X\t\t: ");
	if (irh->iris_center_largest_x == IID_COORDINATE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->iris_center_largest_x);

	FPRINTF(fp, "\tIris center, smallest Y\t\t: ");
	if (irh->iris_center_smallest_y == IID_COORDINATE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->iris_center_smallest_y);

	FPRINTF(fp, "\tIris center, largest Y\t\t: ");
	if (irh->iris_center_largest_y == IID_COORDINATE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->iris_center_largest_y);

	FPRINTF(fp, "\tIris diameter smallest\t\t: ");
	if (irh->iris_diameter_smallest == IID_COORDINATE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->iris_diameter_smallest);
	FPRINTF(fp, "\tIris Diameter Highest\t\t: ");
	if (irh->iris_diameter_largest == IID_COORDINATE_UNDEF)
		FPRINTF(fp, "Undefined\n");
	else
		FPRINTF(fp, "%hu\n", irh->iris_diameter_largest);

	FPRINTF(fp, "\tImage Length\t\t\t: %u\n", irh->image_length);


// XXX If an option flag is passed, save the image data to a file
	return (PRINT_OK);
err_out:
	return (PRINT_ERROR);
}

int
print_iibdb(FILE *fp, IIBDB *iibdb)
{
	int ret;
	int i;
	IGH *hdr;
	IRH *irh;

	hdr = &iibdb->general_header;
	FPRINTF(fp, "Format ID\t\t\t: %s\nSpecification Version\t\t: %s\n",
	    hdr->format_id, hdr->format_version);
	FPRINTF(fp, "Record Length\t\t\t: %u\n",
	    hdr->record_length);
	FPRINTF(fp, "Number of Irises Represented\t: %d\n", hdr->num_irises);
	FPRINTF(fp, "Certification flag\t\t: 0x%02X\n", hdr->cert_flag);
	FPRINTF(fp, "Number of Eyes Represented\t: %d (%s)\n",
	    hdr->num_eyes,
	    iid_code_to_str(IID_CODE_CATEGORY_NUM_EYES, hdr->num_eyes));

	/* Print the image headers */
	i = 1;
	TAILQ_FOREACH(irh, &iibdb->image_headers, list) {
		FPRINTF(fp, "Iris Representation Header %d:\n", i);
		ret = print_irh(fp, irh);
		if (ret != PRINT_OK)
			ERR_OUT("Could not print header %d", i);
		i++;
	}
	FPRINTF(fp, "\n");
	return (PRINT_OK);
err_out:
	return (PRINT_ERROR);
}

/******************************************************************************/
/* Implementation of the higher level access routines.                        */
/******************************************************************************/
int
get_irh_count(IIBDB *iibdb)
{
	return (iibdb->general_header.num_irises);
}

int
get_irhs(IIBDB *iibdb, IRH *irhs[])
{
	int count = 0;

	IRH *irh;

	TAILQ_FOREACH(irh, &iibdb->image_headers, list) {
		irhs[count] = irh;
		count++;
	}
	return (count);
}

int
clone_iibdb(IIBDB *src, IIBDB **dst, int cloneimg)
{
	IIBDB *liibdb;
	IRH *srcirh, *dstirh;
	int ret;

	liibdb = NULL;
	dstirh = NULL;
	
	ret = new_iibdb(&liibdb);
	if (ret != 0)
		ALLOC_ERR_RETURN("Cloned IIBDB");

	liibdb->general_header = src->general_header;
	TAILQ_FOREACH(srcirh, &src->image_headers, list) {
		ret = new_irh(&dstirh);
		if (ret != 0)
			ALLOC_ERR_OUT("Cloned Iris Representation Header");
		COPY_IRH(srcirh, dstirh);
		dstirh->iibdb = srcirh->iibdb;
		add_irh_to_iibdb(dstirh, liibdb);
		if (cloneimg) {
			dstirh->image_data =
			    (uint8_t *)malloc(srcirh->image_length);
			if (dstirh->image_data == NULL)
				ALLOC_ERR_OUT("Cloned image data");
			bcopy(srcirh->image_data, dstirh->image_data,
			    srcirh->image_length);
		} else {
			dstirh->image_data = NULL;
			dstirh->image_length = 0;
		}
	}
	*dst = liibdb;
	return (0);
err_out:
	if (liibdb != NULL)
		free_iibdb(liibdb);
	return (-1);
}
