/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Copyright (C) 2018 - David Oberhollenzer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "syslogd.h"

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

const char *level_id_to_string(int level)
{
	return enum_to_name(levels, level);
}

const char *facility_id_to_string(int level)
{
	return enum_to_name(facilities, level);
}
