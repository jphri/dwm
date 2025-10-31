# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

OBJ   = src/drw.o src/dwm.o src/util.o src/config.o src/parse_key.o src/tags.o src/layout.o src/movestack.o
ENTRY = src/main.o

TESTS=\
	test_parse_buttonsymbol\
	test_parse_keysymbol

all: dwm ${TESTS}

test: ${TESTS}
	for f in ${TESTS}; do \
		./"$$f" ;\
	done

.c.o:
	${CC} -c ${CFLAGS} $< -o $@

${OBJ}: config.mk

dwm: ${OBJ} ${ENTRY}
	${CC} -o $@ $^ ${LDFLAGS}

test_%: src/test_%.c ${OBJ}
	${CC} -o $@ $^ ${LDFLAGS}

clean:
	rm -f ${TESTS}
	rm -f dwm ${OBJ} ${ENTRY} dwm-${VERSION}.tar.gz

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dwm.1 drw.h util.h ${SRC} dwm.png transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/man1/dwm.1

.PHONY: all clean dist install uninstall test
