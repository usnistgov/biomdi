/*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
*/
/******************************************************************************/
/* Header file for the Iris Image Data Record format, as specified in the     */
/* Iris Exchange (IREX) Evaluation 2008 specification.                        */
/*                                                                            */
/* Layout of the entire iris image biometric data block (record) in memory:   */
/*                                                                            */
/*       Iris Image Biometric Data Record                                     */
/*   +-------------------------------------+                                  */
/*   |  iris general header (16 bytes)     |                                  */
/*   +-------------------------------------+                                  */
/*       List {                                                               */
/*           +-----------------------------------|                            */
/*           |  iris representation header       |                            */
/*           +-----------------------------------|                            */
/*           +------------+                                                   */
/*           | image data |                                                   */
/*           +------------+                                                   */
/*       }                                                                    */
/*                                                                            */
/******************************************************************************/
#ifndef _IID_H
#define _IID_H

#include <string.h>
/*
 * Class codes for converting codes to strings. See the iid_code_str()
 * function defined below.
 */
#define IID_CODE_CATEGORY_ORIENTATION		0
#define IID_CODE_CATEGORY_IMAGE_TYPE		1
#define IID_CODE_CATEGORY_IMAGE_FORMAT		2
#define IID_CODE_CATEGORY_EYE_LABEL		3
#define IID_CODE_CATEGORY_NUM_EYES		4
#define IID_CODE_CATEGORY_COMPRESSION_HISTORY	5

/******************************************************************************/
/* Definitions for the Iris Record Header fields.                             */
/******************************************************************************/
#define IID_FORMAT_ID				"IIR"
#define IID_FORMAT_ID_LEN			4
#define IID_ISO_FORMAT_VERSION			"020"
#define IID_FORMAT_VERSION_LEN			4
#define IID_RECORD_HEADER_LENGTH		15
#define IID_MIN_IRISES				1
#define IID_MAX_IRISES				65535
#define IID_MIN_EYES				0
#define IID_MAX_EYES				2
#define IID_NUM_EYES_UNKNOWN			0
#define IID_NUM_EYES_LEFT_OR_RIGHT		1
#define IID_NUM_EYES_LEFT_AND_RIGHT		2

struct iris_general_header {
#define igh_startcopy		format_id
	char			format_id[IID_FORMAT_ID_LEN];
	char			format_version[IID_FORMAT_VERSION_LEN];
	uint32_t		record_length;
	uint16_t		num_irises;
	uint8_t			cert_flag;
	uint8_t			num_eyes;
#define igh_endcopy		dummy
	uint32_t		dummy;
};
typedef struct iris_general_header IGH;

/******************************************************************************/
/* Definitions for the Iris Image Header fields.                              */
/******************************************************************************/

/* Capture Date field */
#define IID_CAPTURE_DATE_LEN			9
#define IID_CAPTURE_DATE_STRING_LEN		20

/* Capture device ID fields */
#define IID_CAPTURE_DEVICE_TECHNOLOGY_UNSPEC	0x00
#define IID_CAPTURE_DEVICE_TECHNOLOGY_CMOSCCD	0x01
#define IID_CAPTURE_DEVICE_UNSPEC		0x0000

/* Quality block field */
#define IID_IMAGE_QUAL_MAX_ENTRIES		255
#define IID_IMAGE_QUAL_MIN_SCORE		0
#define IID_IMAGE_QUAL_MAX_SCORE		100
#define IID_IMAGE_QUAL_FAILED			255

/* Eye label field */
#define IID_SUBJECT_EYE_UNDEF			0x00
#define IID_SUBJECT_EYE_RIGHT			0x01
#define IID_SUBJECT_EYE_LEFT			0x02

/* Image type field */
#define IID_TYPE_UNCROPPED			0x01
#define IID_TYPE_VGA				0x02
#define IID_TYPE_CROPPED			0x03
#define IID_TYPE_CROPPED_AND_MASKED		0x07

/* Image format field */
#define IID_IMAGEFORMAT_MONO_RAW		0x02
#define IID_IMAGEFORMAT_MONO_JPEG2000		0x0A
#define IID_IMAGEFORMAT_MONO_PNG		0x0E

