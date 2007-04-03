#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#

# This set of directories is where the header files, libraries, programs,
# and man pages are to be installed.
INCPATH := /usr/local/include
LIBPATH := /usr/local/lib
BINPATH := /usr/local/bin
MANPATH := /usr/local/man/man1

#
# Each package that includes this common file can define these variables:
# COMMONINCOPT : Location of include files from other packages, specified
#                as a compiler option (e.g. -I/usr/local/an2k/include
#
# COMMONLIBOPT : Location of libraries from other packages, specified as a
#                compiler option (e.g. -L/usr/local/an2k/lib)
#
# The next set of variables are set by files that include this file, and
# specify the location of package-only files:
#
# LOCALINC : Location where include files are stored.
# LOCALLIB : Location where the libaries are stored.
# LOCALBIN : Location where the programs are stored.
# LOCALMAN : Locatation where the man pages are stored.
#
CP := cp -f
RM := rm -f
PWD := $(shell pwd)
OS := $(shell uname -s)

ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	ROOT = Administrator
else
	ROOT  = root
endif

#
# If there are any 'non-standard' include or lib directories that need to
# be searched prior to the 'standard' libraries, add the to the CFLAGS
# variable.

CFLAGS := -g $(COMMONINCOPT) -I$(LOCALINC) -I$(INCPATH) $(COMMONLIBOPT) -L$(LOCALLIB) -L$(LIBPATH)
