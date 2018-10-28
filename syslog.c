/* SPDX-License-Identifier: ISC */
#include <getopt.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "syslogd.h"

static int facility = 1;
static int level = LOG_INFO;
static int flags = LOG_NDELAY | LOG_NOWAIT;
static const char *ident = "(shell)";

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "console", required_argument, NULL, 'c' },
	{ "facility", required_argument, NULL, 'f' },
	{ "level", required_argument, NULL, 'l' },
	{ "ident", required_argument, NULL, 'i' },
	{ NULL, 0, NULL, 0 },
};

static const char *shortopt = "hVcf:l:i:";

static const char *helptext =
"Usage: syslog [OPTION]... [STRING]...\n\n"
"Concatenate the given STRINGs and send a log message to the syslog daemon.\n"
"\n"
"The following OPTIONSs can be used:\n"
"  -f, --facility <facility>  Logging facilty name or numeric identifier.\n"
"  -l, --level <level>        Log level name or numeric identifier.\n"
"  -i, --ident <name>         Program name for log syslog message.\n"
"                             Default is \"%s\".\n\n"
"  -c, --console              Write to the console if opening the syslog\n"
"                             socket fails.\n\n"
"  -h, --help                 Print this help text and exit\n"
"  -V, --version              Print version information and exit\n\n";

static const char *version_string =
"syslog (usyslog) " PACKAGE_VERSION "\n"
"Copyright (C) 2018 David Oberhollenzer\n\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n";

static void usage(int status)
{
	const char *str;
	int i;

	if (status != EXIT_SUCCESS) {
		fputs("Try `syslog --help' for more information\n", stderr);
	} else {
		printf(helptext, ident);

		fputs("The following values can be used for --level:\n",
		      stdout);

		i = 0;
		while ((str = level_id_to_string(i)) != NULL) {
			printf("  %s (=%d)%s\n", str, i,
			       i == level ? ", set as default" : "");
			++i;
		}

		fputs("\nThe following values can be used for --facility:\n",
		      stdout);

		i = 0;
		while ((str = facility_id_to_string(i)) != NULL) {
			printf("  %s (=%d)%s\n", str, i,
			       i == facility ? ", set as default" : "");
			++i;
		}
	}

	exit(status);
}

static int readint(const char *str)
{
	int x = 0;

	if (!isdigit(*str))
		return -1;

	while (isdigit(*str))
		x = x * 10 + (*(str++)) - '0';

	return (*str == '\0') ? x : -1;
}

static void process_options(int argc, char **argv)
{
	int c;

	for (;;) {
		c = getopt_long(argc, argv, shortopt, options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			facility = readint(optarg);
			if (facility >= 0)
				break;
			facility = facility_id_from_string(optarg);
			if (facility < 0) {
				fprintf(stderr, "Unknown facility name '%s'\n",
					optarg);
				usage(EXIT_FAILURE);
			}
			break;
		case 'l':
			level = readint(optarg);
			if (level >= 0)
				break;
			level = level_id_from_string(optarg);
			if (level < 0) {
				fprintf(stderr, "Unknown log level '%s'\n",
					optarg);
				usage(EXIT_FAILURE);
			}
			break;
		case 'i':
			ident = optarg;
			break;
		case 'c':
			flags |= LOG_CONS;
			break;
		case 'V':
			fputs(version_string, stdout);
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}
}


int main(int argc, char **argv)
{
	size_t len = 0;
	char *str;
	int i;

	process_options(argc, argv);

	if (optind >= argc) {
		fputs("Error: no log string provided.\n", stderr);
		usage(EXIT_FAILURE);
	}

	for (i = optind; i < argc; ++i)
		len += strlen(argv[i]);

	len += argc - optind - 1;

	str = calloc(1, len + 1);
	if (str == NULL) {
		fputs("syslog: out of memory\n", stderr);
		return EXIT_FAILURE;
	}

	for (i = optind; i < argc; ++i) {
		if (i > optind)
			strcat(str, " ");
		strcat(str, argv[i]);
	}

	openlog(ident, flags, facility << 3);
	syslog(level, "%s", str);
	closelog();

	free(str);
	return EXIT_SUCCESS;
}
