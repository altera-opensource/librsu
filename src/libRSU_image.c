/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU.h>
#include <libRSU_image.h>
#include <libRSU_misc.h>
#include <hal/RSU_plat_crc32.h>
#include <utils/RSU_logging.h>

/**
 * @brief pointer block in application image
 */
struct pointer_block {
	/** reserved0 */
	RSU_OSAL_U32 RSVD0;
	/** reserved1 */
	RSU_OSAL_U32 RSVD1;
	/**pointers */
	RSU_OSAL_U64 ptrs[4];
	/** reserved2 */
	RSU_OSAL_U8 RSVD2[0xd4];
	/** crc value*/
	RSU_OSAL_U32 crc;
};

/**
 * @brief search section in the current list of identified sections
 *
 * @param state current state machine state
 * @param section section to be searched
 * @return 1 if section is found, 0 if section is not found
 */
static RSU_OSAL_INT find_section(struct rsu_image_state *state, RSU_OSAL_U64 section)
{
	RSU_OSAL_INT x;

	for (x = 0; x < state->no_sections; x++) {
		if (section == state->sections[x]) {
			return 1;
		}
	}

	return 0;
}

/**
 * @brief add section to the current list of identified sections
 *
 * @param state current state machine state
 * @param section section to be added
 * @return zero value for success, or negative value on error
 */
static RSU_OSAL_INT add_section(struct rsu_image_state *state, RSU_OSAL_U64 section)
{
	if (find_section(state, section)) {
		return 0;
	}

	if (state->no_sections >= MAX_SECTIONS) {
		return -1;
	}

	state->sections[state->no_sections++] = section;

	return 0;
}

/**
 * @brief process signature block
 *
 * @note  Determine if the signature block is part of an absolute image, and add its section
 * pointers to the list of identified sections.
 *
 * @param state current state machine state
 * @param block signature block
 * @param info slot where the data will be written
 * @return zero value for success, or negative value on error
 */
static RSU_OSAL_INT sig_block_process(struct rsu_image_state *state, RSU_OSAL_VOID *block,
				      struct rsu_slot_info *info)
{
	RSU_OSAL_CHAR *data = (RSU_OSAL_CHAR *)block;
	struct pointer_block *ptr_blk = (struct pointer_block *)(data + SIG_BLOCK_PTR_OFFS);
	RSU_OSAL_INT x;

	/* Determine if absolute image - only done for 2nd block in an image
	 * which is always a signature block
	 */
	if (state->offset == IMAGE_BLOCK_SZ) {
		for (x = 0; x < 4; x++) {
			if (ptr_blk->ptrs[x] > (RSU_OSAL_U64)info->size) {
				state->absolute = 1;
				RSU_LOG_INF("Identified absolute image.");
				break;
			}
		}
	}

	/* Add pointers to list of identified sections */
	for (x = 0; x < 4; x++) {
		if (ptr_blk->ptrs[x]) {
			if (state->absolute) {
				add_section(state, ptr_blk->ptrs[x] - info->offset);
			} else {
				add_section(state, ptr_blk->ptrs[x]);
			}
		}
	}

	return 0;
}

/**
 * @brief adjust signature block pointers before writing to flash
 *
 * @note This function checks that the section pointers are consistent, and for non-absolute images
 * it updates them to match the destination slot, also re-computing the CRC.
 *
 * @param state current state machine state
 * @param block signature block
 * @param info slot where the data will be written
 * @return zero value for success, or negative value on error
 */
static RSU_OSAL_INT sig_block_adjust(struct rsu_image_state *state, RSU_OSAL_VOID *block,
				     struct rsu_slot_info *info)
{
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_INT x;
	RSU_OSAL_CHAR *data = (RSU_OSAL_CHAR *)block;
	struct pointer_block *ptr_blk = (struct pointer_block *)(data + SIG_BLOCK_PTR_OFFS);

	/*
	 * Check CRC on 4kB block before proceeding.  All bytes must be
	 * bit-swapped before they can used in CRC32 library function.
	 * The CRC value is stored in big endian in the bitstream.
	 */
	swap_bits(block, IMAGE_BLOCK_SZ);
	calc_crc = rsu_crc32(0, block, SIG_BLOCK_CRC_OFFS);
	if (swap_endian32(ptr_blk->crc) != calc_crc) {
		RSU_LOG_ERR("Error: Bad CRC32. Calc = %08X / From Block = %08x", calc_crc,
			    swap_endian32(ptr_blk->crc));
		return -1;
	}
	swap_bits(block, IMAGE_BLOCK_SZ);

	/* Check pointers */
	for (x = 0; x < 4; x++) {
		RSU_OSAL_S64 ptr = ptr_blk->ptrs[x];

		if (!ptr) {
			continue;
		}

		if (state->absolute) {
			ptr -= info->offset;
		}

		if (ptr > info->size) {
			RSU_LOG_ERR("Error: A pointer not within the slot");
			return -1;
		}
	}

	/* Absolute images do not require pointer updates */
	if (state->absolute) {
		return 0;
	}

	/* Update pointers */
	for (x = 0; x < 4; x++) {
		if (ptr_blk->ptrs[x]) {
			RSU_OSAL_U64 old = ptr_blk->ptrs[x];

			ptr_blk->ptrs[x] += info->offset;
			RSU_LOG_ERR("Adjusting pointer 0x%llx -> 0x%llx.", old, ptr_blk->ptrs[x]);
		}
	}

