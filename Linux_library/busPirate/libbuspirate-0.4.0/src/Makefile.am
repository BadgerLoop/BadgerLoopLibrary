SUBDIRS = . dev

libbuspirate_la_SOURCES = \
	buspirate.c \
	buspirate.h \
	serial.c \
	serial.h \
	i2c.c \
	i2c.h \
	spi.c \
	spi.h \
	raw.c \
	raw.h
libbuspirateinclude_HEADERS = \
	buspirate.h \
	debug.h \
	i2c.h \
	spi.h \
	raw.h
libbuspirateincludedir = $(includedir)/buspirate

lib_LTLIBRARIES = libbuspirate.la
libbuspirate_la_LDFLAGS = -no-undefined -lm

AM_CFLAGS = -D__BUSPIRATE__

