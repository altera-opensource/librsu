/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file libRSU.h
 * @brief Contains the public fuctions to be used by each application to exercise libRSU functionality.
 */

#ifndef LIBRSU_H
#define LIBRSU_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Initialization failed */
#define ELIB	       1
/** Configuration failed */
#define ECFG	       2
/** Invalid slot number */
#define ESLOTNUM       3
/** Invalid format */
#define EFORMAT	       4
/** Trying to use a non existant slot */
#define EERASE	       5
/** Trying to program a programmed slot */
#define EPROGRAM       6
/** Comaprison failure */
#define ECMP	       7
/** Trying to program larger than slot size */
#define ESIZE	       8
/**  Slot name not found or reserved slot name */
#define ENAME	       9
/** error in IO operations */
#define EFILEIO	       10
/** error during callback execution */
#define ECALLBACK      11
/** lowlevel error */
#define ELOWLEVEL      12
/** error for modyfying/erasing write protected content */
#define EWRPROT	       13
/** invalid arguments */
#define EARGS	       14
/** corrupted CPB*/
#define ECORRUPTED_CPB 15
/** corrupted SPT */
#define ECORRUPTED_SPT 16


/** RSU_VERSION_CRT_DCMF_IDX */
#define RSU_VERSION_CRT_DCMF_IDX(v) (((v)&0xF0000000) >> 28)
/** RSU_VERSION_ERROR_SOURCE*/
#define RSU_VERSION_ERROR_SOURCE(v) (((v)&0x0FFF0000) >> 16)
/** RSU_VERSION_ACMF_VERSION */
#define RSU_VERSION_ACMF_VERSION(v) (((v)&0xFF00) >> 8)
/** RSU_VERSION_DCMF_VERSION*/
#define RSU_VERSION_DCMF_VERSION(v) ((v)&0xFF)


/** DCMF_VERSION_MAJOR */
#define DCMF_VERSION_MAJOR(v)  (((v)&0xFF000000) >> 24)
/** DCMF_VERSION_MINOR */
#define DCMF_VERSION_MINOR(v)  (((v)&0x00FF0000) >> 16)
/** DCMF_VERSION_UPDATE */
#define DCMF_VERSION_UPDATE(v) (((v)&0x0000FF00) >> 8)

/**
 * @brief SDM notify value to be sent from final OS boot-up.
 */
#define RSU_SDM_NOTIFY_OS (0x0002U)

/**
 * @brief structure to capture SDM status log information.
 */
struct rsu_status_info {
	/** version */
	RSU_OSAL_U64 version;
	/** state information*/
	RSU_OSAL_U64 state;
	/** current image location */
	RSU_OSAL_U64 current_image;
	/** previous failed image location*/
	RSU_OSAL_U64 fail_image;
	/** error loction */
	RSU_OSAL_U64 error_location;
	/** error details */
	RSU_OSAL_U64 error_details;
	/** currewnt retry counter*/
	RSU_OSAL_U64 retry_counter;
};

/**
 * @brief Macro to retrieve MAJOR version of RSU library.
 */
#define RSU_MAJOR_VERSION(x) (((x) >> 16) & 0xFFFF)

/**
 * @brief Macro to retrieve MINOR version of RSU library.
 */
#define RSU_MINOR_VERSION(x) ((x) & 0xFFFF)

/**
 * @brief get the current library version as MAJOR.MINOR .
 *
 * @return version number in MAJOR.MINOR(XXXX.YYYY) format.
 */
RSU_OSAL_U32 rsu_get_version(RSU_OSAL_VOID);

/**
 * @brief Load the configuration file and initialize internal data
 *
 * @param[in] filename configuration file to load (if Null or empty string, the default string will
 * be /etc/librsu.rc)
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT librsu_init(RSU_OSAL_CHAR *filename);

/**
 * @brief cleanup internal data and release librsu
 * @return Nil
 */
RSU_OSAL_VOID librsu_exit(RSU_OSAL_VOID);

/**
 * @brief report HPS software execution stage as a 16bit number stage: software execution stage
 *
 * @param[in] value value to be sent to SDM for notify.
 * @return 0, on success, or negative Error Code on failure.
 */
RSU_OSAL_INT rsu_notify(RSU_OSAL_INT value);

/**
 * @brief  Copy the Secure Domain Manager status log to info struct info
 *
 * @param[out] info pointer to info struct
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_status_log(struct rsu_status_info *info);

/**
 * @brief clear errors from the current status log
 *
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_clear_error_status(RSU_OSAL_VOID);

/**
 * @brief  reset the retry counter, so that the currently
 *         running image may be tried again after a watchdog
 *         timeout.
 *
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_reset_retry_counter(RSU_OSAL_VOID);

/**
 * @brief get the number of slots defined
 *
 * @return number of defined slots or negative error code
 */
RSU_OSAL_INT rsu_slot_count(RSU_OSAL_VOID);

/**
 * @brief return slot number based on name
 *
 * @param[in] name name of slot
 * @return slot number on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_by_name(RSU_OSAL_CHAR *name);

/**
 * @brief structure to capture slot information details
 *
 */
struct rsu_slot_info {
	/** slot name*/
	RSU_OSAL_CHAR name[16];
	/** offset in memory*/
	RSU_OSAL_U64 offset;
	/** size of slot*/
	RSU_OSAL_INT size;
	/** priority in cpb table*/
	RSU_OSAL_INT priority;
};

/**
 * @brief return the attributes of a slot
 *
 * @param[in] slot slot number
 * @param[out] info pointer to info structure
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_get_info(RSU_OSAL_INT slot, struct rsu_slot_info *info);

/**
 * @brief get the size of a slot
 *
 * @param[in] slot slot number
 * @return the size of the slot in bytes, or Error Code
 */
RSU_OSAL_INT rsu_slot_size(RSU_OSAL_INT slot);

/**
 * @brief get the Decision CMF load priority of a slot
 *
 * @note: Priority of zero means the slot has no priority and is disabled. The slot with priority of
 * one has the highest priority.
 *
 *
 * @param[in] slot slot number
 * @return the priority of the slot, or Error Code
 */
RSU_OSAL_INT rsu_slot_priority(RSU_OSAL_INT slot);

/**
 * @brief erase all data in a slot to prepare for programming. Remove the slot if it is in the CPB.
 *
 * @param[in] slot slot number
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_erase(RSU_OSAL_INT slot);

/**
 * @brief program a slot using FPGA config data from a buffer and enter slot into CPB
 *
 * @param[in] slot slot number
 * @param[in] buf pointer to data buffer
 * @param[in] size bytes to read from buffer
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size);

/**
 * @brief program a slot using factory update data from a buffer and enter slot into CPB
 *
 * @param[in] slot slot number
 * @param[in] buf pointer to data buffer
 * @param[in] size bytes to read from buffer
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_factory_update_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf,
						 RSU_OSAL_INT size);

/**
 * @brief program a slot using FPGA config data from a file and enter slot into CPB
 *
 * @param[in] slot slot number
 * @param[in] filename input data file
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename);

/**
 * @brief program a slot using factory update data from a file and enter slot into CPB
 *
 * @param[in] slot slot number
 * @param[in] filename input data file
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_factory_update_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename);

/**
 * @brief program a slot using raw data from a buffer. The slot is not entered into the CPB
 *
 * @param[in] slot slot number
 * @param[in] buf pointer to data buffer
 * @param[in] size bytes to read from
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_buf_raw(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size);

/**
 * @brief program a slot using raw data from a file.The slot is not entered into the CPB
 *
 * @param[in] slot slot number
 * @param[in] filename input data file
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_file_raw(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename);

/**
 * @brief verify FPGA config data in a slot against a buffer
 *
 * @param[in] slot slot number
 * @param[in] buf pointer to data buffer
 * @param[in] size bytes to read from buffer
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_verify_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size);

/**
 * @brief verify FPGA config data in a slot against a file
 *
 * @param[in] slot slot number
 * @param[in] filename input data file
 * @return  0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_verify_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename);

/**
 * @brief verify raw data in a slot against a buffer
 *
 * @param[in] slot slot number
 * @param[in] buf pointer to data buffer
 * @param[in] size bytes to read from buffer
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_verify_buf_raw(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size);

/**
 * @brief verify raw data in a slot against a file
 *
 * @param[in] slot slot number
 * @param[in] filename input data file
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_verify_file_raw(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename);

/**
 * @brief function pointer type for callback function for providing data.
 *
 * @param[out] buf which needs to be filled by callback function.
 * @param[in] size size of the buffer provided to callback function.
 * @return number of bytes copied into buffer , 0 on end of content and negative number on error
 */
typedef RSU_OSAL_INT (*rsu_data_callback)(RSU_OSAL_VOID *buf, RSU_OSAL_INT size);

