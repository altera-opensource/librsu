/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <hal/RSU_plat_file.h>
#include <utils/RSU_logging.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

RSU_OSAL_FILE *plat_filesys_open(RSU_OSAL_CHAR *filename, RSU_filesys_flags_t flag)
{
	if (!filename) {
		RSU_LOG_ERR("invalid argument\n");
		return NULL;
	}

	RSU_OSAL_FILE *file;

	errno = 0;

	if (flag == RSU_FILE_READ) {
		file = fopen(filename, "r");
	} else if (flag == RSU_FILE_WRITE) {
		file = fopen(filename, "w+");
	} else if (flag == RSU_FILE_APPEND) {
		file = fopen(filename, "a+");
	} else {
		RSU_LOG_ERR("invalid argument in flag\n");
		return NULL;
	}

	if (errno) {
		RSU_LOG_ERR("error in opening file with error : %d\n", errno);
		return NULL;
	}

	return file;
}

RSU_OSAL_INT plat_filesys_read(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file)
{

	if (!buf || len <= 0 || !file) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = fread(buf, 1, len, file);
	if (ret < 0) {
		RSU_LOG_ERR("error in reading the file %d", errno);
	}
	return ret;
}

RSU_OSAL_INT plat_filesys_write(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file)
{
	if (!buf || len <= 0 || !file) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = fwrite(buf, 1, len, file);
	if (ret < 0) {
		RSU_LOG_ERR("error in writing to file %d", errno);
	}
	return ret;
}

RSU_OSAL_INT plat_filesys_fgets(RSU_OSAL_CHAR *str, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file)
{
	if (str == NULL || len == 0 || file == NULL) {
		return -EINVAL;
	}
	RSU_OSAL_INT ret = 0;
	RSU_OSAL_CHAR *ptr;
	errno = 0;
	ptr = fgets(str, len, file);
	if (ptr == NULL && errno == 0) {
		return 1;
	} else if (errno != 0) {
		return -errno;
	} else {
		return 0;
	}
}

RSU_OSAL_INT plat_filesys_fseek(RSU_OSAL_OFFSET offset, RSU_filesys_whence_t whence,
				     RSU_OSAL_FILE *file)
{
	if (!file) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = 0;
	if (whence == RSU_SEEK_SET) {
		ret = fseek(file, offset, SEEK_SET);
	} else if (whence == RSU_SEEK_CUR) {
		ret = fseek(file, offset, SEEK_CUR);
	} else if (whence == RSU_SEEK_END) {
		ret = fseek(file, offset, SEEK_END);
	} else {
		RSU_LOG_ERR("invalid whence %d\n", whence);
		return -EINVAL;
	}

	if (errno) {
		RSU_LOG_ERR("error in fseek : %s",strerror(errno));
	}

	return ret;
}

RSU_OSAL_INT plat_filesys_ftruncate(RSU_OSAL_OFFSET length, RSU_OSAL_FILE *file)
{
	if (!file) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = ftruncate(fileno(file), length);
	if (ret) {
		RSU_LOG_ERR("error in closing the ftruncate %d", ret);
	}
	return ret;
}

RSU_OSAL_INT plat_filesys_close(RSU_OSAL_FILE *file)
{
	if (!file) {
		return -EINVAL;
	}
	RSU_OSAL_INT ret = fclose(file);
	if (ret) {
		RSU_LOG_ERR("error in closing the file %d", ret);
		return ret;
	}
	return ret;
}

RSU_OSAL_INT plat_filesys_terminate(RSU_OSAL_VOID)
{
	return 0;
}

/* Init API */
RSU_OSAL_INT plat_filesys_init(struct filesys_ll_intf *filesys_intf)
{
	if (!filesys_intf) {
		return -EINVAL;
	}
	filesys_intf->open = plat_filesys_open;
	filesys_intf->read = plat_filesys_read;
	filesys_intf->fgets = plat_filesys_fgets;
	filesys_intf->write = plat_filesys_write;
	filesys_intf->fseek = plat_filesys_fseek;
	filesys_intf->ftruncate = plat_filesys_ftruncate;
	filesys_intf->close = plat_filesys_close;
	filesys_intf->terminate = plat_filesys_terminate;
	return 0;
}
