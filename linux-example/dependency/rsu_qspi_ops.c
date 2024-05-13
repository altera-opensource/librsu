/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include "rsu_linux_utils.h"
#include <hal/RSU_plat_qspi.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mtd/mtd-user.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEVICE "/dev/mtd0"

static int dev_file = -1;
static struct mtd_info_user dev_info;
static char qspi_file[RSU_DEV_BUF_SIZE] = DEVICE;

RSU_OSAL_INT plat_qspi_read(RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *data, RSU_OSAL_SIZE len)
{
	RSU_LOG_DBG("received read qspi");
	int cnt = 0;
	char *ptr = data;
	int rtn;

	if (dev_file < 0) {
		return -EINVAL;
	}

	if (lseek(dev_file, offset, SEEK_SET) != offset) {
		return -EINVAL;
	}

	while (cnt < len) {
		rtn = read(dev_file, ptr, len - cnt);

		if (rtn < 0) {
			RSU_LOG_ERR("error: Read error (errno=%i)", errno);
			return rtn;
		}

		cnt += rtn;
		ptr += rtn;
	}

	return 0;
}

RSU_OSAL_INT plat_qspi_write(RSU_OSAL_OFFSET offset, const RSU_OSAL_VOID *data, RSU_OSAL_SIZE len)
{
	RSU_LOG_DBG("received write qspi");

	int cnt = 0;
	char *ptr = (char *)data;
	int rtn;

	if (dev_file < 0) {
		return -EINVAL;
	}

	if (lseek(dev_file, offset, SEEK_SET) != offset) {
		return -EINVAL;
	}

	while (cnt < len) {
		rtn = write(dev_file, ptr, len - cnt);

		if (rtn < 0) {
			RSU_LOG_ERR("error: Write error (errno=%i)", errno);
			return rtn;
		}

		cnt += rtn;
		ptr += rtn;
	}

	return 0;
}

RSU_OSAL_INT plat_qspi_erase(RSU_OSAL_OFFSET offset, RSU_OSAL_SIZE len)
{
	struct erase_info_user erase;
	int rtn;

	RSU_LOG_DBG("received erase info ");

	if (dev_file < 0) {
		return -EIO;
	}

	if (offset % dev_info.erasesize) {
		RSU_LOG_ERR("error: Erase offset 0x08%jx not erase block aligned", offset);
		return -ECANCELED;
	}

	if (len % dev_info.erasesize) {
		RSU_LOG_ERR("error: Erase length %lu not erase block aligned", len);
		return -EPERM;
	}

	erase.start = offset;
	erase.length = len;

	rtn = ioctl(dev_file, MEMERASE, &erase);

	if (rtn < 0) {
		RSU_LOG_ERR("error: Erase error (errno=%i)", errno);
		return -EFAULT;
	}

	return 0;
}

RSU_OSAL_INT plat_qspi_terminate(RSU_OSAL_VOID)
{
	close(dev_file);
	dev_file = -1;
	RSU_LOG_DBG("qspi terminated");
	return 0;
}

/* Init plat_qspi */
RSU_OSAL_INT plat_qspi_init(struct qspi_ll_intf *qspi_intf, RSU_OSAL_CHAR *config_file)
{
	if (qspi_intf == NULL || config_file == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_CHAR *type_str;
	RSU_OSAL_FILE *file;
	RSU_OSAL_CHAR line[RSU_DEV_BUF_SIZE], *argv[NUM_ARGS];
	RSU_OSAL_INT argc;
	RSU_OSAL_U32 linenum;

	file = fopen(config_file, "r");
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

		if (strcmp(argv[0], "root") == 0) {
			if (strncmp(argv[1], "qspi", RSU_DEV_BUF_SIZE) == 0) {
				strncpy(qspi_file, argv[2], RSU_DEV_BUF_SIZE);
			} else {
				RSU_LOG_ERR("root device not qspi at linenum %d\n", linenum);
			}
			break;
		}
	}

	fclose(file);

	dev_file = open(qspi_file, O_RDWR | O_SYNC);
	if (dev_file < 0) {
		RSU_LOG_ERR("Unable to open '%s'\n", qspi_file);
		strncpy(qspi_file, DEVICE, RSU_DEV_BUF_SIZE);
		RSU_LOG_ERR("Trying to open '%s'\n", qspi_file);
		dev_file = open(qspi_file, O_RDWR | O_SYNC);
		if (dev_file < 0) {
			RSU_LOG_ERR("Unable to open '%s'\n", qspi_file);
			return -ENODEV;
		}
	}
	RSU_LOG_DBG("root is %.*s for qspi operations\n", RSU_DEV_BUF_SIZE, qspi_file);

	if (ioctl(dev_file, MEMGETINFO, &dev_info)) {
		RSU_LOG_ERR("error: Unable to find mtd info for '%s'\n", DEVICE);
		close(dev_file);
		return -EACCES;
	}

	switch (dev_info.type) {
	case MTD_NORFLASH:
		type_str = "NORFLASH";
		break;
	case MTD_NANDFLASH:
		type_str = "NANDFLASH";
		break;
	case MTD_RAM:
		type_str = "RAM";
		break;
	case MTD_ROM:
		type_str = "ROM";
		break;
	case MTD_DATAFLASH:
		type_str = "DATAFLASH";
		break;
	case MTD_UBIVOLUME:
		type_str = "UBIVOLUME";
		break;
	default:
		type_str = "[UNKNOWN]";
		break;
	};

	RSU_LOG_DBG("MTD flash type is (%i) %s", dev_info.type, type_str);
	RSU_LOG_DBG("MTD flash size = %i", dev_info.size);
	RSU_LOG_DBG("MTD flash erase size = %i", dev_info.erasesize);
	RSU_LOG_DBG("MTD flash write size = %i", dev_info.writesize);

	if (dev_info.flags & MTD_WRITEABLE) {
		RSU_LOG_DBG("MTD flash is MTD_WRITEABLE");
	}

	if (dev_info.flags & MTD_BIT_WRITEABLE) {
		RSU_LOG_DBG("MTD flash is MTD_BIT_WRITEABLE");
	}

	if (dev_info.flags & MTD_NO_ERASE) {
		RSU_LOG_DBG("MTD flash is MTD_NO_ERASE");
	}

	if (dev_info.flags & MTD_POWERUP_LOCK) {
		RSU_LOG_DBG("MTD flash is MTD_POWERUP_LOCK");
	}

	qspi_intf->read = plat_qspi_read;
	qspi_intf->write = plat_qspi_write;
	qspi_intf->erase = plat_qspi_erase;
	qspi_intf->terminate = plat_qspi_terminate;

	return 0;
}
