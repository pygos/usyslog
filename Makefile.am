ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CFLAGS = $(WARN_CFLAGS)

usyslogd_SOURCES = syslogd.c syslogd.h proto.c logfile.c mksock.c protomap.c
klogd_SOURCES = klogd.c
syslog_SOURCES = syslog.c protomap.c

dist_man1_MANS = syslog.1
bin_PROGRAMS = syslog
sbin_PROGRAMS = usyslogd klogd
EXTRA_DIST = LICENSE README.md
