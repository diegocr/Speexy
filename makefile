
APP	= Speexy
SYS	= _020
CPU	= -m68020-60
PACKAGE	= $(APP)
CC	= gcc
NSTDL	= -noixemul #-nostdlib
CFLAGS	= $(CPU) -O2 -noixemul -c -g -Wall $(NSTDL) -I.
LFLAGS	= -s
LIBS	= -logg -ldebug -lamiga

COMPILE		= $(CC) $(CFLAGS)
LINK		= $(CC) $(NSTDL) -Wl,-Map,$@.map,--cref

DEPDIR		= .deps
OBJDIR		= .objs

CCDEPMODE	= depmode=$(CC)
depcomp		= $(SHELL) /bin/depcomp

OBJS =	\
	$(OBJDIR)/amuistartup$(SYS).o	\
	$(OBJDIR)/ahi$(SYS).o	\
	$(OBJDIR)/classes$(SYS).o	\
	$(OBJDIR)/mui$(SYS).o	\
	$(OBJDIR)/speexdec$(SYS).o

default:	$(PACKAGE)

$(PACKAGE): $(OBJS)
	if test -f $@; then if test -f $@~; then rm -f $@~; fi; mv -f $@ $@~; fi;
	@echo Linking Debug version...
	$(LINK) -o $@.deb $(OBJS) $(LIBS)
	@echo Linking pure...
	$(LINK) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

all:
	$(MAKE) SYS='_020' CPU='-m68020-60'
	$(MAKE) SYS='_020f' CPU='-m68020 -m68881'
	$(MAKE) SYS='_040' CPU='-m68040 -m68881'
	$(MAKE) SYS='_060' CPU='-m68060 -m68881'


include ./$(DEPDIR)/ahi$(SYS).Po
include ./$(DEPDIR)/amuistartup$(SYS).Po
include ./$(DEPDIR)/classes$(SYS).Po
include ./$(DEPDIR)/mui$(SYS).Po
include ./$(DEPDIR)/speexdec$(SYS).Po

$(OBJDIR)/%$(SYS).o: %.c
	@echo Making $@...
	source='$<' object='$@' libtool=no \
	depfile='$(DEPDIR)/$*$(SYS).Po' tmpdepfile='$(DEPDIR)/$*$(SYS).TPo' \
	$(CCDEPMODE) $(depcomp) \
	$(COMPILE) -o $@ -c ./$<

clean:
	rm -f $(OBJDIR)/*$(SYS).o


.SILENT:
