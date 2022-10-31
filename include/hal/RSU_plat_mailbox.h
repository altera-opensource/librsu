/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_plat_mailbox.h
 * @brief contains SDM mailbox functionality that each platform must populate.
 */
#ifndef RSU_PLAT_MAILBOX_H
#define RSU_PLAT_MAILBOX_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * @brief spt address struct returned by lower platform layers.Contains the SPT address location in
 * QSPI.
 */
struct mbox_data_rsu_spt_address {
	/**SPT0 address in QSPI address space.*/
	RSU_OSAL_U64 spt0_address;
	/**SPT1 address in QSPI address space.*/
	RSU_OSAL_U64 spt1_address;
};

/**
 * @brief mbox status info structure returned by lower platform layers.
 */
struct mbox_status_info {
	/**dependent on ACDS version.*/
	RSU_OSAL_U64 version;
	/**failure code for the failed_image. Zero if no image failed.*/
	RSU_OSAL_U64 state;
	/**QSPI flash address for the currently running image.*/
	RSU_OSAL_U64 current_image;
	/**QSPI flash address for the image which failed. Zero if no image failed.*/
	RSU_OSAL_U64 fail_image;
	/**error location within the failed_image. Zero if no image failed.*/
	RSU_OSAL_U64 error_location;
	/** opaque error code for failed_image, with no meaning to users. Zero if no image failed.*/
	RSU_OSAL_U64 error_details;
	/**count of the number of retries that have been attempted on the current image. The
	 * counter is 0 initially, set to 1 after the first retry, then 2 after a second retry. A
	 * user cannot choose to allow more than 3 attempts to boot an image, so the counter does
	 * not go higher than 2.
	 */
	RSU_OSAL_U64 retry_counter;
};

/**
 * @brief typedef get_rsu_status function which communicates with SDM to query RSU_GET_ERROR_STATUS
 * mailbox command.
 *
 * @param[out] data pointer to @ref mbox_status_info object provided by the library.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*mbox_get_rsu_status_t)(struct mbox_status_info *data);

/**
 * @brief typedef send_rsu_update function to send the address of next application image to SDM.
 *
 * @note The address is send in a staggered form, where the address is initially copied to ATF BL31
 * and passed to SDM on next cold reboot command. This is done so that OS can gracefully exit before
 * the cold reboot.
 *
 * @param[in] addr address to application image in QSPI.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*mbox_send_rsu_update_t)(RSU_OSAL_U64 addr);

/**
 * @brief typedef get_spt_addresses function to query SPT address from SDM.
 *
 * @param[out] mbox_data_retry_counter pointer to struct mbox_data_rsu_spt_address object to be provided by library and
 * filled by the function.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*mbox_get_spt_addresses_t)(struct mbox_data_rsu_spt_address *data);

/**
 * @brief typedef rsu_notify function which send SDM_NOTIFY mailbox command to SDM.
 *
 * @param[in] notify notify value which the library sends to SDM.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*mbox_rsu_notify_t)(RSU_OSAL_U32 notify);

/**
 * @brief typedef terminate function which closes the channel to SDM and free up resources.
 *
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*mbox_terminate_t)(RSU_OSAL_VOID);

/**
 * @brief mailbox api that each platform need to populate and attach to libRSU.
 *
 */
struct mbox_ll_intf {
	/** function pointer to get the rsu status from SDM */
	mbox_get_rsu_status_t get_rsu_status;
	/** function pointer ti set the app image to load to SDM */
	mbox_send_rsu_update_t send_rsu_update;
	/** function pointer to get the SPT address in QSPI from SDM*/
	mbox_get_spt_addresses_t get_spt_addresses;
	/** function pointer to execute RSU notify commnad in SDM*/
	mbox_rsu_notify_t rsu_notify;
	/** terminate interface function pointer*/
	mbox_terminate_t terminate;
};

/**
 * @brief mailbox init function which is called by library to initialize the SDM mailbox interface.
 *
 * @param[out] mbox_intf pointer to struct mbox_ll_intf object which needs to be filled by the function
 * @param[in] config_file config file name which can be used to pass configuration parameters to init
 * function.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT plat_mbox_init(struct mbox_ll_intf *mbox_intf, RSU_OSAL_CHAR *config_file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
