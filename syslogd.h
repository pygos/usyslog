/* SPDX-License-Identifier: ISC */
#ifndef SYSLOGD_H
#define SYSLOGD_H


#include <sys/types.h>
#include <stdbool.h>
#include <time.h>

#include "config.h"


#define SYSLOG_SOCKET "/dev/log"
#define SYSLOG_PATH "/var/log"
#define DEFAULT_USER "syslogd"
#define DEFAULT_GROUP "syslogd"


/*
  encapsulates the split up data from a message received
  through the local syslog socket.
 */
typedef struct {
	int facility;
	int level;
	time_t timestamp;
	pid_t pid;
	const char *ident;
	const char *message;
} syslog_msg_t;


enum {
	/*
	  Rotate log data in a way that we still generate a continuous stream
	  of log data. E.g. in the case of log files, move the current log file
	  to one suffixed with a timestamp. We don't lose any log data.
	 */
	LOG_ROTATE_CONTINUOUS = 0x00,

	/*
	  Rotate log data by overwriting old data with more recent data.
	  E.g. in the case of log files, move the current log file to one
	  with a constant prefix, overwriting any existing data.
	 */
	LOG_ROTATE_OVERWRITE = 0x01,

	/*
	  Automatically do a log rotatation if a log stream reaches a preset
	  size limit.
	 */
	LOG_ROTATE_SIZE_LIMIT = 0x10,
};

typedef struct log_backend_t {
	int (*init)(struct log_backend_t *log, int flags, size_t sizelimit);

	void (*cleanup)(struct log_backend_t *log);

	int (*write)(struct log_backend_t *log, const syslog_msg_t *msg);

	void (*rotate)(struct log_backend_t *log);
} log_backend_t;


extern log_backend_t *logmgr;

/*
  Parse a message string received from the syslog socket and produce
  a split up representation for the message.
 */
int syslog_msg_parse(syslog_msg_t *msg, char *str);

/* Create a unix DGRAM socket. */
int mksock(const char *path);

const char *level_id_to_string(int level);

const char *facility_id_to_string(int level);

#endif /* LOGFILE_H */
