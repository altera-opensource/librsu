/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_plat_qspi.h
 * @brief contains QSPI device operations that each platform must populate.
 */

#ifndef RSU_PLAT_QSPI_H
#define RSU_PLAT_QSPI_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief read from QSPI memory.
 *
 * @param[in] offset offset to which we need to read from.
 * @param[out] data pointer to buffer which is provided by library ,where read data is copied into.
 * @param[in] len length of buffer
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*qspi_read_t)(RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *data, RSU_OSAL_SIZE len);

/**
 * @brief write to QSPI memory.
 *
 * @param[in] offset offset to which we need to write to.
 * @param[in] data pointer to buffer which is provided by library ,where write data is kept.
 * @param[in] len length of buffer.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*qspi_write_t)(RSU_OSAL_OFFSET offset, const RSU_OSAL_VOID *data,
				     RSU_OSAL_SIZE len);

/**
 * @brief erase QSPI memory.
 *
 * @param[in] offset offset to which we need to erase.
 * @param[in] len length of memory region.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*qspi_erase_t)(RSU_OSAL_OFFSET offset, RSU_OSAL_SIZE len);

/**
 * @brief terminate and cleanup any QSPI related resources.
 *
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*qspi_terminate_t)(RSU_OSAL_VOID);

/**
 * @brief structure of function pointers which deals with qspi related functionality.
 *
 */
struct qspi_ll_intf {
	/** function pointer for reading from qspi device*/
	qspi_read_t read;
	/** function pointer for writing to qspi device*/
	qspi_write_t write;
	/** function pointer for erasing qspi device*/
	qspi_erase_t erase;
	/** terminate interface function pointer*/
	qspi_terminate_t terminate;
};

/**
 * @brief platform initialization function to initialize qspi resource.
 *
 * @param[out] qspi_intf pointer to struct qspi_ll_intf object needs to be populated by @ref
 * plat_qspi_init().
 * @param[in] config_file config file name which can be used to pass configuration parameters to
 * init function.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT plat_qspi_init(struct qspi_ll_intf *qspi_intf, RSU_OSAL_CHAR *config_file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
