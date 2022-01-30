INCDIR=../include
LIBDIR=../lib
OBJDIR=./obj

DIRS=$(LIBDIR) $(OBJDIR)

CFLAGS=-I$(INCDIR) -DBOB_INCLUDE_FLOAT_SUPPORT

all:	$(DIRS) $(LIBDIR)/libbobc.a

$(DIRS):
	mkdir $(@F)

OBJS=\
$(OBJDIR)/bobcom.o \
$(OBJDIR)/bobeval.o \
$(OBJDIR)/bobscn.o \
$(OBJDIR)/bobwcode.o

$(OBJS):	$(OBJDIR)%.o:	.%.c $(INCDIR)/bob.h
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBDIR)/libbobc.a:	$(OBJS)
	@$(AR) crs $(LIBDIR)/libbobc.a $(OBJS)

clean:	$(DIRS)
	rm -f $(LIBDIR)/libbobc.a
	rm -f $(OBJDIR)/*.o
