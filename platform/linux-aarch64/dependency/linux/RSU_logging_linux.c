/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU_OSAL.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <string.h>
#include <stdarg.h>

typedef enum {
	RSU_STDERR,
	RSU_FILE
} rsu_log_type_t;

rsu_loglevel_t rsu_curr_loglevel = L_LOG_LVL;
rsu_log_type_t rsu_log_type = RSU_STDERR;
RSU_OSAL_FILE *RSU_log_file = NULL;

#define NUM_ARGS		(16U)
#define RSU_DEV_BUF_SIZE	(128U)

RSU_OSAL_INT RSU_set_logging(rsu_loglevel_t level)
{
	if (level >= L_LOG_MAX) {
		RSU_LOG_ERR("wrong log level provided : %d", level);
		return -EINVAL;
	}
	rsu_curr_loglevel = level;
	return 0;
}

RSU_OSAL_INT RSU_logging_init(RSU_OSAL_CHAR *cfg_file)
{
	if (cfg_file == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_FILE *file;
	RSU_OSAL_CHAR line[RSU_DEV_BUF_SIZE] = {0}, *argv[NUM_ARGS] = {0};
	RSU_OSAL_INT argc;
	RSU_OSAL_U32 linenum;

	file = fopen(cfg_file, "r");
	if (!file) {
		return -ENOENT;
	}

	linenum = 0;
	while (fgets(line, RSU_DEV_BUF_SIZE, file) != NULL) {
		linenum++;
		argc = split_line(line, argv, NUM_ARGS);
		if (argc != 3) {
			continue;
		}
		/*TODO:check if we need to check the return from RSU_set_logging()*/
		if (strcmp(argv[0], "log") == 0) {
			if (strcmp(argv[1], "off") == 0) {
				RSU_set_logging(L_LOG_OFF);
				fclose(file);
				return 0;
			} else if (strcmp(argv[1], "err") == 0) {
				RSU_set_logging(L_LOG_ERR);
			} else if (strcmp(argv[1], "low") == 0) {
				RSU_set_logging(L_LOG_WRN);
			} else if (strcmp(argv[1], "med") == 0) {
				RSU_set_logging(L_LOG_INF);
			} else if (strcmp(argv[1], "high") == 0) {
				RSU_set_logging(L_LOG_DBG);
			} else {
				RSU_LOG_ERR("Error in getting logging configuration at line number "
					    "%u\n",
					    linenum);
				RSU_LOG_ERR("Setting log level to med and output as stderr\n");
				RSU_set_logging(L_LOG_INF);
				rsu_log_type = RSU_STDERR;
				fclose(file);
				return 0;
			}

			if (strcmp(argv[2], "stderr") == 0) {
				rsu_log_type = RSU_STDERR;
			} else {
				RSU_OSAL_FILE *tfile = fopen(argv[2], "w");
				if (tfile == NULL) {
					RSU_LOG_ERR("Error in opening logfile, defined ar %u",
						    linenum);
					fclose(file);
					return -EINVAL;
				}
				rsu_log_type = RSU_FILE;
				RSU_log_file = tfile;
			}
			fclose(file);
			return 0;
		}
	}
	fclose(file);
	return 0;
}

RSU_OSAL_VOID RSU_logging_exit(RSU_OSAL_VOID)
{
	if (rsu_log_type == RSU_FILE) {
		fflush(RSU_log_file);
		fclose(RSU_log_file);
		RSU_log_file = NULL;
	}
}

RSU_OSAL_VOID RSU_logger(rsu_loglevel_t level, const RSU_OSAL_CHAR *format, ...)
{
	const RSU_OSAL_CHAR *l_log[] = {"", "err:", "low:", "med:", "high:"};
	va_list arg;

	if (level == L_LOG_OFF || level > rsu_curr_loglevel) {
		return;
	}

	if (rsu_log_type == RSU_STDERR) {
		fprintf(stderr, "\n[%s]", l_log[level]);
		fflush(stderr);
		va_start(arg, format);
		vfprintf(stderr, format, arg);
		va_end(arg);
		fprintf(stderr, "\n");
		fflush(stderr);
	} else if (rsu_log_type == RSU_FILE) {
		if (!RSU_log_file) {
			return;
		}
		fprintf(RSU_log_file, "\n[%s]", l_log[level]);
		fflush(RSU_log_file);
		va_start(arg, format);
		vfprintf(RSU_log_file, format, arg);
		va_end(arg);
		fprintf(RSU_log_file, "\n");
		fflush(RSU_log_file);
	}
}
