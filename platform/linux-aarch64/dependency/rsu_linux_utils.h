/*
 * Copyright (C) 2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file rsu_linux_utils.h
 * @brief rsu linux utils helper.
 */

#ifndef RSU_LINUX_UTILS_H_
#define RSU_LINUX_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <RSU_OSAL_types.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>

#ifndef DEFAULT_RSU_DEV
#define DEFAULT_RSU_DEV "/sys/devices/platform/stratix10-rsu.0"
#endif

#define RSU_DEV_BUF_SIZE	(128U)
#define RSU_FILE_OP_BUF_SIZE	(256U)
#define NUM_ARGS		(16U)

RSU_OSAL_INT get_devattr(const RSU_OSAL_CHAR *rsu_dev, const RSU_OSAL_CHAR *attr, RSU_OSAL_U64 *const value);
RSU_OSAL_INT put_devattr(const RSU_OSAL_CHAR *rsu_dev, const RSU_OSAL_CHAR *attr, RSU_OSAL_U64 value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