/* Image properties field */
#define IID_ORIENTATION_UNDEF			0
#define IID_ORIENTATION_BASE			1
#define IID_ORIENTATION_FLIPPED			2
#define IID_HORZ_ORIENTATION_MASK		0x0003
#define IID_HORZ_ORIENTATION_SHIFT		0
#define IID_VERT_ORIENTATION_MASK		0x000C
#define IID_VERT_ORIENTATION_SHIFT		2
#define IID_PREV_COMPRESSION_UNDEF		0
#define IID_PREV_COMPRESSION_LOSSLESS_NONE	1
#define IID_PREV_COMPRESSION_LOSSY		2
#define IID_PREV_COMPRESSION_MASK		0x00C0
#define IID_PREV_COMPRESSION_SHIFT		6

#define IID_IMAGE_BIT_DEPTH_MIN			8

/* Range field */
#define IID_RANGE_UNASSIGNED			0
#define IID_RANGE_FAILED			1
#define IID_RANGE_OVERFLOW			65535

/* Roll angle fields */
#define IID_ROLL_ANGLE_MIN			0
#define IID_ROLL_ANGLE_MAX			65534
#define IID_ROLL_ANGLE_UNDEF			65535
#define IID_ROLL_ANGLE_UNCERTAINTY_MIN		0
#define IID_ROLL_ANGLE_UNCERTAINTY_MAX		65534

/*Iris centre and diameter fields */
#define IID_COORDINATE_UNDEF			0
#define IID_COORDINATE_SMALLEST_XY		1
#define IID_COORDINATE_LARGEST_XY		65535

struct iris_quality_block {
	uint8_t	 score;
	uint16_t algorithm_vendor_id;
	uint16_t algorithm_id;
};
typedef struct iris_quality_block IIDQB;

struct iris_representation_header {
#define irh_startcopy			representation_length
	uint32_t			representation_length;
	uint8_t				capture_date[IID_CAPTURE_DATE_LEN];
	uint8_t				capture_device_tech_id;
	uint16_t			capture_device_vendor_id;
	uint16_t			capture_device_type_id;
	uint8_t				num_quality_blocks;
	IIDQB			quality_block[IID_IMAGE_QUAL_MAX_ENTRIES];
	uint16_t			representation_number;
	uint8_t				eye_label;
	uint8_t				image_type;
	uint8_t				image_format;
	uint8_t				horz_orientation;
	uint8_t				vert_orientation;
	uint8_t				compression_history;
	uint16_t			image_width;
	uint16_t			image_height;
	uint8_t				bit_depth;
	uint16_t			range;
	uint16_t			roll_angle;
	uint16_t			roll_angle_uncertainty;
	uint16_t			iris_center_smallest_x;
	uint16_t			iris_center_largest_x;
	uint16_t			iris_center_smallest_y;
	uint16_t			iris_center_largest_y;
	uint16_t			iris_diameter_smallest;
	uint16_t			iris_diameter_largest;
	uint32_t			image_length;
#define irh_endcopy			image_data
	uint8_t				*image_data;
	TAILQ_ENTRY(iris_representation_header)	list;
	struct iris_image_biometric_data_block *iibdb; /* ptr to parent block */
};
typedef struct iris_representation_header IRH;

struct iris_image_biometric_data_block {
	IGH						general_header;
	TAILQ_HEAD(, iris_representation_header)	image_headers;
};
typedef struct iris_image_biometric_data_block IIBDB;

/******************************************************************************/
/* Define the interface for allocating and freeing iris image data blocks.    */
/******************************************************************************/
int new_irh(IRH **irh);
void free_irh(IRH *irh);

int new_iibdb(IIBDB **iibdb);
void free_iibdb(IIBDB *iibdb);

/******************************************************************************/
/* Define the interface for reading/writing/verifying iris image data blocks. */
/******************************************************************************/
/******************************************************************************/
/* Functions to read Iris Image records from a file, or buffer. Each function */
/* reads/scans the complete record, including all sub-records. For example,   */
/* read_iibdb() reads the general header, then the iris representation        */
/* headers and associated image data.                                         */
/* The FILE and BDB structs are modified by these functions.                  */
/*                                                                            */
/* Parameters:                                                                */
/*   fp     The open file pointer.                                            */
/*   bdb    Pointer to the biometric data block containing iris data.         */
/*   irh    Pointer to the output iris representation header structure.       */
/*   iibdb  Pointer to the output iris image biometric datablock structure.   */
/*                                                                            */
/* Return:                                                                    */
/*        READ_OK     Success                                                 */
/*        READ_EOF    End of file encountered                                 */
/*        READ_ERROR  Failure                                                 */
/******************************************************************************/
int read_iibdb(FILE *fp, IIBDB *iibdb);
int scan_iibdb(BDB *bdb, IIBDB *iibdb);
int read_irh(FILE *fp, IRH *irh);
int scan_irh(BDB *bdb, IRH *irh);

