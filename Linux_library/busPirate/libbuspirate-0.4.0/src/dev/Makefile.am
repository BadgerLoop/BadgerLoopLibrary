AM_CFLAGS = -D__BUSPIRATE__

buspirate_demo_SOURCES = \
	24c02.c \
	24c02.h \
	24fc1025.c \
	24fc1025.h \
	cc2500.c \
	cc2500.h \
	ds1324.c \
	ds1324.h \
	enc28j60.c \
	enc28j60.h \
	ht1632.c \
	ht1632.h \
	pcd8544.c \
	pcd8544.h \
	demo.c
buspirate_demo_LDADD = ../libbuspirate.la

buspirate_demo24_SOURCES = \
	802_15_4.c \
	802_15_4.h \
	mrf24j40.c \
	mrf24j40.h \
	demo-mrf24j40.c
buspirate_demo24_LDADD = ../libbuspirate.la

bin_PROGRAMS = buspirate-demo buspirate-demo24

TESTS = \
	buspirate-demo
