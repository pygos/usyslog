ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CFLAGS = $(WARN_CFLAGS)

usyslogd_SOURCES = syslogd.c syslogd.h proto.c logfile.c mksock.c protomap.c
klogd_SOURCES = klogd.c

bin_PROGRAMS =
sbin_PROGRAMS = usyslogd klogd
# EXTRA_DIST = README LICENSE
