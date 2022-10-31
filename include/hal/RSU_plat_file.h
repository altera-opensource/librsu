/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_plat_file.h
 * @brief contains file operations that each platform must populate.
 */

#ifndef RSU_PLAT_FILE_H
#define RSU_PLAT_FILE_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief RSU OSAL flags for file operations.
 */
typedef enum {
	/** file read operation*/
	RSU_FILE_READ,
	/** file write operation*/
	RSU_FILE_WRITE,
	/** file append operation*/
	RSU_FILE_APPEND
} RSU_filesys_flags_t;

/**
 * @brief RSU OSAL flags for moving file pointer.
 */
typedef enum {
	/** set relative to start*/
	RSU_SEEK_SET,
	/** set relative to current pointer*/
	RSU_SEEK_CUR,
	/** set relative to end of file*/
	RSU_SEEK_END
} RSU_filesys_whence_t;

/**
 * @brief typedef open function for opening a file.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[in] filename path to the file in the system.
 * @param[in] flag file operation flag as per @ref RSU_filesys_flags_t.
 * @return pointer to file object, NULL if error.
 */
typedef RSU_OSAL_FILE *(*file_open_t)(RSU_OSAL_CHAR *filename, RSU_filesys_flags_t flag);

/**
 * @brief typedef read function to read a chunk from a opened file.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[out] buf pointer to buffer to which the file contents are copied into.
 * @param[in] len number of bytes to be read from file.
 * @param[in] file pointer to file object which was returned from open function.
 * @return number of bytes read from file, negative number on error.
 */
typedef RSU_OSAL_INT (*file_read_t)(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file);

/**
 * @brief typedef write function to write a chunk to a opened file.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[in] buf pointer to buffer from which the contents are written to the file.
 * @param[in] len number bytes to be written.
 * @param[in] file pointer to file object returned from open function.
 * @return number of bytes written to file, negative number on error.
 */
typedef RSU_OSAL_INT (*file_write_t)(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file);

/**
 * @brief typedef fgets function to read a string of characters from a file.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[out] str pointer to a string buffer to which the read characters are stored.
 * @param[in] len length of buffer.
 * @param[in] file pointer to file object returned from open function.
 * @return 0 on success, 1 on reaching end of file, negative number on error.
 */
typedef RSU_OSAL_INT (*file_fgets_t)(RSU_OSAL_CHAR *str, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file);

/**
 * @brief typedef fseek function to move the file pointer on an opened file.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[in] offset relative position to the desired location.
 * @param[in] whence base location from which the relative offset is calculated. @ref RSU_filesys_whence_t.
 * @param[in] file pointer to file object returned from open function.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*file_fseek_t)(RSU_OSAL_OFFSET offset, RSU_filesys_whence_t whence,
				RSU_OSAL_FILE *file);

/**
 * @brief typedef fgets function to truncate a file to a known size.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[in] length truncate length in bytes.
 * @param[in] file pointer to file object returned from open function.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*file_ftruncate_t)(RSU_OSAL_OFFSET length, RSU_OSAL_FILE *file);

/**
 * @brief typedef close function to close an opened file.
 *
 * @note needs to be populated for all platforms.
 *
 * @param[in] file pointer to file object returned from open function.
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*file_close_t)(RSU_OSAL_FILE *file);

/**
 * @brief typedef function to terminate and cleanup RSU related file operation.
 *
 * @return 0 on success, negative number on error.
 */
typedef RSU_OSAL_INT (*file_terminate_t)(RSU_OSAL_VOID);

/**
 * @brief file system interface structure which needs to be populated @see plat_filesys_init().
 */
struct filesys_ll_intf {
	/** file open function pointer*/
	file_open_t open;
	/** file read function pointer*/
	file_read_t read;
	/** file write function pointer*/
	file_write_t write;
	/** file fgets function pointer*/
	file_fgets_t fgets;
	/** file fseek function pointer*/
	file_fseek_t fseek;
	/** file ftruncate function pointer*/
	file_ftruncate_t ftruncate;
	/** file close function pointer*/
	file_close_t close;
	/** terminate interface function pointer*/
	file_terminate_t terminate;
};

/**
 * @brief RSU filesystem initialization function.
 *
 * @param[out] filesys_intf interface structure to be provided by RSU library.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT plat_filesys_init(struct filesys_ll_intf *filesys_intf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
