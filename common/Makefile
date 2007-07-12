#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
#
# Top-level make file to install the BIOMDI common header files.
#
LOCALINC := src/include
INCPATH := /usr/local/include
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	ROOT = Administrator
else
	ROOT  = root
endif

all:
	@echo 'Nothing to be done for "make all".'

install:
	install -m 644 -o $(ROOT) $(LOCALINC)/*.h $(INCPATH)
clean:
	@echo 'Nothing to be done for "make clean".'
