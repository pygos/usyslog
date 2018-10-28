/* SPDX-License-Identifier: ISC */
#include "syslogd.h"

#include <string.h>

typedef struct {
	const char *name;
	int id;
} enum_map_t;

static const enum_map_t levels[] = {
	{ "emergency", 0 },
	{ "alert", 1 },
	{ "critical", 2 },
	{ "error", 3 },
	{ "warning", 4 },
	{ "notice", 5 },
	{ "info", 6 },
	{ "debug", 7 },
	{ NULL, 0 },
};

static const enum_map_t facilities[] = {
	{ "kernel", 0 },
	{ "user", 1 },
	{ "mail", 2 },
	{ "daemon", 3 },
	{ "auth", 4 },
	{ "syslog", 5 },
	{ "lpr", 6 },
	{ "news", 7 },
	{ "uucp", 8 },
	{ "clock", 9 },
	{ "authpriv", 10 },
	{ "ftp", 11 },
	{ "ntp", 12 },
	{ "audit", 13 },
	{ "alert", 14 },
	{ "cron", 15 },
	{ "local0", 16 },
	{ "local1", 17 },
	{ "local2", 18 },
	{ "local3", 19 },
	{ "local4", 20 },
	{ "local5", 21 },
	{ "local6", 22 },
	{ "local7", 23 },
	{ NULL, 0 },
};

static const char *enum_to_name(const enum_map_t *map, int id)
{
	while (map->name != NULL && map->id != id)
		++map;

	return map->name;
}

static int enum_by_name(const enum_map_t *map, const char *name)
{
	while (map->name != NULL && strcmp(map->name, name) != 0)
		++map;

	return map->name == NULL ? -1 : map->id;
}

const char *level_id_to_string(int level)
{
	return enum_to_name(levels, level);
}

const char *facility_id_to_string(int id)
{
	return enum_to_name(facilities, id);
}

int level_id_from_string(const char *level)
{
	return enum_by_name(levels, level);
}

int facility_id_from_string(const char *fac)
{
	return enum_by_name(facilities, fac);
}
