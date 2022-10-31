/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef MOCK_SPT_H
#define MOCK_SPT_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

#define SPT_MAGIC_NUMBER  0x57713427
#define SPT_VERSION	  0
#define SPT_FLAG_RESERVED 1
#define SPT_FLAG_READONLY 2

#define SPT_MAX_PARTITIONS 127

struct SUB_PARTITION_TABLE {
	RSU_OSAL_U32 magic_number;
	RSU_OSAL_U32 version;
	RSU_OSAL_U32 partitions;
	RSU_OSAL_U32 checksum;
	RSU_OSAL_U32 __RSVD[4];

	struct {
		RSU_OSAL_CHAR name[16];
		RSU_OSAL_U64 offset;
		RSU_OSAL_U32 length;
		RSU_OSAL_U32 flags;
	} partition[SPT_MAX_PARTITIONS];
}__attribute__((__packed__));

union CMF_POINTER_BLOCK {
	struct {
		RSU_OSAL_U32 magic_number;
		RSU_OSAL_U32 header_size;
		RSU_OSAL_U32 cpb_size;
		RSU_OSAL_U32 cpb_reserved;
		RSU_OSAL_U32 image_ptr_offset;
		RSU_OSAL_U32 image_ptr_slots;
	} header;
	struct {
		uint32_t rsv[8];
		uint64_t imp_ptr[508];
	} image;
	RSU_OSAL_U8 data[4 * 1024];
}__attribute__((__packed__));

struct spt_with_padding{
	struct SUB_PARTITION_TABLE mock_spt;
	char padding[28 * 1024];
}__attribute__((__packed__));
struct cpb_with_padding{
	union CMF_POINTER_BLOCK mock_cpb;
	char padding[28 * 1024];
}__attribute__((__packed__));

struct full{
	struct spt_with_padding mock_spt_full[2];
	struct cpb_with_padding mock_cpb_full[2];
	char slot1[28 * 1024];
}__attribute__((__packed__));

#ifdef __cplusplus
}



#endif  /* __cplusplus */

#endif
