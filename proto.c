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
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "syslogd.h"

static const char *months[] = {
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
};

static const int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static int isleap(int year)
{
	return ((year % 4 == 0) && (year % 100 != 0)) || ((year % 400) == 0);
}

static int mdays(int year, int month)
{
	return (isleap(year) && month == 2) ? 29 : days[month - 1];
}

static char *read_num(char *str, int *out, int maxval)
{
	if (str == NULL || !isdigit(*str))
		return NULL;
	for (*out = 0; isdigit(*str); ++str) {
		(*out) = (*out) * 10 + (*str) - '0';
		if ((*out) > maxval)
			return NULL;
	}
	return str;
}

static char *skip_space(char *str)
{
	if (str == NULL || !isspace(*str))
		return NULL;
	while (isspace(*str))
		++str;
	return str;
}

static char *read_date_bsd(char *str, struct tm *tm)
{
	int year, month, day, hour, minute, second;
	time_t t;

	/* decode date */
	for (month = 0; month < 12; ++month) {
		if (strncmp(str, months[month], 3) == 0) {
			str = skip_space(str + 3);
			break;
		}
	}

	str = read_num(str, &day, 31);
	str = skip_space(str);

	t = time(NULL);
	if (localtime_r(&t, tm) == NULL)
		return NULL;

	year = tm->tm_year;

	/* sanity check */
	if (str == NULL || month >= 12 || day < 1)
		return NULL;
	if (month == 11 && tm->tm_mon == 0)
		--year;
	if (day > mdays(year + 1900, month + 1))
		return NULL;

	/* decode time */
	str = read_num(str, &hour, 23);
	if (str == NULL || *(str++) != ':')
		return NULL;
	str = read_num(str, &minute, 59);
	if (str == NULL || *(str++) != ':')
		return NULL;
	str = read_num(str, &second, 59);
	str = skip_space(str);

	/* store result */
	memset(tm, 0, sizeof(*tm));
	tm->tm_sec = second;
	tm->tm_min = minute;
	tm->tm_hour = hour;
	tm->tm_mday = day;
	tm->tm_mon = month;
	tm->tm_year = year;
	return str;
}

static char *decode_priority(char *str, int *priority)
{
	while (isspace(*str))
		++str;
	if (*(str++) != '<')
		return NULL;
	str = read_num(str, priority, 23 * 8 + 7);
	if (str == NULL || *(str++) != '>')
		return NULL;
	while (isspace(*str))
		++str;
	return str;
}

int syslog_msg_parse(syslog_msg_t *msg, char *str)
{
	char *ident, *ptr;
	struct tm tstamp;
	pid_t pid = 0;
	int priority;
	size_t len;

	memset(msg, 0, sizeof(*msg));

	str = decode_priority(str, &priority);
	if (str == NULL)
		return -1;

	msg->facility = priority >> 3;
	msg->level = priority & 0x07;

	str = read_date_bsd(str, &tstamp);
	if (str == NULL)
		return -1;

	ident = str;
	while (*str != '\0' && *str != ':')
		++str;

	if (*str == ':') {
		*(str++) = '\0';
		while (isspace(*str))
			++str;

		ptr = ident;
		while (*ptr != '[' && *ptr != '\0')
			++ptr;

		if (*ptr == '[') {
			*(ptr++) = '\0';

			while (isdigit(*ptr))
				pid = pid * 10 + *(ptr++) - '0';
		}
	} else {
		ident = NULL;
	}

	if (ident != NULL && ident[0] == '\0')
		ident = NULL;

	msg->timestamp = mktime(&tstamp);
	msg->pid = pid;
	msg->ident = ident;
	msg->message = str;

	len = strlen(str);
	while (len > 0 && isspace(str[len - 1]))
		--len;
	str[len] = '\0';

	if (ident != NULL) {
		for (ptr = ident; *ptr != '\0'; ++ptr) {
			if (!isalnum(*ptr))
				*ptr = '_';
		}
	}

	return 0;
}
