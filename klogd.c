/* SPDX-License-Identifier: ISC */
#include <sys/klog.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

#include "config.h"

enum {
	KLOG_CLOSE = 0,
	KLOG_OPEN = 1,
	KLOG_READ = 2,
	KLOG_CONSOLE_OFF = 6,
	KLOG_CONSOLE_ON = 7,
	KLOG_CONSOLE_LEVEL = 8,
};

static char log_buffer[4096];
static sig_atomic_t running = 1;
static int level = 0;

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "level", required_argument, NULL, 'l' },
	{ NULL, 0, NULL, 0 },
};

static const char *shortopt = "hVl:";

static const char *helptext =
"Usage: klogd [OPTION]... \n\n"
"Collect printk() messages from the kernel and forward them to syslogd.\n"
"\n"
"The following OPTIONSs can be used:\n"
"  -l, --level <level>  Minimum log level that should be printed to console.\n"
"                       If not set, logging to console is turned off.\n"
"  -h, --help           Print this help text and exit\n"
"  -V, --version        Print version information and exit\n\n";

static const char *version_string =
"klogd (usyslog) " PACKAGE_VERSION "\n"
"Copyright (C) 2018 David Oberhollenzer\n\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n";

static void process_options(int argc, char **argv)
{
	int c;

	for (;;) {
		c = getopt_long(argc, argv, shortopt, options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'l':
			level = strtoul(optarg, NULL, 10);
			break;
		case 'h':
			fputs(helptext, stdout);
			exit(EXIT_SUCCESS);
		case 'V':
			fputs(version_string, stdout);
			exit(EXIT_SUCCESS);
		default:
			fputs("Try `klogd --help' for more information\n",
			      stderr);
			exit(EXIT_FAILURE);
		}
	}
}

static void sighandler(int signo)
{
	if (signo == SIGTERM || signo == SIGINT)
		running = 0;
}

static void sigsetup(void)
{
	struct sigaction act;
	sigset_t mask;

	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigprocmask(SIG_SETMASK, &mask, NULL);
}

static void log_open(void)
{
	klogctl(KLOG_OPEN, NULL, 0);

	if (level) {
		klogctl(KLOG_CONSOLE_LEVEL, NULL, level);
	} else {
		klogctl(KLOG_CONSOLE_OFF, NULL, 0);
	}

	openlog("kernel", 0, LOG_KERN);
}

static void log_close(void)
{
	klogctl(KLOG_CONSOLE_ON, NULL, 0);
	klogctl(KLOG_CLOSE, NULL, 0);
	syslog(LOG_NOTICE, "-- klogd terminating --");
}

int main(int argc, char **argv)
{
	int diff, count = 0, priority, ret = EXIT_SUCCESS;
	char *ptr, *end;

	process_options(argc, argv);
	sigsetup();
	log_open();

	while (running) {
		diff = klogctl(KLOG_READ, log_buffer + count,
			       sizeof(log_buffer) - 1 - count);

		if (diff < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_CRIT, "klogctl read: %s", strerror(errno));
			ret = EXIT_FAILURE;
			break;
		}

		count += diff;
		log_buffer[count] = '\0';
		ptr = log_buffer;

		for (;;) {
			end = strchr(ptr, '\n');
			if (end == NULL) {
				if (ptr != log_buffer) {
					count = strlen(ptr);
					memmove(log_buffer, ptr, count + 1);
				}
				break;
			}

			*(end++) = '\0';
			priority = LOG_INFO;

			if (*ptr == '<') {
				++ptr;
				if (*ptr)
					priority = strtoul(ptr, &ptr, 10);
				if (*ptr == '>')
					++ptr;
			}

			if (*ptr != '\0')
				syslog(priority, "%s", ptr);
			ptr = end;
		}
	}

	log_close();
	return ret;
}
