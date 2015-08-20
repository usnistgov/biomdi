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
ifeq ($(OS), Darwin)
	LDEXTRA=-Wl,-search_paths_first
endif
all: pgm2fir.cpp
	$(CC) $(LDEXTRA) $? -lstdc++ -lnfiq -lmindtct -lnetpbm -lwsq -lutil -ljpegl -lmlp -lfet -lcblas -lioutil -lutil $(CFLAGS) -o pgm2fir
	${CP} pgm2fir $(LOCALBIN)
	${CP} pgm2fir.1 $(LOCALMAN)

clean:
	$(RM) pgm2fir $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
