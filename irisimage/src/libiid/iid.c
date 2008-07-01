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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <biomdi.h>
#include <biomdimacro.h>
#include <iid.h>

char *
iid_code_to_str(int class, int code)
{
	switch(class) {
	case IID_CODE_CLASS_ORIENTATION :
		switch(code) {
			case IID_ORIENTATION_UNDEF : return("Undefined");
			case IID_ORIENTATION_BASE : return("Base");
			case IID_ORIENTATION_FLIPPED : return("Flipped");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_SCAN_TYPE :
		switch(code) {
			case IID_SCAN_TYPE_CORRECTED : return("Corrected");
			case IID_SCAN_TYPE_PROGRESSIVE : return("Progressive");
			case IID_SCAN_TYPE_INTERLACE_FRAME :
				return("Interlace Frame");
			case IID_SCAN_TYPE_INTERLACE_FIELD :
				return("Interlace Field");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_OCCLUSION :
		switch(code) {
			case IID_IROCC_UNDEF : return("Undefined");
			case IID_IROCC_PROCESSED : return("Processed");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_OCCLUSION_FILLING :
		switch(code) {
			case IID_IROCC_ZEROFILL : return("Zero Fill");
			case IID_IROCC_UNITFILL : return("Unit Fill");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_BOUNDARY_EXTRACTION :
		switch(code) {
			case IID_IRBNDY_UNDEF : return("Undefined");
			case IID_IRBNDY_PROCESSED : return("Processed");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_IMAGE_FORMAT :
		switch(code) {
			case IID_IMAGEFORMAT_MONO_RAW : return("Mono Raw");
			case IID_IMAGEFORMAT_RGB_RAW : return("RGB Raw");
			case IID_IMAGEFORMAT_MONO_JPEG : return("Mono JPEG");
			case IID_IMAGEFORMAT_RGB_JPEG : return("RGB JPEG");
			case IID_IMAGEFORMAT_MONO_JPEG_LS : return("Mono JPEG LS");
			case IID_IMAGEFORMAT_RGB_JPEG_LS : return("RGB JPEG LS");
			case IID_IMAGEFORMAT_MONO_JPEG2000 : return("Mono JPEG 2000");
			case IID_IMAGEFORMAT_RGB_JPEG2000 : return("RGB JPEG 2000");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_IMAGE_TRANSFORMATION :
		switch(code) {
			case IID_TRANS_UNDEF : return("Undefined");
			case IID_TRANS_STD : return("Standard");
			default : return("Invalid code");
		}
	case IID_CODE_CLASS_BIOMETRIC_SUBTYPE :
		switch(code) {
			case IID_EYE_UNDEF : return("Undefined");
			case IID_EYE_RIGHT : return("Right Eye");
			case IID_EYE_LEFT : return("Left Eye");
			default : return("Invalid code");
		}
	default : return("Invalid class");
	}
}

/******************************************************************************/
/* Implement the interface for allocating and freeing iris image data blocks. */
/******************************************************************************/
int
new_iih(IIH **iih)
{
	IIH *liih;

	liih = (IIH *)malloc(sizeof(IIH));
	if (liih == NULL)
		ALLOC_ERR_RETURN("Iris image header");
	memset((void *)liih, 0, sizeof(IIH));
	*iih = liih;
}

void
free_iih(IIH *iih)
{
	if (iih->image_data != NULL)
		free(iih->image_data);
	free(iih);
}

int
new_ibsh(IBSH **ibsh)
{
	IBSH *libsh;

	libsh = (IBSH *)malloc(sizeof(IBSH));
	if (libsh == NULL)
		ALLOC_ERR_RETURN("Iris biometric subtype header");
	memset((void *)libsh, 0, sizeof(IBSH));
	TAILQ_INIT(&libsh->image_headers);
	*ibsh = libsh;
}

void
free_ibsh(IBSH *ibsh)
{
	IIH *iih;

	while (!TAILQ_EMPTY(&ibsh->image_headers)) {
		iih = TAILQ_FIRST(&ibsh->image_headers);
		TAILQ_REMOVE(&ibsh->image_headers, iih, list);
		free_iih(iih);
	}
}

int
new_iibdb(IIBDB **iibdb)
{
	IIBDB *liibdb;

	liibdb = (IIBDB *)malloc(sizeof(IIBDB));
	if (liibdb == NULL)
		ALLOC_ERR_RETURN("Iris image biometric data block");
	memset((void *)liibdb, 0, sizeof(IIBDB));
	*iibdb = liibdb;
	return (0);
}

void
free_iibdb(IIBDB *iibdb)
{
	IIH *iih;
	int i;

	if (iibdb->biometric_subtype_headers[0] != NULL)
		free_ibsh(iibdb->biometric_subtype_headers[0]);
	if (iibdb->biometric_subtype_headers[1] != NULL)
		free_ibsh(iibdb->biometric_subtype_headers[1]);
	free(iibdb);
}

void
add_iih_to_ibsh(IIH *iih, IBSH *ibsh)
{
	iih->ibsh = ibsh;
	TAILQ_INSERT_TAIL(&ibsh->image_headers, iih, list);
}

/******************************************************************************/
/* Implement the interface for reading/writing/verifying iris image data      */
/* blocks.                                                                    */
/******************************************************************************/
int
read_iih(FILE *fp, IIH *iih)
{
	SREAD(&iih->image_number, fp);
	CREAD(&iih->image_quality, fp);
	SREAD(&iih->rotation_angle, fp);
	SREAD(&iih->rotation_uncertainty, fp);
	LREAD(&iih->image_length, fp);
	if (iih->image_length != 0) {
		iih->image_data = (uint8_t *)malloc(iih->image_length);
		if (iih->image_data == NULL)
			ALLOC_ERR_OUT("Image data buffer");
		OREAD(iih->image_data, 1, iih->image_length, fp);
	}
	return (READ_OK);
eof_out:
	return (READ_EOF);
err_out:
	return (READ_ERROR);
}

int
read_ibsh(FILE *fp, IBSH *ibsh)
{
	int i;
	int ret;
	IIH *iih;

	CREAD(&ibsh->biometric_subtype, fp);
	SREAD(&ibsh->num_images, fp);
	for (i = 0; i < ibsh->num_images; i++) {
		ret = new_iih(&iih);
		if (iih == NULL)
			ALLOC_ERR_OUT("Iris image header");
		ret = read_iih(fp, iih);
		if (ret != READ_OK)
			READ_ERR_OUT("Iris image header");
		add_iih_to_ibsh(iih, ibsh);
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
	IBSH *ibsh;
	uint16_t sval;
	int i;
	int ret;
	IRH *hdr;

	/* Read the Iris record header */
	hdr = &iibdb->record_header;
	OREAD(hdr->format_id, 1, IID_FORMAT_ID_LEN, fp);
	OREAD(hdr->format_version, 1, IID_FORMAT_VERSION_LEN, fp);
	LREAD(&hdr->record_length, fp);
	SREAD(&hdr->capture_device_id, fp);
	CREAD(&hdr->biometric_subtype_count, fp);
	SREAD(&hdr->record_header_length, fp);
	SREAD(&sval, fp);
	hdr->horizontal_orientation = (sval & IID_HORIZONTAL_ORIENTATION_MASK)
	    >> IID_HORIZONTAL_ORIENTATION_SHIFT;
	hdr->vertical_orientation = (sval & IID_VERTICAL_ORIENTATION_MASK)
	    >> IID_VERTICAL_ORIENTATION_SHIFT;
	hdr->scan_type = (sval & IID_SCAN_TYPE_MASK) >> IID_SCAN_TYPE_SHIFT;
	hdr->iris_occlusions = (sval & IID_IRIS_OCCLUSIONS_MASK)
	    >> IID_IRIS_OCCLUSIONS_SHIFT;
	hdr->occlusion_filling = (sval & IID_OCCLUSION_FILLING_MASK)
	    >> IID_OCCLUSION_FILLING_SHIFT;
	hdr->boundary_extraction = (sval & IID_BOUNDARY_EXTRACTION_MASK)
	    >> IID_BOUNDARY_EXTRACTION_SHIFT;
	SREAD(&hdr->diameter, fp);
	SREAD(&hdr->image_format, fp);
	SREAD(&hdr->image_width, fp);
	SREAD(&hdr->image_height, fp);
	CREAD(&hdr->intesity_depth, fp);
	CREAD(&hdr->image_transformation, fp);
	OREAD(hdr->device_unique_id, 1, IID_DEVICE_UNIQUE_ID_LEN, fp);

	/* Read the image headers and image data */
	for (i = 0; i < iibdb->record_header.biometric_subtype_count; i++) {
		if (new_ibsh(&ibsh) < 0) 
			ALLOC_ERR_OUT("Iris Biometric Subtype Header %d");
		ret = read_ibsh(fp, ibsh);
		if (ret == READ_OK)
			iibdb->biometric_subtype_headers[i] = ibsh;
		else if (ret == READ_EOF)
			// XXX Handle a partial read?
			return (READ_EOF);
		else
			READ_ERR_OUT("Iris Biometric Subtype Header %d", i);
	}
	return (READ_OK);

eof_out:
	return (READ_EOF);
err_out:
	return (READ_ERROR);
}

int
write_iih(FILE *fp, IIH *iih)
{
	SWRITE(iih->image_number, fp);
	CWRITE(iih->image_quality, fp);
	SWRITE(iih->rotation_angle, fp);
	SWRITE(iih->rotation_uncertainty, fp);
	LWRITE(iih->image_length, fp);
	if (iih->image_data != NULL)
		OWRITE(iih->image_data, 1, iih->image_length, fp);
	return (WRITE_OK);
err_out:
	return (WRITE_ERROR);
}

int
write_ibsh(FILE *fp, IBSH *ibsh)
{
	IIH *iih;
	int ret;

	CWRITE(ibsh->biometric_subtype, fp);
	SWRITE(ibsh->num_images, fp);
	TAILQ_FOREACH(iih, &ibsh->image_headers, list) {
		ret = write_iih(fp, iih);
		if (ret != WRITE_OK)
			WRITE_ERR_OUT("Iris Image Header");
	}
	return (WRITE_OK);
err_out:
	return (WRITE_ERROR);
}

int
write_iibdb(FILE *fp, IIBDB *iibdb)
{
	int i;
	int ret;
	uint16_t sval;
	IRH *hdr;

	/* Write the Iris record header */
	hdr = &iibdb->record_header;

	OWRITE(hdr->format_id, 1, IID_FORMAT_ID_LEN, fp);
	OWRITE(hdr->format_version, 1, IID_FORMAT_VERSION_LEN, fp);
	LWRITE(hdr->record_length, fp);
	SWRITE(hdr->capture_device_id, fp);
	CWRITE(hdr->biometric_subtype_count, fp);
	SWRITE(hdr->record_header_length, fp);
	sval =
	      (hdr->horizontal_orientation << IID_HORIZONTAL_ORIENTATION_SHIFT)
	    | (hdr->vertical_orientation << IID_VERTICAL_ORIENTATION_SHIFT)
	    | (hdr->scan_type << IID_SCAN_TYPE_SHIFT)
	    | (hdr->iris_occlusions << IID_IRIS_OCCLUSIONS_SHIFT)
	    | (hdr->occlusion_filling << IID_OCCLUSION_FILLING_SHIFT)
	    | (hdr->boundary_extraction << IID_BOUNDARY_EXTRACTION_SHIFT);
	SWRITE(sval, fp);
	SWRITE(hdr->diameter, fp);
	SWRITE(hdr->image_format, fp);
	SWRITE(hdr->image_width, fp);
	SWRITE(hdr->image_height, fp);
	CWRITE(hdr->intesity_depth, fp);
	CWRITE(hdr->image_transformation, fp);
	OWRITE(hdr->device_unique_id, 1, IID_DEVICE_UNIQUE_ID_LEN, fp);

	/* Write the image headers and data */
	for (i = 0; i < iibdb->record_header.biometric_subtype_count; i++) {
		ret = write_ibsh(fp, iibdb->biometric_subtype_headers[i]);
		if (ret != WRITE_OK)
			WRITE_ERR_OUT("Iris Biometric Subtype Header %d", i+1);
	}

	return (WRITE_OK);
err_out:
	return (WRITE_ERROR);
}

int
print_iih(FILE *fp, IIH *iih)
{
	FPRINTF(fp, "\tImage Number\t\t: %hu\n", iih->image_number);
	FPRINTF(fp, "\tImage Quality\t\t: %hhu\n", iih->image_quality);
	FPRINTF(fp, "\tRotation Angle\t\t: %hd\n", iih->rotation_angle);
	FPRINTF(fp, "\tRotation Uncertaintity\t: %hu\n",
	    iih->rotation_uncertainty);
	FPRINTF(fp, "\tImage Length\t\t: %u\n", iih->image_length);
// XXX what to do with the image data? verify length? save to file?
	return (PRINT_OK);
err_out:
	return (PRINT_ERROR);
}

int
print_ibsh(FILE *fp, IBSH *ibsh)
{
	int i;
	int ret;
	IIH *iih;

	FPRINTF(fp, "-----------------------------\n");
	FPRINTF(fp, "Iris Biometric Subtype Header\n");
	FPRINTF(fp, "-----------------------------\n");
	FPRINTF(fp, "Biometric Subtype\t\t: 0x%02X (%s)\n",
	    ibsh->biometric_subtype,
	    iid_code_to_str(IID_CODE_CLASS_BIOMETRIC_SUBTYPE,
		ibsh->biometric_subtype));
	FPRINTF(fp, "No. Images\t\t\t: %d\n", ibsh->num_images);
	i = 1;
	TAILQ_FOREACH(iih, &ibsh->image_headers, list) {
		FPRINTF(fp, "Iris Image Header %d:\n", i);
		ret = print_iih(fp, iih);
		if (ret != PRINT_OK)
			ERR_OUT("Could not print Iris Image Header %d", i);
	}
	FPRINTF(fp, "-----------------------------\n");
	return (PRINT_OK);
err_out:
	return (PRINT_ERROR);
}

int
print_iibdb(FILE *fp, IIBDB *iibdb)
{
	int ret;
	int i;

	FPRINTF(fp, "Format ID\t\t\t: %s\nSpec Version\t\t\t: %s\n",
	    iibdb->record_header.format_id, iibdb->record_header.format_version);
	FPRINTF(fp, "Record Length\t\t\t: %u\n",
	    iibdb->record_header.record_length);
	FPRINTF(fp, "Capture Device ID\t\t: 0x%04x\n",
	    iibdb->record_header.capture_device_id);
	FPRINTF(fp, "No. of Iris Biometric Subtypes\t: %d\n",
	    iibdb->record_header.biometric_subtype_count);
	FPRINTF(fp, "Record Header Length\t\t: %d\n",
	    iibdb->record_header.record_header_length);
	FPRINTF(fp, "Iris Image Properties:\n");
	FPRINTF(fp, "\tHorizontal Orientation\t: %hhu (%s)\n",
	    iibdb->record_header.horizontal_orientation,
	    iid_code_to_str(IID_CODE_CLASS_ORIENTATION,
		iibdb->record_header.horizontal_orientation));
	FPRINTF(fp, "\tVertical Orientation\t: %hhu (%s)\n",
	    iibdb->record_header.vertical_orientation,
	    iid_code_to_str(IID_CODE_CLASS_ORIENTATION,
		iibdb->record_header.vertical_orientation));
	FPRINTF(fp, "\tScan Type\t\t: %hhu (%s)\n",
	    iibdb->record_header.scan_type,
	    iid_code_to_str(IID_CODE_CLASS_SCAN_TYPE,
		iibdb->record_header.scan_type));
	FPRINTF(fp, "\tIris Occlusions\t\t: %hhu (%s)\n",
	    iibdb->record_header.iris_occlusions,
	    iid_code_to_str(IID_CODE_CLASS_OCCLUSION,
		iibdb->record_header.iris_occlusions));
	FPRINTF(fp, "\tOcclusion Filling\t: %hhu (%s)\n",
	    iibdb->record_header.occlusion_filling,
	    iid_code_to_str(IID_CODE_CLASS_OCCLUSION_FILLING,
		iibdb->record_header.occlusion_filling));
	FPRINTF(fp, "\tBoundary Extraction\t: %hhu (%s)\n",
	    iibdb->record_header.boundary_extraction,
	    iid_code_to_str(IID_CODE_CLASS_BOUNDARY_EXTRACTION,
		iibdb->record_header.boundary_extraction));
	FPRINTF(fp, "Iris Diameter\t\t\t: %hu\n",
	    iibdb->record_header.diameter);
	FPRINTF(fp, "Image Format\t\t\t: 0x%04X (%s)\n",
	    iibdb->record_header.image_format,
	    iid_code_to_str(IID_CODE_CLASS_IMAGE_FORMAT,
		iibdb->record_header.image_format));
	FPRINTF(fp, "Image Size\t\t\t: %hux%hu\n",
	    iibdb->record_header.image_width, iibdb->record_header.image_height);
	FPRINTF(fp, "Image Depth\t\t\t: %hhu\n",
	    iibdb->record_header.intesity_depth);
	FPRINTF(fp, "Image Transformation\t\t: %hhu (%s)\n",
	    iibdb->record_header.image_transformation,
	    iid_code_to_str(IID_CODE_CLASS_IMAGE_TRANSFORMATION,
		iibdb->record_header.image_transformation));
	FPRINTF(fp, "Device Unique ID\t\t: ");
	if (iibdb->record_header.device_unique_id[0] != 0)
		FPRINTF(fp, "%s\n", iibdb->record_header.device_unique_id);
	else
		FPRINTF(fp, "Not present\n");

	/* Print the image headers */
	for (i = 0; i < iibdb->record_header.biometric_subtype_count; i++) {
		/* Make sure we actually read the biometric subtype */
		if (iibdb->biometric_subtype_headers[i] != NULL) {
			ret = print_ibsh(fp,
			    iibdb->biometric_subtype_headers[i]);
			if (ret != PRINT_OK)
				ERRP("Could not print Iris Biometric "
				    "Subtype Header %d", i+1);
		} else {
			ERRP("Iris Biometric Subtype Header %d not read", i+1);
		}
	}
	FPRINTF(fp, "\n");
	return (PRINT_OK);
err_out:
	return (PRINT_ERROR);
}

int
validate_iih(IIH *iih)
{
	int ret = VALIDATE_OK;

	if (iih->image_number == 0) {
		ERRP("Image number is 0");
		ret = VALIDATE_ERROR;
	}
	if (iih->image_number > iih->ibsh->num_images) {
		ERRP("Image number %hu greater greater than total of %hu",
		    iih->image_number, iih->ibsh->num_images);
		ret = VALIDATE_ERROR;
	}
	if (iih->rotation_uncertainty != IID_ROT_ANGLE_UNDEF)
		CRSR(iih->rotation_uncertainty, IID_ROT_UNCERTAIN_MIN,
		    IID_ROT_UNCERTAIN_MAX, "Rotation Uncertainty");
	return (ret);
}

int
validate_ibsh(IBSH *ibsh)
{
	int ret = VALIDATE_OK;
	int error;
	IIH *iih;

	if ((ibsh->biometric_subtype != IID_EYE_UNDEF) &&
	    (ibsh->biometric_subtype != IID_EYE_RIGHT) &&
	    (ibsh->biometric_subtype != IID_EYE_LEFT)) {
		ERRP("Biometric Subtype 0x%02X invalid",
		    ibsh->biometric_subtype);
		ret = VALIDATE_ERROR;
	}
	TAILQ_FOREACH(iih, &ibsh->image_headers, list) {
		error = validate_iih(iih);
		if (error != VALIDATE_OK)
			ret = VALIDATE_ERROR;
	}
	return (ret);
}

int
validate_iibdb(IIBDB *iibdb)
{
	int ret = VALIDATE_OK;
	int i;
	int error;
	IRH rh = iibdb->record_header;

	if (strncmp(rh.format_id, IID_FORMAT_ID,
	    IID_FORMAT_ID_LEN) != 0) {
		ERRP("Header format ID is [%s], should be [%s]",
		    rh.format_id, IID_FORMAT_ID);
		ret = VALIDATE_ERROR;
	}
	/* The version is restricted to numeric ASCII strings, but we
	 * don't have a specific version string to check for.
	 */
	if (rh.format_version[IID_FORMAT_VERSION_LEN - 1] != 0) {
		ERRP("Header format ID version is not NULL-terminated.");
		ret = VALIDATE_ERROR;
	}
	for (i = 0; i < IID_FORMAT_VERSION_LEN - 2; i++) {
		if (!(isdigit(rh.format_version[i]))) {
			ERRP("Header format ID version is non-numeric.");
			ret = VALIDATE_ERROR;
			break;
		}
	}
	CRSR(rh.biometric_subtype_count, IID_MIN_BIOMETRIC_SUBTYPES,
	    IID_MAX_BIOMETRIC_SUBTYPES, "Biometric Subtype Count");
	CSR(rh.record_header_length, IID_RECORD_HEADER_LENGTH,
	    "Record Header Length");
	//XXX Should we check bitfields in iris image properties?
	//XXX should we check iris diameter against image size?

	if ((rh.image_format != IID_IMAGEFORMAT_MONO_RAW) &&
	    (rh.image_format != IID_IMAGEFORMAT_RGB_RAW) &&
	    (rh.image_format != IID_IMAGEFORMAT_MONO_JPEG) &&
	    (rh.image_format != IID_IMAGEFORMAT_RGB_JPEG) &&
	    (rh.image_format != IID_IMAGEFORMAT_MONO_JPEG_LS) &&
	    (rh.image_format != IID_IMAGEFORMAT_RGB_JPEG_LS) &&
	    (rh.image_format != IID_IMAGEFORMAT_MONO_JPEG2000) &&
	    (rh.image_format != IID_IMAGEFORMAT_RGB_JPEG2000)) {
		ERRP("Image format 0x%04X invalid", rh.image_format);
		ret = VALIDATE_ERROR;
	}
	if ((rh.image_transformation != IID_TRANS_UNDEF) &&
	    (rh.image_transformation != IID_TRANS_STD)) {
		ERRP("Image transformation %hhu invalid",
		    rh.image_transformation);
		ret = VALIDATE_ERROR;
	}
	if ((rh.device_unique_id[0] != IID_DEVICE_UNIQUE_ID_SERIAL_NUMBER) &&
	    (rh.device_unique_id[0] != IID_DEVICE_UNIQUE_ID_MAC_ADDRESS) &&
	    (rh.device_unique_id[0] != IID_DEVICE_UNIQUE_ID_PROCESSOR_ID) &&
	    (memcmp(rh.device_unique_id, IID_DEVICE_UNIQUE_ID_NONE,
		IID_DEVICE_UNIQUE_ID_LEN) != 0)) {
		ERRP("Device Unique ID Invalid");
		ret = VALIDATE_ERROR;
	}
	
	/* Validate the image headers */
	for (i = 0; i < iibdb->record_header.biometric_subtype_count; i++) {
		/* Make sure we actually read the biometric subtype */
		if (iibdb->biometric_subtype_headers[i] != NULL) {
			error = validate_ibsh(
			    iibdb->biometric_subtype_headers[i]);
			if (error != VALIDATE_OK)
				ret = VALIDATE_ERROR;
		}
	}
	return (ret);
}

/******************************************************************************/
/* Implementation of the higher level access routines.                        */
/******************************************************************************/
int
get_ibsh_count(IIBDB *iibdb)
{
	return (iibdb->record_header.biometric_subtype_count);
}

int
get_ibshs(IIBDB *iibdb, IBSH *ibshs[])
{
	int i;

	for (i = 0; i < iibdb->record_header.biometric_subtype_count; i++)
		ibshs[i] = iibdb->biometric_subtype_headers[i];

	return (iibdb->record_header.biometric_subtype_count);
}

int
get_iih_count(IBSH *ibsh)
{
	return (ibsh->num_images);
}

int
get_iihs(IBSH *ibsh, IIH *iihs[])
{
	int count = 0;

	IIH *iih;

	TAILQ_FOREACH(iih, &ibsh->image_headers, list) {
		iihs[count] = iih;
		count++;
	}
	return (count);
}
