PREFIX = /usr
CFLAGS = -std=c99 -Wall -O2 -fstack-protector-strong -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 `pkg-config --cflags gio-2.0`
LDFLAGS = -s -Wl,-z,relro `pkg-config --libs gio-2.0`

sync: sync.c
	gcc $< -o $@ ${CFLAGS} ${LDFLAGS}