/**
 * @brief program and verify a slot using FPGA config data provided by a callback function. Enter
 * the slot into the CPB
 *
 * @param[in] slot slot number
 * @param[in] callback callback function to provide input data
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_program_callback(RSU_OSAL_INT slot, rsu_data_callback callback);

/**
 * @brief program and verify a slot using raw data provided by a callback function. The slot is not
 * entered into the CPB
 *
 * @param[in] slot slot number
 * @param[in] callback callback function to provide input data
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_program_callback_raw(RSU_OSAL_INT slot, rsu_data_callback callback);

/**
 * @brief verify a slot using FPGA config data provided by a callback function.
 *
 * @param[in] slot
 * @param[in] callback
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_verify_callback(RSU_OSAL_INT slot, rsu_data_callback callback);

/**
 * @brief verify a slot using raw data provided by a callback function.
 *
 * @param[in] slot slot number
 * @param[in] callback callback function to provide input data
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_verify_callback_raw(RSU_OSAL_INT slot, rsu_data_callback callback);

/**
 * @brief read the data in a slot and write to a file
 *
 * @param[in] slot slot number
 * @param[in] filename output data file
 * @return 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_copy_to_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename);

/**
 * @brief Set the selected slot as the highest priority. It will be the first slot tried after a
 * power-on reset
 *
 * @param[in] slot slot number
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_enable(RSU_OSAL_INT slot);

/**
 * @brief Remove the selected slot from the priority scheme, but do not erase the slot data so that
 * it can be re-enabled.
 *
 * @param[in] slot slot number
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_disable(RSU_OSAL_INT slot);

/**
 * @brief Request that the selected slot be loaded after the next reboot, no matter the priority. A
 * power-on reset will ignore this request and use slot priority to select the first slot.
 *
 * @param[in] slot slot number
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_load_after_reboot(RSU_OSAL_INT slot);

/**
 * @brief Request that the factory image be loaded after the next reboot. A power-on reset will
 * ignore this request and use slot priority to select the first slot.
 *
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_load_factory_after_reboot(RSU_OSAL_VOID);

/**
 * @brief Rename the selected slot.
 *
 * @param[in] slot slot number
 * @param[in] name new name for slot
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_rename(RSU_OSAL_INT slot, RSU_OSAL_CHAR *name);

/**
 * @brief Delete the selected slot.
 *
 * @param[in] slot slot number
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_delete(RSU_OSAL_INT slot);

/**
 * @brief Create a new slot.
 *
 * @param[in] name slot name
 * @param[in] address slot start address
 * @param[in] size slot size
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_create(RSU_OSAL_CHAR *name, RSU_OSAL_U64 address, RSU_OSAL_U32 size);

/**
 * @brief save spt to the file
 *
 * @note This function is used to save the working SPT to a file.
 *
 * @param[in] name file name which SPT will be saved to
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_save_spt(RSU_OSAL_CHAR *name);

/**
 * @brief restore spt from the file
 *
 * @param[in] name file name which SPT will be restored from
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_restore_spt(RSU_OSAL_CHAR *name);

/**
 * @brief save cpb to the file
 *
 * @note This function is used to save the working CPB to a file.
 *
 * @param[in] filename the name of file which cpb is saved to
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_save_cpb(RSU_OSAL_CHAR *filename);

/**
 * @brief create a header only cpb
 *
 * @note This function is used to create a empty CPB, which include CPB header only.
 *
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_create_empty_cpb(RSU_OSAL_VOID);

/**
 * @brief restore the cpb
 *
 * @note This function is used to restore the CPB from a saved file.
 *
 * @param[in] filename the name of file which cpb is restored from
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_restore_cpb(RSU_OSAL_CHAR *filename);

/**
 * @brief determine if current running image is factory image
 *
 * @param[out] factory set to non-zero value when running factory image, zero otherwise
 * @returns 0 on success, or negative error code on error.
 */
RSU_OSAL_INT rsu_running_factory(RSU_OSAL_INT *factory);

/**
 * @brief retrieve the decision firmware version
 *
 * @note This function is used to retrieve the version of each of the four DCMF copies in flash.
 *
 * @param[out] versions pointer to where the four DCMF versions will be stored
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_dcmf_version(RSU_OSAL_U32 *versions);

/**
 * @brief retrieve the max_retry parameter
 *
 * @note This function is used to retrieve the max_retry parameter from flash.
 *
 * @param[out] value pointer to where the max_retry will be stored
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_max_retry(RSU_OSAL_U8 *value);

/**
 * @brief retrieve the decision firmware status
 *
 * @note This function is used to determine whether decision firmware copies are corrupted in flash,
 * with the currently used decision firmware being used as reference. The status is an array of 4
 * values, one for each decision firmware copy. A 0 means the copy is fine, anything else means the
 * copy is corrupted.
 *
 * @param[out] status pointer to where the status values will be stored
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_dcmf_status(RSU_OSAL_INT *status);

/**
 * @brief read the data in a slot and write to a file
 *
 * @param[in] slot slot number
 * @param[out] buffer pointer to buffer
 * @param[in] size size of buffer
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_slot_copy_to_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size);

/**
 * @brief save spt to buffer
 *
 * @param buffer pointer to buffer where SPT will be copied into.
 *
 * @note This function is used to save the working SPT to a buffer.
 * @note buffer size needs to be greater than 4K + 4 bytes.
 *
 * @param[in] size size of the buffer
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_save_spt_to_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size);

/**
 * @brief restore spt from buffer
 *
 * @param[in] buffer pointer to buffer from which the SPT will be retrieved.
 * @param[in] size size of the buffer.
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_restore_spt_from_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size);

/**
 * @brief save cpb to buffer
 *
 * @note This function is used to save the working CPB to a buffer.
 * @note buffer size needs to be greater than 4K + 4 bytes.
 *
 * @param[out] buffer pointer to buffer to which CPB will be saved to.
 * @param[in] size size of the buffer
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_save_cpb_to_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size);

/**
 * @brief pointer to buffer where CPB will be copied from.
 *
 * @param[in] buffer pointer to buffer from which CPB will be retrieved
 * @param[in] size size of the buffer
 * @return 0 on success, or Error code
 */
RSU_OSAL_INT rsu_restore_cpb_from_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
