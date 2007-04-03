/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
/*
 * Collection of macros that are useful.
 */

#ifndef _M1IO_H
#define _M1IO_H 

/******************************************************************************/
/* Common definitions used throughout                                         */
/******************************************************************************/
// Return codes for the read* functions
#define READ_OK		0
#define READ_EOF	1
#define READ_ERROR	2

// Return codes for the write* functions
#define WRITE_OK	0
#define WRITE_ERROR	1

// Return codes for the print* functions
#define PRINT_OK	0
#define PRINT_ERROR	1

// Return codes for the validate* functions
#define VALIDATE_OK	0
#define VALIDATE_ERROR	1

#define NULL_VERBOSITY_LEVEL	0
#define ERR_VERBOSITY_LEVEL	1
#define INFO_VERBOSITY_LEVEL	2
#define MAX_VERBOSITY_LEVEL	2

/*
 * Note that in order to use most of these macros, two labels, 'err_out'
 * and 'eof_out' must be defined within the function that uses the macro.
 * The presumption is that code starting at 'err_out' will unwind any state
 * (closing files, freeing memory, etc.) before exiting the function with
 * an error code.
 *
 */

#define OREAD(ptr, size, nmemb, stream)					\
	do {								\
		if (fread(ptr, size, nmemb, stream) < nmemb) {		\
		  if (feof(fp)) {					\
			goto eof_out;					\
		  } else {						\
		    fprintf(stderr, 					\
			    "Error reading at position %d from %s:%d\n",\
			    ftell(stream), __FILE__, __LINE__);		\
			goto err_out;					\
		  }							\
		}							\
	} while (0)

#define OWRITE(ptr, size, nmemb, stream)				\
	do {								\
		if (fwrite(ptr, size, nmemb, stream) < nmemb) {		\
		  fprintf(stderr, 					\
			    "Error writing at position %d from %s:%d\n",\
			    ftell(stream), __FILE__, __LINE__);		\
			goto err_out;					\
		}							\
	} while (0)

#define FPRINTF(stream, ...)						\
	do {								\
		if (fprintf(stream, __VA_ARGS__) < 0) {			\
		  fprintf(stderr, 					\
			    "Error printing at position %d from %s:%d\n",\
			    ftell(stream), __FILE__, __LINE__);		\
			goto err_out;					\
		}							\
	} while (0)

// Macros to read a single char, short, or int and convert from big-endian
// to host native format
#define CREAD(ptr, stream)						\
	do {								\
		unsigned char __cval;					\
		OREAD(&__cval, 1, 1, stream);				\
		*ptr = __cval;						\
	} while (0)

#define SREAD(ptr, stream)						\
	do {								\
		unsigned short __sval;					\
		OREAD(&__sval, 2, 1, stream);				\
		*ptr = ntohs(__sval);					\
	} while (0)

#define LREAD(ptr, stream)						\
	do {								\
		unsigned int __lval;					\
		OREAD(&__lval, 4, 1, stream);				\
		*ptr = ntohl(__lval);					\
	} while (0)

// Macros to write a single char, short, or int and convert from big-endian
// to host native format. 
#define CWRITE(ptr, stream)						\
	do {								\
		unsigned char __cval = (unsigned char)(*ptr);		\
		OWRITE(&__cval, 1, 1, stream);				\
	} while (0)

#define SWRITE(ptr, stream)						\
	do {								\
		unsigned short __sval = (unsigned short)htons(*ptr);	\
		OWRITE(&__sval, 2, 1, stream);				\
	} while (0)

#define LWRITE(ptr, stream)						\
	do {								\
		unsigned int __ival = (unsigned long)htonl(*ptr);	\
		OWRITE(&__ival, 4, 1, stream);				\
	} while (0)

// Other common things to check and take action on error
#define OPEN_ERR_EXIT(fn)						\
	do {								\
		fprintf(stderr, "Could not open file %s: ", fn);	\
		fprintf(stderr, "%s\n", strerror(errno));		\
		exit(EXIT_FAILURE);					\
	} while (0)

#define READ_ERR_RETURN(...)						\
	do {								\
		fprintf(stderr, "Error reading ");			\
		fprintf(stderr, __VA_ARGS__);				\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		return (READ_ERROR);					\
	} while (0)

