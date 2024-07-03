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
#include <mock.h>

static RSU_OSAL_VOID swap_bits(RSU_OSAL_CHAR *data, RSU_OSAL_INT len)
{
    RSU_OSAL_INT x, y;
    RSU_OSAL_CHAR tmp;

    for (x = 0; x < len; x++) {
        tmp = 0;
        for (y = 0; y < 8; y++) {
            tmp <<= 1;
            if (data[x] & 1) {
                tmp |= 1;
            }
            data[x] >>= 1;
        }
        data[x] = tmp;
    }
}

/* Mocking function for plat_qspi_read */
RSU_OSAL_INT plat_qspi_read_mock(RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *data, RSU_OSAL_SIZE len)
{
    RSU_OSAL_INT ret;
    RSU_LOG_DBG("received read qspi");
    FILE* qspi_file = fopen(RPD_FILE_TEST_NAME, "rb+");
    RSU_OSAL_CHAR *ptr = (RSU_OSAL_CHAR *)malloc(len);
    if (ptr == NULL) {
        return -ENOMEM;
    }
    fseek(qspi_file, offset, SEEK_SET);
    ret = fread(ptr, 1, len, qspi_file);
    if(ret<0) {
        return ret;
    }
    swap_bits(ptr, len);
    memcpy(data, ptr, len);
    free(ptr);
    fclose(qspi_file);
    return 0;
}

/* Mocking function for plat_qspi_write */
RSU_OSAL_INT plat_qspi_write_mock(RSU_OSAL_OFFSET offset, const RSU_OSAL_VOID *data,
                  RSU_OSAL_SIZE len)
{
    RSU_OSAL_INT ret;
    RSU_LOG_DBG("received write qspi");
    FILE *qspi_file = fopen(RPD_FILE_TEST_NAME, "rb+");
    RSU_OSAL_CHAR *ptr_data = (RSU_OSAL_CHAR *)data;
    RSU_OSAL_CHAR *ptr = (RSU_OSAL_CHAR *)malloc(len);
    if (ptr == NULL) {
        return -ENOMEM;
    }
    fseek(qspi_file, offset, SEEK_SET);
    ret = fread(ptr, 1, len, qspi_file);
    if(ret<0) {
        return ret;
    }
    swap_bits(ptr, len);
    for (uint32_t i = 0; i < len; i++) {
        ptr[i] = ptr[i] & ptr_data[i];
    }
    swap_bits(ptr, len);
    fseek(qspi_file, offset, SEEK_SET);
    fwrite(ptr, 1, len, qspi_file);
    free(ptr);
    fclose(qspi_file);
    return 0;
}

/* Mocking function for plat_qspi_erase */
RSU_OSAL_INT plat_qspi_erase_mock(RSU_OSAL_OFFSET offset, RSU_OSAL_SIZE len)
{
    RSU_LOG_DBG("received erase info ");
    FILE *qspi_file = fopen(RPD_FILE_TEST_NAME, "rb+");
    fseek(qspi_file, offset, SEEK_SET);
    uint8_t ptr = 0xff;
    for (uint32_t i = 0; i < len; i++) {
        fwrite(&ptr, 1, 1, qspi_file);
    }
    fclose(qspi_file);
    return 0;
}

/* Mocking function for terminate */
RSU_OSAL_INT plat_qspi_terminate_mock(RSU_OSAL_VOID)
{
    RSU_LOG_DBG("qspi terminated");
    return 0;
}

/* Init plat_qspi */
RSU_OSAL_INT plat_qspi_init(struct qspi_ll_intf *qspi_intf, RSU_OSAL_CHAR *config_file)
{
    if (!qspi_intf || !config_file) {
        return -EINVAL;
    }

    qspi_intf->read = plat_qspi_read_mock;
    qspi_intf->write = plat_qspi_write_mock;
    qspi_intf->erase = plat_qspi_erase_mock;
    qspi_intf->terminate = plat_qspi_terminate_mock;
    return 0;
}
