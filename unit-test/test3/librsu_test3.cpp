/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <gtest/gtest.h>
#include <utils/RSU_utils.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libRSU.h>
#include <hal/RSU_plat_qspi.h>
#include <hal/RSU_plat_mailbox.h>
#include <hal/RSU_plat_file.h>
#include <hal/RSU_plat_crc32.h>
#include <hal/RSU_plat_misc.h>
#include <string.h>

#include <rsu_mock_utils.h>
#include <mock_spt.h>

#define SPT_CHECKSUM_OFFSET 0x0C
#define SPT_MAGIC_NUMBER    0x57713427
#define SPT_VERSION	    0
#define SPT_FLAG_RESERVED   1
#define SPT_FLAG_READONLY   2
#define SPT_MAX_PARTITIONS  127
#define CPB_MAGIC_NUMBER    0x57789609
#define CPB_HEADER_SIZE	    24

extern struct full mock_full;

/*
 * test case to do initialization without any Application slot:providing invalid slot number as 1
 * (SPT1,SPT0,CPB1,CPB0 occupied first 4 slots(index starts with 0) slot 1 is invalid for Application slot)
 * povided with valid SPT1, CPB0, CPB1
 * povided with valid SPT1, CPB0, CPB1
 * expecting it to recover SPT0
 * test case to check slot count successfully
 * test case to check slot by name with dummy name and expecting it to fail
 * test case to check slot info by providing invalid slot number expecting it to fail
 * test case to check slot priority by providing invalid slot number expecting it to fail
 * test case to check slot size by providing invalid slot number expecting it to fail
 * test case to check slot erase by providing invalid slot number expecting it to fail
 * performing exit for every init test case
 */
TEST(librsu_test3, test_no_slot)
{
	int ret = 0;

	mock_full.mock_spt_full[1].mock_spt.magic_number = SPT_MAGIC_NUMBER;
	mock_full.mock_spt_full[1].mock_spt.version = (RSU_OSAL_U32)1;
	char *spt_data;
	spt_data = (char *)malloc(sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[1].mock_spt.checksum = (RSU_OSAL_U32)0xFFFFFFFF;
	memcpy(spt_data, &mock_full.mock_spt_full[1].mock_spt, sizeof(struct SUB_PARTITION_TABLE));
	memset(spt_data + SPT_CHECKSUM_OFFSET, 0,
	       sizeof(mock_full.mock_spt_full[1].mock_spt.checksum));
	swap_bits(spt_data, sizeof(struct SUB_PARTITION_TABLE));
	RSU_OSAL_U32 calc_crc =
		rsu_crc32(0, (RSU_OSAL_U8 *)spt_data, sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[1].mock_spt.checksum = swap_endian32(calc_crc);
	swap_bits(spt_data, sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[1].mock_spt.magic_number = SPT_MAGIC_NUMBER;
	free(spt_data);

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)4;
	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[0].name, "SPT0");
	mock_full.mock_spt_full[1].mock_spt.partition[0].offset =
		(RSU_OSAL_U64)&mock_full.mock_spt_full[0].mock_spt;
	mock_full.mock_spt_full[1].mock_spt.partition[0].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[1].name, "SPT1");
	mock_full.mock_spt_full[1].mock_spt.partition[1].offset =
		(RSU_OSAL_U64)&mock_full.mock_spt_full[1].mock_spt;
	mock_full.mock_spt_full[1].mock_spt.partition[1].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[2].name, "CPB0");
	mock_full.mock_spt_full[1].mock_spt.partition[2].offset =
		(RSU_OSAL_U64)&mock_full.mock_cpb_full[0].mock_cpb;
	mock_full.mock_spt_full[1].mock_spt.partition[2].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[3].name, "CPB1");
	mock_full.mock_spt_full[1].mock_spt.partition[3].offset =
		(RSU_OSAL_U64)&mock_full.mock_cpb_full[1].mock_cpb;
	mock_full.mock_spt_full[1].mock_spt.partition[3].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	mock_full.mock_cpb_full[1].mock_cpb.header.magic_number = CPB_MAGIC_NUMBER;
	mock_full.mock_cpb_full[1].mock_cpb.header.header_size = CPB_HEADER_SIZE;
	mock_full.mock_cpb_full[1].mock_cpb.header.cpb_size = (RSU_OSAL_S32)4096;

	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 0);

	char c[] = "dummy";
	ret = rsu_slot_by_name(c);
	ASSERT_NE(ret, 0);

	int slot = 1;
	struct rsu_slot_info info;
	ret = rsu_slot_get_info((int)slot, &info);
	ASSERT_NE(ret, 0);

	ret = rsu_slot_size((int)slot);
	ASSERT_NE(ret, 0);

	ret = rsu_slot_priority((int)slot);
	ASSERT_NE(ret, 0);

	ret = rsu_slot_erase((int)slot);
	ASSERT_NE(ret, 0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}