	/* Update CRC in block */
	swap_bits(block, IMAGE_BLOCK_SZ);
	calc_crc = rsu_crc32(0, block, SIG_BLOCK_CRC_OFFS);
	ptr_blk->crc = swap_endian32(calc_crc);
	swap_bits(block, IMAGE_BLOCK_SZ);

	return 0;
}

/**
 * @brief compare two image blocks
 *
 * @param state current state machine state
 * @param block input data provided by user
 * @param vblock verification data read from flash
 * @return non-negative value for successful comparison, or negative value on failure or comparison
 * difference found.
 */
static RSU_OSAL_INT block_compare(struct rsu_image_state *state, RSU_OSAL_VOID *block,
				  RSU_OSAL_VOID *vblock)
{
	RSU_OSAL_CHAR *buf = (RSU_OSAL_CHAR *)block;
	RSU_OSAL_CHAR *vbuf = (RSU_OSAL_CHAR *)vblock;
	RSU_OSAL_INT x;

	for (x = 0; x < IMAGE_BLOCK_SZ; x++) {
		if (vbuf[x] != buf[x]) {
			RSU_LOG_ERR("Expect %02X, got %02X @0x%08X", buf[x], vbuf[x],
				    (RSU_OSAL_U32)state->offset + x);
			return -ECMP;
		}
	}

	return 0;
}

/**
 * @brief compare two signature blocks
 *
 * @note Absolute images are compared directly, while for non-absolute images the pointers and
 * associated CRC are re-computed to see if they match.
 *
 * @param state current state machine state
 * @param ublock input data provided by user
 * @param vblock verification data read from flash
 * @param info slot where the verification data was read from
 * @return zero for success, or negative value on error or finding differences.
 */
static RSU_OSAL_INT sig_block_compare(struct rsu_image_state *state, RSU_OSAL_VOID *ublock,
				      RSU_OSAL_VOID *vblock, struct rsu_slot_info *info)
{
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_INT ret;
	RSU_OSAL_INT x;
	RSU_OSAL_CHAR *block;

	block = rsu_malloc(IMAGE_BLOCK_SZ);
	if (block == NULL) {
		RSU_LOG_ERR("error in allocating memory");
		return -ENOMEM;
	}

	struct pointer_block *ptr_blk = (struct pointer_block *)(block + SIG_BLOCK_PTR_OFFS);

	RSU_LOG_INF("Comparing signature block @0x%08x", (RSU_OSAL_U32)state->offset);

	/* Make a copy of the data provided by the user */
	rsu_memcpy(block, ublock, IMAGE_BLOCK_SZ);

	/* Update signature block to match what we expect in flash */
	if (!state->absolute) {

		/* Update pointers */
		for (x = 0; x < 4; x++) {
			if (ptr_blk->ptrs[x]) {
				ptr_blk->ptrs[x] += info->offset;
			}
		}

		/* Update CRC in block */
		swap_bits(block, IMAGE_BLOCK_SZ);
		calc_crc = rsu_crc32(0, (RSU_OSAL_U8 *)block, SIG_BLOCK_CRC_OFFS);
		ptr_blk->crc = swap_endian32(calc_crc);
		swap_bits(block, IMAGE_BLOCK_SZ);
	}

	ret = block_compare(state, block, vblock);

	rsu_free(block);

	return ret;
}

RSU_OSAL_INT librsu_image_block_init(struct rsu_image_state *state)
{
	RSU_LOG_INF("Resetting image block state machine.");

	state->no_sections = 1;
	add_section(state, 0);
	state->block_type = REGULAR_BLOCK;
	state->absolute = 0;
	state->offset = -IMAGE_BLOCK_SZ;

	return 0;
}

RSU_OSAL_INT librsu_image_block_process(struct rsu_image_state *state, RSU_OSAL_VOID *block,
					RSU_OSAL_VOID *vblock, struct rsu_slot_info *info)
{
	RSU_OSAL_U32 magic;

	state->offset += IMAGE_BLOCK_SZ;

	if (find_section(state, state->offset)) {
		state->block_type = SECTION_BLOCK;
	}

	switch (state->block_type) {

	case SECTION_BLOCK:
		magic = *(RSU_OSAL_U32 *)block;
		if (magic == CMF_MAGIC) {
			RSU_LOG_INF("Found CMF section @0x%08x.", (RSU_OSAL_U32)state->offset);
			state->block_type = SIGNATURE_BLOCK;
		} else {
			state->block_type = REGULAR_BLOCK;
		}

		if (vblock) {
			return block_compare(state, block, vblock);
		}
		break;

	case SIGNATURE_BLOCK:
		RSU_LOG_INF("Found signature block @0x%08x.", (RSU_OSAL_U32)state->offset);

		if (sig_block_process(state, block, info)) {
			return -1;
		}

		state->block_type = REGULAR_BLOCK;

		if (vblock) {
			return sig_block_compare(state, block, vblock, info);
		}

		if (sig_block_adjust(state, block, info)) {
			return -1;
		}

		break;

	case REGULAR_BLOCK:
		break;
	}

	if (vblock) {
		return block_compare(state, block, vblock);
	}

	return 0;
}
