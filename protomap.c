/* SPDX-License-Identifier: ISC */
#include "syslogd.h"

#include <string.h>

static const char *levels[] = {
	"emergency",
	"alert",
	"critical",
	"error",
	"warning",
	"notice",
	"info",
	"debug",
};

static const char *facilities[] = {
	"kernel",
	"user",
	"mail",
	"daemon",
	"auth",
	"syslog",
	"lpr",
	"news",
	"uucp",
	"clock",
	"authpriv",
	"ftp",
	"ntp",
	"audit",
	"alert",
	"cron",
	"local0",
	"local1",
	"local2",
	"local3",
	"local4",
	"local5",
	"local6",
	"local7",
};

const char *level_id_to_string(int level)
{
	return (level < 0 || level > 7) ? NULL : levels[level];
}

const char *facility_id_to_string(int id)
{
	return (id < 0 || id > 23) ? NULL : facilities[id];
}

int level_id_from_string(const char *level)
{
	size_t i;
	for (i = 0; i < sizeof(levels) / sizeof(levels[0]); ++i) {
		if (strcmp(level, levels[i]) == 0)
			return i;
	}
	return -1;
}

int facility_id_from_string(const char *fac)
{
	size_t i;
	for (i = 0; i < sizeof(facilities) / sizeof(facilities[0]); ++i) {
		if (strcmp(fac, facilities[i]) == 0)
			return i;
	}
	return -1;
}