/*
 * test case to do initialization with one Application slot:
 * providing valid slot number as 4(SPT1,SPT0,CPB1,CPB0 occupied first 4 slots(index starts with 0))
 * povided with valid SPT1, CPB0, CPB1
 * expecting it to recover SPT0
 * test case to check slot count successfully
 * test case to check slot by name with dummy name and expecting it to be successfull
 * test case to check slot info by providing invalid slot number expecting it to be successfull
 * test case to check slot priority by providing invalid slot number expecting it to be successfull
 * test case to check slot size by providing invalid slot number expecting it to be successfull
 * test case to check slot erase by providing invalid slot number expecting it to be successfull
 * performing exit for every init test case
 */
TEST(librsu_test3, test_one_slot)
{
    int ret = 0;

	mock_full.mock_spt_full[1].mock_spt.magic_number = SPT_MAGIC_NUMBER;
	mock_full.mock_spt_full[1].mock_spt.version = (RSU_OSAL_U32)1;
	char *spt_data;
	spt_data = (char *)malloc(sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[1].mock_spt.checksum = (RSU_OSAL_U32)0xFFFFFFFF;
	memcpy(spt_data, &mock_full.mock_spt_full[1].mock_spt, sizeof(struct SUB_PARTITION_TABLE));
	memset(spt_data + SPT_CHECKSUM_OFFSET, 0, sizeof(mock_full.mock_spt_full[1].mock_spt.checksum));
	swap_bits(spt_data, sizeof(struct SUB_PARTITION_TABLE));
	RSU_OSAL_U32 calc_crc =
		rsu_crc32(0, (RSU_OSAL_U8*)spt_data, sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[1].mock_spt.checksum = swap_endian32(calc_crc);
	swap_bits(spt_data, sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[1].mock_spt.magic_number = SPT_MAGIC_NUMBER;
	free(spt_data);

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)5;
	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[0].name, "SPT0");
	mock_full.mock_spt_full[1].mock_spt.partition[0].offset = (RSU_OSAL_U64)&mock_full.mock_spt_full[0].mock_spt;
	mock_full.mock_spt_full[1].mock_spt.partition[0].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[1].name, "SPT1");
	mock_full.mock_spt_full[1].mock_spt.partition[1].offset = (RSU_OSAL_U64)&mock_full.mock_spt_full[1].mock_spt;
	mock_full.mock_spt_full[1].mock_spt.partition[1].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[2].name, "CPB0");
	mock_full.mock_spt_full[1].mock_spt.partition[2].offset = (RSU_OSAL_U64)&mock_full.mock_cpb_full[0].mock_cpb;
	mock_full.mock_spt_full[1].mock_spt.partition[2].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[3].name, "CPB1");
	mock_full.mock_spt_full[1].mock_spt.partition[3].offset = (RSU_OSAL_U64)&mock_full.mock_cpb_full[1].mock_cpb;
	mock_full.mock_spt_full[1].mock_spt.partition[3].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	strcpy(mock_full.mock_spt_full[1].mock_spt.partition[4].name, "SLOT1");
	mock_full.mock_spt_full[1].mock_spt.partition[4].offset = (RSU_OSAL_U64)(&mock_full.slot1);
	mock_full.mock_spt_full[1].mock_spt.partition[4].length =
		(RSU_OSAL_U32)sizeof(mock_full.slot1);

	mock_full.mock_cpb_full[1].mock_cpb.header.magic_number = CPB_MAGIC_NUMBER;
	mock_full.mock_cpb_full[1].mock_cpb.header.header_size = CPB_HEADER_SIZE;
	mock_full.mock_cpb_full[1].mock_cpb.header.cpb_size = (RSU_OSAL_S32)4096;
	mock_full.mock_cpb_full[1].mock_cpb.header.image_ptr_offset = (RSU_OSAL_U64)0x20;
	mock_full.mock_cpb_full[1].mock_cpb.image.imp_ptr[0] = (uint64_t)&mock_full.slot1;
	mock_full.mock_cpb_full[1].mock_cpb.header.image_ptr_slots = (RSU_OSAL_U32)1;

	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 1);

	char c[] = "SLOT1";
	ret = rsu_slot_by_name(c);
	ASSERT_EQ(ret, 0);

	int slot = 0;
	struct rsu_slot_info info;
	ret = rsu_slot_get_info((int)slot, &info);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_size((int)slot);
	printf("\n returned slot size %d\n", ret);
	ASSERT_EQ(ret, 28 * 1024);

	ret = rsu_slot_priority((int)slot);
	ASSERT_EQ(ret, 1);

	ret = rsu_slot_erase((int)slot);
	ASSERT_EQ(ret, 0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}
