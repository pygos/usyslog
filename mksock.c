/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "syslogd.h"

int mksock(const char *path)
{
	struct sockaddr_un un;
	const char *errmsg;
	int fd;

	fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, path);

	unlink(un.sun_path);

	if (bind(fd, (struct sockaddr *)&un, sizeof(un))) {
		errmsg ="bind";
		goto fail_errno;
	}

	if (chmod(path, 0777)) {
		errmsg = "chmod";
		goto fail_errno;
	}

	return fd;
fail_errno:
	fprintf(stderr, "%s: %s: %s\n", path, errmsg, strerror(errno));
	close(fd);
	unlink(path);
	return -1;
}
