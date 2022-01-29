INCDIR=../include
LIBDIR=../lib
OBJDIR=./obj

DIRS=$(LIBDIR) $(OBJDIR)

CFLAGS=-I$(INCDIR) # -DBOB_INCLUDE_FLOAT_SUPPORT

all:	$(DIRS) $(LIBDIR)/libbobi.a

$(DIRS):
	mkdir $(@F)

OBJS=\
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
$(OBJDIR)/bobmatrix.o \
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

$(OBJS):	$(OBJDIR)%.o:	.%.c $(INCDIR)/bob.h
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBDIR)/libbobi.a:	$(OBJS)
	@$(AR) crs $(LIBDIR)/libbobi.a $(OBJS)

clean:	$(DIRS)
	rm -f $(LIBDIR)/libbobi.a
	rm -f $(OBJDIR)/*.o
