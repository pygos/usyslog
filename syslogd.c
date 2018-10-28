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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include "syslogd.h"

static const struct option long_opts[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "rotate-replace", no_argument, NULL, 'r' },
	{ "chroot", no_argument, NULL, 'c' },
	{ "max-size", required_argument, NULL, 'm' },
	{ "user", required_argument, NULL, 'u' },
	{ "group", required_argument, NULL, 'g' },
	{ NULL, 0, NULL, 0 },
};

static const char *short_opts = "hVcrm:u:g:";

const char *usage_string =
"Usage: usyslogd [OPTIONS..]\n\n"
"The following options are supported:\n"
"  -h, --help             Print this help text and exit\n"
"  -V, --version          Print version information and exit\n"
"  -r, --rotate-replace   Replace old log files when doing log rotation.\n"
"  -m, --max-size <size>  Automatically rotate log files bigger than this.\n"
"  -u, --user <name>      Run the syslog daemon as this user. If not set,\n"
"                         try to use the user '" DEFAULT_USER "'.\n"
"  -g, --group <name>     Run the syslog daemon as this group. If not set,\n"
"                         try to use the group '" DEFAULT_GROUP "'.\n"
"  -c, --chroot           If set, do a chroot into the log file path.\n";



static volatile sig_atomic_t syslog_run = 1;
static volatile sig_atomic_t syslog_rotate = 0;
static int log_flags = 0;
static size_t max_size = 0;
static uid_t uid = 0;
static gid_t gid = 0;
static bool dochroot = false;



static void sighandler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		syslog_run = 0;
		break;
	case SIGHUP:
		syslog_rotate = 1;
		break;
	default:
		break;
	}
}

static void signal_setup(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
}

static int handle_data(int fd)
{
	char buffer[2048];
	syslog_msg_t msg;
	ssize_t ret;

	memset(buffer, 0, sizeof(buffer));

	ret = read(fd, buffer, sizeof(buffer));
	if (ret <= 0)
		return -1;

	if (syslog_msg_parse(&msg, buffer))
		return -1;

	return logmgr->write(logmgr, &msg);
}

#define GPL_URL "https://gnu.org/licenses/gpl.html"

static const char *version_string =
"usyslogd (usyslog) " PACKAGE_VERSION "\n"
"Copyright (C) 2018 David Oberhollenzer\n\n"
"License GPLv3+: GNU GPL version 3 or later <" GPL_URL ">.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n";

static void process_options(int argc, char **argv)
{
	struct passwd *pw = getpwnam(DEFAULT_USER);
	struct group *grp = getgrnam(DEFAULT_GROUP);
	char *end;
	int i;

	if (pw != NULL)
		uid = pw->pw_uid;

	if (grp != NULL)
		gid = grp->gr_gid;

	for (;;) {
		i = getopt_long(argc, argv, short_opts, long_opts, NULL);
		if (i == -1)
			break;

		switch (i) {
		case 'r':
			log_flags |= LOG_ROTATE_OVERWRITE;
			break;
		case 'm':
			log_flags |= LOG_ROTATE_SIZE_LIMIT;
			max_size = strtol(optarg, &end, 10);
			if (max_size == 0 || *end != '\0') {
				fputs("Numeric argument > 0 expected for -m\n",
				      stderr);
				goto fail;
			}
			break;
		case 'u':
			pw = getpwnam(optarg);
			if (pw == NULL) {
				fprintf(stderr, "Cannot get UID for user %s\n",
					optarg);
				goto fail;
			}
			uid = pw->pw_uid;
			break;
		case 'g':
			grp = getgrnam(optarg);
			if (grp == NULL) {
				fprintf(stderr,
					"Cannot get GID for group %s\n",
					optarg);
				goto fail;
			}
			gid = grp->gr_gid;
			break;
		case 'c':
			dochroot = true;
			break;
		case 'h':
			fputs(usage_string, stdout);
			exit(EXIT_SUCCESS);
		case 'V':
			fputs(version_string, stdout);
			exit(EXIT_SUCCESS);
		default:
			goto fail;
		}
	}
	return;
fail:
	fputs("Try `usyslogd --help' for more information\n", stderr);
	exit(EXIT_FAILURE);
}

static int chroot_setup(void)
{
	if (mkdir(SYSLOG_PATH, 0750)) {
		if (errno != EEXIST) {
			perror("mkdir " SYSLOG_PATH);
			return -1;
		}
	}

	if (uid > 0 && gid > 0 && chown(SYSLOG_PATH, uid, gid) != 0) {
		perror("chown " SYSLOG_PATH);
		return -1;
	}

	if (chmod(SYSLOG_PATH, 0750)) {
		perror("chmod " SYSLOG_PATH);
		return -1;
	}

	if (chdir(SYSLOG_PATH)) {
		perror("cd " SYSLOG_PATH);
		return -1;
	}

	if (dochroot && chroot(SYSLOG_PATH) != 0) {
		perror("chroot " SYSLOG_PATH);
		return -1;
	}

	return 0;
}

static int user_setup(void)
{
	if (gid > 0 && setresgid(gid, gid, gid) != 0) {
		perror("setgid");
		return -1;
	}
	if (uid > 0 && setresuid(uid, uid, uid) != 0) {
		perror("setuid");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int sfd, status = EXIT_FAILURE;

	process_options(argc, argv);

	signal_setup();

	sfd = mksock(SYSLOG_SOCKET);
	if (sfd < 0)
		return EXIT_FAILURE;

	if (uid > 0 && gid > 0 && chown(SYSLOG_SOCKET, uid, gid) != 0) {
		perror("chown " SYSLOG_SOCKET);
		return -1;
	}

	if (chroot_setup())
		return EXIT_FAILURE;

	if (user_setup())
		return EXIT_FAILURE;

	if (logmgr->init(logmgr, log_flags, max_size))
		goto out;

	while (syslog_run) {
		if (syslog_rotate) {
			logmgr->rotate(logmgr);
			syslog_rotate = 0;
		}

		handle_data(sfd);
	}

	status = EXIT_SUCCESS;
out:
	logmgr->cleanup(logmgr);
	if (sfd > 0)
		close(sfd);
	unlink(SYSLOG_SOCKET);
	return status;
}
