#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
include ../common.mk
SOURCES = biomdi.c
OBJECTS = biomdi.o

all: $(SOURCES)
ifeq ($(OS), Darwin)
	$(CC) -dynamiclib $(EXTRALIBS) $(CFLAGS) $(SOURCES) -o libbiomdi.dylib
	$(CP) libbiomdi.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(CC) $(CFLAGS) -c $(SOURCES)
	ar rs libbiomdi.a $(OBJECTS)
	ranlib libbiomdi.a
	$(CC) -shared -o libbiomdi.dll -Wl,--out-implib=libbiomdi.dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive libbiomdi.a -Wl,--no-whole-archive
	$(CP) libbiomdi.a $(LOCALLIB)
	$(CP) libbiomdi.dll.a $(LOCALLIB)
	$(CP) libbiomdi.dll $(LOCALLIB)
else
	$(CC) $(CFLAGS) -shared $(SOURCES) -lm -o libbiomdi.so
	$(CP) libbiomdi.so $(LOCALLIB)
endif
endif

clean:
	$(RM) $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