#define READ_ERR_OUT(...)						\
	do {								\
		fprintf(stderr, "Error reading ");			\
		fprintf(stderr, __VA_ARGS__);				\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		goto err_out;						\
	} while (0)

#define ALLOC_ERR_EXIT(msg)						\
	do {								\
		fprintf(stderr, "Error allocating %s.", msg);		\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		exit(EXIT_FAILURE);					\
	} while (0)

#define ALLOC_ERR_RETURN(msg)						\
	do {								\
		fprintf(stderr, "Error allocating %s.", msg);		\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		return (-1);						\
	} while (0)

#define ALLOC_ERR_OUT(msg)						\
	do {								\
		fprintf(stderr, "Error allocating %s.", msg);		\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		goto err_out;						\
	} while (0)

#define WRITE_ERR_OUT(...)						\
	do {								\
		fprintf(stderr, "Error writing ");			\
		fprintf(stderr, __VA_ARGS__);				\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		goto err_out;						\
	} while (0)

#define ERR_OUT(...)							\
	do {								\
		fprintf(stderr, "ERROR: ");				\
		fprintf(stderr, __VA_ARGS__);				\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		goto err_out;						\
	} while (0)

#define ERR_EXIT(...)							\
	do {								\
		fprintf(stderr, "ERROR: ");				\
		fprintf(stderr, __VA_ARGS__);				\
		fprintf(stderr, " (line %d in %s).\n", __LINE__, __FILE__);\
		exit(EXIT_FAILURE);					\
	} while (0)

/*
 * Check in Range and Warn: Check if a value falls within a given integer
 * range, and print given message if not in range.
 */
#define CRW(value, low, high, msg)					\
	do {								\
		if (((value) < (low)) || (value) > (high)) {		\
			fprintf(stderr, "Warning: %s not in range\n", msg);\
		}							\
	} while (0)

/*                
 * Compare and Set Return: Macro to compare two integer values and set  
 * variable 'ret' to indicate an error. Will print an optional message. 
 * Parameter 'valid' is considered the valid value.
 */                     
#define CSR(value, valid, msg)						\
	do {								\
		if ((value) != (valid)) {				\
			if(msg != NULL)					\
			    fprintf(stderr,				\
				"ERROR: %s not %d.\n", msg, valid);	\
			ret = VALIDATE_ERROR;				\
		}							\
	} while (0)							\

/*                
 * Negative Compare and Set Return: Macro to compare two integer values
 * and set variable 'ret' to indicate an error if the values are equal.
 * Will print an optional message.
 */                     
#define NCSR(value, invalid, msg)					\
	do {								\
		if ((value) == (invalid)) {				\
			if(msg != NULL)					\
			    fprintf(stderr,				\
				"ERROR: %s invalid value %d.\n", msg, value);\
			ret = VALIDATE_ERROR;				\
		}							\
	} while (0)							\

/*
 * Print INFO and ERROR messages to the appropriate place.
 */

#define INFOP(...)							\
	do {								\
		fprintf(stdout, "INFO: ");				\
		fprintf(stdout, __VA_ARGS__);				\
		fprintf(stdout, ".\n");					\
	} while (0)

#define ERRP(...)							\
	do {								\
		fprintf(stderr, "ERROR: ");				\
		fprintf(stderr, __VA_ARGS__);				\
		fprintf(stderr, ".\n");					\
	} while (0)

/*
 * SOME systems may not have the latest queue(3) package, so borrow some
 * of the macros from queue.h on a system that does have the complete 
 * package.
 */
/*
 * Copyright (c) 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#ifndef TAILQ_FIRST
#define TAILQ_FIRST(head) ((head)->tqh_first)
#endif

#ifndef TAILQ_NEXT
#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#endif

#ifndef TAILQ_EMPTY
#define TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#endif

#ifndef TAILQ_FOREACH
#define TAILQ_FOREACH(var, head, field)                                 \
	for (var = TAILQ_FIRST(head); var; var = TAILQ_NEXT(var, field))
#endif

#endif /* !_M1IO_H  */
