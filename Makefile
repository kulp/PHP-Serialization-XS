LIB_EXT ?= .a
RANLIB  ?= ranlib
DEFINES += _POSIX_C_SOURCE=200112L _XOPEN_SOURCE=700
CFLAGS  += -std=c99 -W -Wall -Werror -Wextra -pedantic-errors \
		   $(addprefix -D,$(DEFINES)) -Wno-unused-parameter
CFLAGS  += -g # for debugging only
LDLIBS  += -lyaml
CFILES  += $(wildcard *.c)
TARGETS += ps2yaml

vpath %.c parser

STORE_C = $(wildcard parser/*store.c)
STORE_O = $(STORE_C:.c=.o)

all: libphpserialize$(LIB_EXT) $(TARGETS)

libphpserialize$(LIB_EXT): ps_parser.o $(STORE_O)
	$(AR) cr $@ $^
	$(RANLIB) $@

ps2yaml: ps2yaml.o ps_parser.o
ifeq ($(USE_MMAP),1)
ps2yaml: DEFINES += USE_MMAP
ps2yaml: mmapstore.o
else
ps2yaml: filestore.o
endif

CLEANFILES += $(TARGETS) *.[od] $(STORE_O) libphpserialize$(LIB_EXT)

.PHONY: clean
clean:
	-$(RM) -r $(CLEANFILES)

ifneq ($(MAKECMDGOALS),clean)
-include $(CFILES:.c=.d)
endif

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) -MM -MG -MF $@.$$$$ $<; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

