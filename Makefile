HDRDIR=./include
BINDIR=./bin
LIBDIR=./lib
OBJDIR=./obj

DIRS=$(BINDIR) $(LIBDIR) $(OBJDIR)
HDRS=$(HDRDIR)/bob.h $(HDRDIR)/bobcom.h $(HDRDIR)/bobint.h
PROGS=$(BINDIR)/bob2 $(BINDIR)/bobc2 $(BINDIR)/bobi2 $(BINDIR)/bobmerge2
LIBS=$(LIBDIR)/libbobc.a $(LIBDIR)/libbobi.a

CFLAGS=-I$(HDRDIR) -DBOB_INCLUDE_FLOAT_SUPPORT

all:	$(DIRS) $(PROGS) $(LIBS)

$(BINDIR):
	mkdir $(@F)

$(LIBDIR):
	mkdir $(@F)

$(OBJDIR):
	mkdir $(@F)

###############
# BOB
###############

BOB_OBJS=\
$(OBJDIR)/bob.o

$(BINDIR)/bob2:	$(BOB_OBJS) lib/libbobc.a lib/libbobi.a
	$(CC) -o $@ $(BOB_OBJS) -L$(LIBDIR) -lbobc -lbobi -lm

$(BOB_OBJS):	$(OBJDIR)%.o:	bob%.c
	$(CC) -c $(CFLAGS) $< -o $@

###############
# BOBC
###############

BOBC_OBJS=\
$(OBJDIR)/bobc.o

$(BINDIR)/bobc2:	$(BOBC_OBJS) lib/libbobc.a lib/libbobi.a
	$(CC) -o $@ $(BOBC_OBJS) -L$(LIBDIR) -lbobc -lbobi -lm

$(BOBC_OBJS):	$(OBJDIR)%.o:	bobc%.c $(HDRS)
	$(CC) -c $(CFLAGS) $< -o $@

###############
# BOBI
###############

BOBI_OBJS=\
$(OBJDIR)/bobi.o

$(BINDIR)/bobi2:	$(BOBI_OBJS) lib/libbobi.a
	$(CC) -o $@ $(BOBI_OBJS) -L$(LIBDIR) -lbobi -lm

$(BOBI_OBJS):	$(OBJDIR)%.o:	bobi%.c $(HDRS)
	$(CC) -c $(CFLAGS) $< -o $@

###############
# BOBCOM
###############

BOBCOM_OBJS=\
$(OBJDIR)/bobcom.o \
$(OBJDIR)/bobeval.o \
$(OBJDIR)/bobscn.o \
$(OBJDIR)/bobwcode.o

$(BOBCOM_OBJS):	$(OBJDIR)%.o:	bobcom%.c $(HDRS)
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBDIR)/libbobc.a:	$(BOBCOM_OBJS)
	@$(AR) crs $(LIBDIR)/libbobc.a $(BOBCOM_OBJS)

###############
# BOBINT
###############

BOBINT_OBJS=\
$(OBJDIR)/bobcobject.o \
$(OBJDIR)/bobdebug.o \
$(OBJDIR)/bobenter.o \
$(OBJDIR)/bobenv.o \
$(OBJDIR)/boberror.o \
$(OBJDIR)/bobfcn.o \
$(OBJDIR)/bobfile.o \
$(OBJDIR)/bobfloat.o \
$(OBJDIR)/bobhash.o \
$(OBJDIR)/bobheap.o \
$(OBJDIR)/bobint.o \
$(OBJDIR)/bobinteger.o \
$(OBJDIR)/bobmath.o \
$(OBJDIR)/bobmethod.o \
$(OBJDIR)/bobobject.o \
$(OBJDIR)/bobparse.o \
$(OBJDIR)/bobrcode.o \
$(OBJDIR)/bobstdio.o \
$(OBJDIR)/bobstream.o \
$(OBJDIR)/bobstring.o \
$(OBJDIR)/bobsymbol.o \
$(OBJDIR)/bobtype.o \
$(OBJDIR)/bobvector.o

$(BOBINT_OBJS):	$(OBJDIR)%.o:	bobint%.c $(HDRS)
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBDIR)/libbobi.a:	$(BOBINT_OBJS)
	@$(AR) crs $(LIBDIR)/libbobi.a $(BOBINT_OBJS)

###############
# BOBMERGE
###############

BOBMERGE_OBJS=\
$(OBJDIR)/bobmerge.o

$(BINDIR)/bobmerge2:	$(BOBMERGE_OBJS) $(HDRS)
	$(CC) -o $@ $(BOBMERGE_OBJS)

$(BOBMERGE_OBJS):	$(OBJDIR)%.o:	util%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:	$(DIRS)
	rm -rf $(BINDIR)
	rm -rf $(LIBDIR)
	rm -rf $(OBJDIR)
	rm -f test/*.txt

.PHONY: test
test: all
	( cd test; ./all_tests.sh )