/******************************************************************************/
/* Functions to write Iris Image records to a file, or buffer. Each           */
/* function writes/pushes the complete record, including all sub-records.     */
/* The FILE and BDB structs are modified by these functions.                  */
/*                                                                            */
/* Parameters:                                                                */
/*   fp     The open file pointer.                                            */
/*   bdb    Pointer to the biometric data block containing iris data.         */
/*   irh    Pointer to the input iris representation header structure.        */
/*   iibdb  Pointer to the input iris image biometric datablock structure.    */
/*                                                                            */
/* Return:                                                                    */
/*        WRITE_OK    Success                                                 */
/*        WRITE_ERROR Failure                                                 */
/******************************************************************************/
int write_irh(FILE *fp, IRH *irh);
int push_irh(BDB *bdb, IRH *irh);
int write_iibdb(FILE *fp, IIBDB *iibdb);
int push_iibdb(BDB *bdb, IIBDB *iibdb);

/******************************************************************************/
/* Functions to print Iris Image records to a file in human-readable form.    */
/*                                                                            */
/* Parameters:                                                                */
/*   fp     The open file pointer.                                            */
/*   irh    Pointer to the input iris representation header structure.        */
/*   iibdb  Pointer to the input iris image biometric datablock structure.    */
/*                                                                            */
/* Return:                                                                    */
/*        WRITE_OK    Success                                                 */
/*        WRITE_ERROR Failure                                                 */
/******************************************************************************/
int print_irh(FILE *fp, IRH *irh);
int print_iibdb(FILE *fp, IIBDB *iibdb);

/******************************************************************************/
/* Functions to validate Iris Image records according to the requirements of  */
/* the ISO/IEC 19794-6:2005 Iris Image Data standard.                         */
/*                                                                            */
/* Parameters:                                                                */
/*   irh    Pointer to the input iris representation header structure.        */
/*   iibdb  Pointer to the input iris image biometric datablock structure.    */
/*                                                                            */
/* Return:                                                                    */
/*        VALIDATE_OK       Record does conform                               */
/*        VALIDATE_ERROR    Record does NOT conform                           */
/******************************************************************************/
int validate_irh(IRH *irh);
int validate_iibdb(IIBDB *fir);

void add_irh_to_iibdb(IRH *irh, IIBDB *iibdb);

/******************************************************************************/
/* Defintion of the higher level access routines.                             */
/******************************************************************************/
int get_irh_count(IIBDB *iibdb);
int get_irhs(IIBDB *iibdb, IRH *irhs[]);

/*
 * Convert a code (image orientation, scan type, etc. to a string.
 *
 * Parameters:
 *   category	One of the category (IID_CATEGORY_IMAGE_TYPE, etc.) codes
 *   code	The coded to convert, e.g. one of the scan types
 * Returns:
 *   A string matching the code within the category, or 'INVALID'
 */
char * iid_code_to_str(int category, int code);

/*
 * Determine whether an encoded date is valid, or undefined, according
 * to the Iris Image specification.
 *
 * Parameters:
 *   date	The encoded date array
 * Returns:
 *   IID_CAPTURE_DATE_VALID
 *   IID_CAPTURE_DATE_INVALID
 *   IID_CAPTURE_DATE_UNDEFINED
 */
int encoded_date_check(uint8_t *date);

/*
 * Clone a complete Iris Image Data Block. The caller is responsible for
 * freeing the clone. Will return -1 on allocation failures, 0 otherwise.
 * Parameters:
 *    src      Pointer to the source data block
 *    dst      Pointer to the destination data block
 *    cloneimg Flag indicating whether to clone the image data
 * The macros that follow copy the data items in each structure, but do
 * not copy the linkages between data blocks.
 */
int clone_iibdb(IIBDB *src, IIBDB **dst, int cloneimg);

#define COPY_IGH(src, dst)						\
	memcpy(&dst->igh_startcopy, &src->igh_startcopy,		\
	    (unsigned)((uint8_t *)&dst->igh_endcopy -			\
		(uint8_t *)&dst->igh_startcopy))

#define COPY_IRH(src, dst)						\
	memcpy(&dst->irh_startcopy, &src->irh_startcopy,		\
	    (unsigned)((uint8_t *)&dst->irh_endcopy -			\
		(uint8_t *)&dst->irh_startcopy))

#endif 	/* _IID_H */
