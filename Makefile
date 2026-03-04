.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OBJ =\
	git-protection.o

HDR =

all: git-protection
$(OBJ): $(HDR)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

git-protection: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)
	$(BUILD_SUDO) sh -c 'chown -- 0:0 $@ && chmod -- 4755 $@'

install: git-protection
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1/"
	cp -- git-protection "$(DESTDIR)$(PREFIX)/bin/"
	cp -- git-protection.1 "$(DESTDIR)$(MANPREFIX)/man1/"

post-install:
	chown -- '0:0' "$(DESTDIR)$(PREFIX)/bin/git-protection"
	chmod -- 4755 "$(DESTDIR)$(PREFIX)/bin/git-protection"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/bin/git-protection"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man1/git-protection.1"

clean:
	-rm -f -- *.o *.a *.lo *.su *.so *.so.* *.gch *.gcov *.gcno *.gcda
	-rm -f -- git-protection

.SUFFIXES:
.SUFFIXES: .o .c

.PHONY: all install post-install uninstall clean
