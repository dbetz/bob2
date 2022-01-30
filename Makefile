BINDIR=./bin
LIBDIR=./lib
OBJDIR=./obj

DIRS=$(BINDIR) $(LIBDIR) $(OBJDIR)

PROGS=$(BINDIR)/bob2 $(BINDIR)/bobc2 $(BINDIR)/bobi2 $(BINDIR)/bobmerge2
LIBS=$(LIBDIR)/libbobc.a $(LIBDIR)/libbobi.a

CFLAGS=-I./include -I./bobcom -I./bobint -DBOB_INCLUDE_FLOAT_SUPPORT

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

$(BOBC_OBJS):	$(OBJDIR)%.o:	bobc%.c
	$(CC) -c $(CFLAGS) $< -o $@

###############
# BOBI
###############

BOBI_OBJS=\
$(OBJDIR)/bobi.o

$(BINDIR)/bobi2:	$(BOBI_OBJS) lib/libbobi.a
	$(CC) -o $@ $(BOBI_OBJS) -L$(LIBDIR) -lbobi -lm

$(BOBI_OBJS):	$(OBJDIR)%.o:	bobi%.c
	$(CC) -c $(CFLAGS) $< -o $@

###############
# BOBCOM
###############

$(LIBDIR)/libbobc.a:
	$(MAKE) -C bobcom

###############
# BOBINT
###############

$(LIBDIR)/libbobi.a:
	$(MAKE) -C bobint

###############
# BOBMERGE
###############

BOBMERGE_OBJS=\
$(OBJDIR)/bobmerge.o

$(BINDIR)/bobmerge2:	$(BOBMERGE_OBJS)
	$(CC) -o $@ $(BOBMERGE_OBJS)

$(BOBMERGE_OBJS):	$(OBJDIR)%.o:	util%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:	$(DIRS)
	rm -f $(BINDIR)/*
	rm -f $(OBJDIR)/*
	$(MAKE) -C bobcom clean
	$(MAKE) -C bobint clean
