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
all: elliptical.c polar.c fmrprune.c
	$(CC) $(CFLAGS) -c polar.c
#	$(CC) $(CFLAGS) -c elliptical.c -DDEBUG
	$(CC) $(CFLAGS) -c elliptical.c
	$(CC) $(CFLAGS) -c rectangular.c
#	$(CC) $(CFLAGS) -c rectangular.c -DDEBUG
	$(CC) $(CFLAGS) -c random.c
	$(CC) $(CFLAGS) fmrprune.c elliptical.o polar.o random.o rectangular.o -lm -lfmr -o $(LOCALBIN)/fmrprune
	$(CP) fmrprune.1 $(LOCALMAN)

clean:
	$(RM) *.o
