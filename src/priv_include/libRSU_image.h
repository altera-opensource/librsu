/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef __LIBRSU_IMAGE_H__
#define __LIBRSU_IMAGE_H__

#include <libRSU_OSAL.h>

/*
 * A bitstream is a recurrent data structure composed of sections. Each section
 * consists of 4KB blocks. The first block in a section is called the main
 * descriptor and the first 32bit value in that descriptor identifies the
 * section typpe, with 0x62294895 denoting a CMF section.
 * The second block in a section is called a signature block. The last 256 bytes
 * of the signature block are called the main image pointer, and contains up to
 * four pointers to other sections in the bitstream. The entire signature block,
 * including the main pointer area is protected by a 32-bit CRC.
 *
 * The slot size is used to determine if the bitstream was generated using a
 * slot offset address of zero. The main image pointers of all the CMF sections
 * identified in the bitstream are updated when programming into a slot if all
 * of the pointers are less than the slot size.
 */

#define IMAGE_BLOCK_SZ	   0x1000     /** Bitstream block size */
#define SIG_BLOCK_PTR_OFFS 0x0F00     /** Signature block pointer offset */
#define SIG_BLOCK_CRC_OFFS 0x0FFC     /** Signature block CRC offset */
#define CMF_MAGIC	   0x62294895 /** Magic identifier for CMF sections */

/**
 * @brief enum rsu_block_type - enumeration for image block types
 */
enum rsu_block_type {
	SECTION_BLOCK = 0, /** section block*/
	SIGNATURE_BLOCK, /** signature block*/
	REGULAR_BLOCK /** all other block types*/
};

/** maximum number of sections supported for an image */
#define MAX_SECTIONS 64

/**
 * @brief struct rsu_image_state - structure for stated of image processing
 *
 * This structure is used to maintain the state of image parsing, both for
 * relocating images to final destination in flash, and also for verifying
 * images already stored in flash.
 */
struct rsu_image_state {
	RSU_OSAL_S32 offset; /** current block offset in bytes*/
	enum rsu_block_type block_type; /** current block type*/
	RSU_OSAL_U64 sections[MAX_SECTIONS]; /** identified section offsets*/
	RSU_OSAL_INT no_sections; /** number of identified sections*/
	RSU_OSAL_INT absolute; /** current image is an absolute image*/
};

/**
 * librsu_image_block_init() - initialize state machine for processing blocks
 * @param[in] state: current state machine state
 *
 * Function is called before processing images either for writing to flash or
 * for comparison with verification data.
 *
 * @returns 0 on success, or -1 on error
 */
RSU_OSAL_INT librsu_image_block_init(struct rsu_image_state *state);

/**
 * librsu_image_block_process() - process image blocks
 *
 * @param[in] state: current state machine state
 * @param[in] block: pointer to current 4KB image block
 * @param[in] vblock: pointer to current 4KB image verification block
 * @param[in] info rsu_slot_info structure for target slot
 *
 * Image blocks are processed either for updating before writing to flash
 * (when vblock==NULL) or for comparison with verification data
 * (when vblock!=NULL)
 *
 * @returns 0 on success and -1 on error
 */
RSU_OSAL_INT librsu_image_block_process(struct rsu_image_state *state, RSU_OSAL_VOID *block,
					RSU_OSAL_VOID *vblock, struct rsu_slot_info *info);

#endif
