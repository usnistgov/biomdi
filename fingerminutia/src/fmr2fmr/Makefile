#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
include ../common.mk
all: ansi2iso iso2ansi fmr2fmr
ansi2iso: ansi2iso.c ansi2iso.h
	$(CC) -c $(CFLAGS) ansi2iso.c -o ansi2iso.o

iso2ansi: iso2ansi.c iso2ansi.h
	$(CC) -c $(CFLAGS) iso2ansi.c -o iso2ansi.o

fmr2fmr: fmr2fmr.c ansi2iso iso2ansi
	$(CC) $(CFLAGS) ansi2iso.o iso2ansi.o fmr2fmr.c -lm -lfmr -o $(LOCALBIN)/fmr2fmr
	$(CP) fmr2fmr.1 $(LOCALMAN)

clean:
	$(RM) *.o
