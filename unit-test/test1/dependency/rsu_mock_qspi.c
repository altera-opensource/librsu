/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <hal/RSU_plat_qspi.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Defined macro for bufferlength */
#define bufferlength (100 * 1024)
/* Static Array for mocking */
static uint8_t g_arr[bufferlength];

/* Mocking function for plat_qspi_read */
RSU_OSAL_INT plat_qspi_read_mock(RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *data, RSU_OSAL_SIZE len)
{
	RSU_LOG_INF("received read qspi");
	uint8_t *ret;
	ret = (uint8_t *)data;
	uint32_t i = 0;
	for (i = 0; i < len; i++) {
		ret[i] = g_arr[i + offset];
	}

	return 0;
}

/* Mocking function for plat_qspi_write */
RSU_OSAL_INT plat_qspi_write_mock(RSU_OSAL_OFFSET offset, const RSU_OSAL_VOID *data,
				  RSU_OSAL_SIZE len)
{
	uint8_t *ptr = (uint8_t *)data;
	RSU_LOG_INF("received write qspi");
	uint32_t i = 0;
	for (i = 0; i < len; i++) {
		g_arr[i + offset] = g_arr[i + offset] & ptr[i];
	}
	return 0;
}

/* Mocking function for plat_qspi_erase */
RSU_OSAL_INT plat_qspi_erase_mock(RSU_OSAL_OFFSET offset, RSU_OSAL_SIZE len)
{
	RSU_LOG_INF("received erase info ");
	uint32_t i = 0;
	for (i = offset; i < offset + len; i++) {
		g_arr[i] = 0xff;
	}
	return 0;
}

/* Mocking function for terminate */
RSU_OSAL_INT plat_qspi_terminate_mock(RSU_OSAL_VOID)
{
	RSU_LOG_INF("qspi terminated");
	return 0;
}

/* Init plat_qspi */
RSU_OSAL_INT plat_qspi_init(struct qspi_ll_intf *qspi_intf, RSU_OSAL_CHAR *config_file)
{
	if (!qspi_intf || !config_file) {
		return -EINVAL;
	}

	memset(g_arr,0xff,bufferlength);

	qspi_intf->read = plat_qspi_read_mock;
	qspi_intf->write = plat_qspi_write_mock;
	qspi_intf->erase = plat_qspi_erase_mock;
	qspi_intf->terminate = plat_qspi_terminate_mock;
	return 0;
}
