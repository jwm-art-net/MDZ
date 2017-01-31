VERSION :=0.1.2

PREFIX :=/usr/local
BINDIR :=$(PREFIX)/bin

PROG := mdz

CC := gcc

#DEFS := -Wall -Wextra -pedantic -std=gnu99 -DVERSION=\"$(VERSION)\"
DEFS := -Wall -Wextra -std=gnu99  -DVERSION=\"$(VERSION)\"

CFLAGS := $(DEFS) -O3 -fomit-frame-pointer -Winline -DNDEBUG

#Other CFLAGS options:
#CFLAGS := $(DEFS) -O3 -fomit-frame-pointer -Winline -DWITH_TMZ_CMDLINE
CFLAGS := $(DEFS) -ggdb -DRTH_NDEBUG


LIBS   := -lm -lpng -lz -lgmp -lmpfr -lpthread

SRC    := $(wildcard *.c)
OBJS   := $(patsubst %.c, %.o, $(SRC))
HEADERS:= $(wildcard *.h)

DISTDIR :=../mdz-$(VERSION)

PIXMAPS:= $(wildcard ../pixmaps/*.xpm)
GALLERY:= $(wildcard ../gallery/*.mdz)

SRC_DIR:=$(abspath $(lastword $(MAKEFILE_LIST)))

$(PROG): $(OBJS)
	$(CC) `pkg-config --cflags gtk+-2.0` $(CFLAGS) -DSRC_DIR=\"$(SRC_DIR)\" $(OBJS) -o $(PROG) \
	`pkg-config --libs gtk+-2.0` $(LIBS)
	
%.o : %.c $(HEADERS)
	$(CC) `pkg-config --cflags gtk+-2.0` $(CFLAGS) -DSRC_DIR=\"$(SRC_DIR)\" -c $<

main.o: main.c $(HEADERS)

clean:
	rm -f $(PROG) $(OBJS)

install: $(PROG)
	install $(PROG) $(BINDIR)

dist:
	mkdir $(DISTDIR)
	cp ../AUTHORS $(DISTDIR)/
	cp ../BUGS $(DISTDIR)/
	cp ../CHANGES $(DISTDIR)/
	cp ../COPYING $(DISTDIR)/
	cp ../INSTALL $(DISTDIR)/
	cp ../Makefile $(DISTDIR)/
	cp ../NEWS $(DISTDIR)/
	cp ../README $(DISTDIR)/
	cp ../THANKS $(DISTDIR)/
	cp ../TODO $(DISTDIR)/
	mkdir $(DISTDIR)/gallery
	cp $(GALLERY) $(DISTDIR)/gallery/
	mkdir $(DISTDIR)/pixmaps
	cp $(PIXMAPS) $(DISTDIR)/pixmaps/
	mkdir $(DISTDIR)/src
	cp $(HEADERS) $(DISTDIR)/src/
	cp $(SRC) $(DISTDIR)/src/
	cp Makefile $(DISTDIR)/src/
	tar -cvjf ../mdz-$(VERSION).tar.bz2 $(DISTDIR)
	rm -rf $(DISTDIR)
