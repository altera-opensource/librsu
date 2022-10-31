/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_plat_misc.h
 * @brief contains miscellaneous functionality that might be different between platforms
 * that each platform must populate.
 */

#ifndef RSU_PLAT_MISC_H
#define RSU_PLAT_MISC_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief struct rsu_dcmf_status holds the status of 4 dcmf copy in QSPI.
 *
 */
struct rsu_dcmf_status {
	/** DCMF status information on four copies in QSPI*/
	RSU_OSAL_U32 dcmf[4];
};

/**
 * @brief struct rsu_dcmf_version holds the version of 4 dcmf copy in QSPI.
 *
 */
struct rsu_dcmf_version {
	/** DCMF version information on 4 copies of DCMF in QSPI*/
	RSU_OSAL_INT dcmf[4];
};

/**
 * @brief typedef misc_rsu_get_dcmf_status function to get the dcmf status information.
 *
 * @param[out] data pointer to struct rsu_dcmf_status object
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*misc_rsu_get_dcmf_status_t)(struct rsu_dcmf_status *data);

/**
 * @brief typedef misc_su_get_max_retry_count function to get the max retry value.
 *
 * @param[out] rsu_max_retry pointer to RSU_OSAL_U8 object given by library to be filled by function
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*misc_rsu_get_max_retry_count_t)(RSU_OSAL_U8 *rsu_max_retry);

/**
 * @brief typedef misc_rsu_get_dcmf_version function to get the dcmf version.
 *
 * @param[out] version pointer to struct rsu_dcmf_version to be filled by function.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*misc_rsu_get_dcmf_version_t)(struct rsu_dcmf_version *version);

/**
 * @brief typedef function to terminate and cleanup RSU related misc operation.
 *
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*misc_terminate_t)(RSU_OSAL_VOID);

/**
 * @brief structure holds the function pointer which needs to be populated by @ref
 * plat_rsu_misc_init() for each platform.
 *
 */
struct rsu_ll_misc {
	/** get dcmf status */
	misc_rsu_get_dcmf_status_t rsu_get_dcmf_status;
	/** get max retry value */
	misc_rsu_get_max_retry_count_t rsu_get_max_retry_count;
	/** get dcmf version*/
	misc_rsu_get_dcmf_version_t rsu_get_dcmf_version;
	/** terminate interface function pointer*/
	misc_terminate_t terminate;
};

/**
 * @brief Initializes the miscellaneous functions in rsu platform layer.These functions might be
 * different in different platforms.
 *
 * @param[out] misc_intf pointer to struct rsu_ll_misc which needs to be populated by the function
 * @param[in] config_file config file name which can be used to pass configuration parameters to init
 * function.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT plat_rsu_misc_init(struct rsu_ll_misc *misc_intf, RSU_OSAL_CHAR *config_file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
