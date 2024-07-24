/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU_cfg.h>
#include <libRSU_ll_intf.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <libRSU_ops.h>
#include <string.h>

#define NUM_ARGS		(16U)
#define RSU_DEV_BUF_SIZE	(128U)

static struct librsu_ll_intf *hal;

RSU_OSAL_INT librsu_cfg_parse(RSU_OSAL_CHAR *filename, struct librsu_hl_intf **intf)
{
	if (filename == NULL || intf == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret;

	hal = rsu_malloc(sizeof(struct librsu_ll_intf));
	if (hal == NULL) {
		RSU_LOG_ERR("Failed to allocate memory");
		return -ENOMEM;
	}

	rsu_memset(hal, 0, sizeof(struct librsu_ll_intf));

	ret = RSU_logging_init(filename);
	if (ret != 0) {
		RSU_LOG_ERR("error in setting log information");
		rsu_free(hal);
		hal = NULL;
		return -EINVAL;
	}

	ret = plat_filesys_init(&(hal->file));
	if (ret != 0) {
		RSU_logging_exit();
		rsu_free(hal);
		hal = NULL;
		return -ENXIO;
	}

	ret = plat_mbox_init(&(hal->mbox), filename);
	if (ret != 0) {
		RSU_LOG_ERR("Error during initializing mailbox API");
		ret = hal->file.terminate();
		if (ret) {
			RSU_LOG_ERR("Error in terminating file system");
		}
		RSU_logging_exit();
		rsu_free(hal);
		hal = NULL;
		return -ENXIO;
	}

	ret = plat_qspi_init(&(hal->qspi), filename);
	if (ret != 0) {
		RSU_LOG_ERR("Error during initializing QSPI API");
		ret = hal->mbox.terminate();
		if (ret) {
			RSU_LOG_ERR("Error in terminating mailbox");
		}
		ret = hal->file.terminate();
		if (ret) {
			RSU_LOG_ERR("Error in terminating file system");
		}
		RSU_logging_exit();
		rsu_free(hal);
		hal = NULL;
		return -ENXIO;
	}

	ret = plat_rsu_misc_init(&(hal->misc), filename);
	if (ret != 0) {
		RSU_LOG_ERR("Error during initializing misc API");
		ret = hal->mbox.terminate();
		if (ret) {
			RSU_LOG_ERR("Error in terminating mailbox");
		}
		ret = hal->file.terminate();
		if (ret) {
			RSU_LOG_ERR("Error in terminating file system");
		}
		ret = hal->qspi.terminate();
		if (ret) {
			RSU_LOG_ERR("Error in terminating qspi system");
		}
		RSU_logging_exit();
		rsu_free(hal);
		hal = NULL;
		return -ENXIO;
	}

	ret = librsu_common_cfg_parse(filename, hal);
	if (ret != 0) {
		RSU_LOG_ERR("error in setting common configuration");
	}

	RSU_LOG_DBG("Platform initialization completed");

	ret = rsu_qspi_open(hal, intf);
	if (ret) {
		RSU_LOG_ERR("Error in opening RSU in qspi %d", ret);
		librsu_cfg_reset();
		return -ENODEV;
	}

	return 0;
}

RSU_OSAL_INT librsu_common_cfg_parse(RSU_OSAL_CHAR *filename, struct librsu_ll_intf *intf)
{
	if (filename == NULL || intf == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_FILE *file;
	RSU_OSAL_CHAR line[RSU_DEV_BUF_SIZE], *argv[NUM_ARGS] = {0};
	RSU_OSAL_INT argc, linenum;
	RSU_OSAL_U32 slot;

	intf->spt_checksum_enabled = 1; /*spt_checksum_enabled is enabled by default*/

	file = intf->file.open(filename, RSU_FILE_READ);
	if (!file) {
		RSU_LOG_ERR("Error in opening configuration file");
		return -ENOENT;
	}

	linenum = 0;
	while (intf->file.fgets(line, RSU_DEV_BUF_SIZE, file) == 0) {
		linenum++;
		argc = split_line(line, argv, NUM_ARGS);

		if (argc < 1)
			continue;

		if (argv[0][0] == '#')
			continue;

		if (strcmp(argv[0], "write-protect") == 0) {
			if (argc != 2) {
				RSU_LOG_ERR("error: Wrong number of parameters for '%s' @%i", argv[0],
					linenum);
				intf->file.close(file);
				return -EINVAL;
			}
			slot = strtoul(argv[1], NULL, 10);
			if (slot > 31) {
				RSU_LOG_ERR("error: Write protection only works on first 32 slots @%i",
					linenum);
				intf->file.close(file);
				return -EINVAL;
			}
			intf->writeprotect |= (1 << slot);
		} else if (strcmp(argv[0], "rsu-spt-checksum") == 0) {
			if (argc != 2) {
				RSU_LOG_ERR("Wrong number of parameters for '%s' @%i", argv[0],
					linenum);
				intf->file.close(file);
				return -EINVAL;
			}
			intf->spt_checksum_enabled = strtoul(argv[1], NULL, 10);
		}
	}
	intf->file.close(file);
	return 0;
}

RSU_OSAL_VOID librsu_cfg_reset(RSU_OSAL_VOID)
{
	hal->mbox.terminate();
	hal->qspi.terminate();
	hal->file.terminate();
	hal->misc.terminate();
	RSU_logging_exit();
	rsu_free(hal);
	hal = NULL;
}

struct librsu_ll_intf *librsu_get_ll_inf(RSU_OSAL_VOID)
{
	return hal;
}

RSU_OSAL_INT librsu_cfg_writeprotected(RSU_OSAL_INT slot)
{
	if (slot < 0) {
		return -1;
	}

	if (slot > 31) {
		return 0;
	}

	if (hal->writeprotect & (1 << slot)) {
		return 1;
	}

	return 0;
}

RSU_OSAL_INT librsu_cfg_spt_checksum_enabled(RSU_OSAL_VOID)
{

	if (hal->spt_checksum_enabled) {
		return 1;
	}

	return 0;
}
