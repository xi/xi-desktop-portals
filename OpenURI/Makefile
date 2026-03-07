PREFIX = /usr
CFLAGS = -std=c99 -Wall -O2 -fstack-protector-strong -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 `pkg-config --cflags gio-2.0`
LDFLAGS = -s -Wl,-z,relro `pkg-config --libs gio-2.0`

all: open-uri

open-uri: open-uri.c
	gcc $< -o $@ ${CFLAGS} ${LDFLAGS}

clean:
	rm -f open-uri

install: all
	install -D -m 755 xiterm ${DESTDIR}${PREFIX}/bin/xi-portal-open-uri

.PHONY: all clean install
