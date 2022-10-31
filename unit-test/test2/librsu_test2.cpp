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
#define SPT_MAX_PARTITIONS 127
#define CPB_MAGIC_NUMBER 0x57789609
#define CPB_HEADER_SIZE	 24

extern struct full mock_full;

/*
 * dummy test case
 */
TEST(librsu_test2, test_pass)
{
    ASSERT_EQ(1,1);
}

/*
 * test case to check init with a valid SPT1 table
 * expecting it to recover SPT0 during initialization
 * performing exit for every init test case
 */
TEST(librsu_test2, test_valid_spt1)
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

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)4;
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


	ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
	ASSERT_EQ(ret,0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}

/*
 * test case to check init with a valid SPT0 table
 * expecting it to recover SPT1 during initialization
 * performing exit for every init test case
 */
TEST(librsu_test2, test_valid_spt0)
{
    int ret = 0;

	mock_full.mock_spt_full[0].mock_spt.magic_number = SPT_MAGIC_NUMBER;
	mock_full.mock_spt_full[0].mock_spt.version = (RSU_OSAL_U32)1;
	char *spt_data;
	spt_data = (char *)malloc(sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[0].mock_spt.checksum = (RSU_OSAL_U32)0xFFFFFFFF;
	memcpy(spt_data, &mock_full.mock_spt_full[0].mock_spt, sizeof(struct SUB_PARTITION_TABLE));
	memset(spt_data + SPT_CHECKSUM_OFFSET, 0, sizeof(mock_full.mock_spt_full[0].mock_spt.checksum));
	swap_bits(spt_data, sizeof(struct SUB_PARTITION_TABLE));
	RSU_OSAL_U32 calc_crc =
		rsu_crc32(0, (RSU_OSAL_U8*)spt_data, sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[0].mock_spt.checksum = swap_endian32(calc_crc);
	swap_bits(spt_data, sizeof(struct SUB_PARTITION_TABLE));
	mock_full.mock_spt_full[0].mock_spt.magic_number = SPT_MAGIC_NUMBER;
	free(spt_data);

	mock_full.mock_spt_full[0].mock_spt.partitions = (RSU_OSAL_U32)4;
	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[0].name, "SPT0");
	mock_full.mock_spt_full[0].mock_spt.partition[0].offset = (RSU_OSAL_U64)&mock_full.mock_spt_full[0].mock_spt;
	mock_full.mock_spt_full[0].mock_spt.partition[0].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[1].name, "SPT1");
	mock_full.mock_spt_full[0].mock_spt.partition[1].offset = (RSU_OSAL_U64)&mock_full.mock_spt_full[1].mock_spt;
	mock_full.mock_spt_full[0].mock_spt.partition[1].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[2].name, "CPB0");
	mock_full.mock_spt_full[0].mock_spt.partition[2].offset = (RSU_OSAL_U64)&mock_full.mock_cpb_full[0].mock_cpb;
	mock_full.mock_spt_full[0].mock_spt.partition[2].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[3].name, "CPB1");
	mock_full.mock_spt_full[0].mock_spt.partition[3].offset = (RSU_OSAL_U64)&mock_full.mock_cpb_full[1].mock_cpb;
	mock_full.mock_spt_full[0].mock_spt.partition[3].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
	ASSERT_EQ(ret,0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}

/*
 * test case to check init with both valid SPT0, SPT1 tables
 * expecting it to verify both as same
 * performing exit for every init test case
 */
TEST(librsu_test2, test_valid_both_spt)
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

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)4;
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

	/* SPT0 */
	mock_full.mock_spt_full[0].mock_spt.version = (RSU_OSAL_U32)1;
	mock_full.mock_spt_full[0].mock_spt.checksum = swap_endian32(calc_crc);
	mock_full.mock_spt_full[0].mock_spt.magic_number = SPT_MAGIC_NUMBER;

	mock_full.mock_spt_full[0].mock_spt.partitions = (RSU_OSAL_U32)4;
	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[0].name, "SPT0");
	mock_full.mock_spt_full[0].mock_spt.partition[0].offset = (RSU_OSAL_U64)&mock_full.mock_spt_full[0].mock_spt;
	mock_full.mock_spt_full[0].mock_spt.partition[0].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[1].name, "SPT1");
	mock_full.mock_spt_full[0].mock_spt.partition[1].offset = (RSU_OSAL_U64)&mock_full.mock_spt_full[1].mock_spt;
	mock_full.mock_spt_full[0].mock_spt.partition[1].length =
		(RSU_OSAL_U32)sizeof(struct SUB_PARTITION_TABLE);

	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[2].name, "CPB0");
	mock_full.mock_spt_full[0].mock_spt.partition[2].offset = (RSU_OSAL_U64)&mock_full.mock_cpb_full[0].mock_cpb;
	mock_full.mock_spt_full[0].mock_spt.partition[2].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	strcpy(mock_full.mock_spt_full[0].mock_spt.partition[3].name, "CPB1");
	mock_full.mock_spt_full[0].mock_spt.partition[3].offset = (RSU_OSAL_U64)&mock_full.mock_cpb_full[1].mock_cpb;
	mock_full.mock_spt_full[0].mock_spt.partition[3].length =
		(RSU_OSAL_U32)sizeof(union CMF_POINTER_BLOCK);

	ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
	ASSERT_EQ(ret,0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}

/*
 * test case to check init with a valid CPB1 table
 * expecting it to recover CPB0 during initialization
 * performing exit for every init test case
 */
TEST(librsu_test2, test_valid_cpb1)
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

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)4;
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

	mock_full.mock_cpb_full[1].mock_cpb.header.magic_number = CPB_MAGIC_NUMBER;
	mock_full.mock_cpb_full[1].mock_cpb.header.header_size = CPB_HEADER_SIZE;
	mock_full.mock_cpb_full[1].mock_cpb.header.cpb_size = (RSU_OSAL_S32)4096;


	ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
	ASSERT_EQ(ret,0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}

/*
 * test case to check init with a valid CPB0 table
 * expecting it to recover CPB1 during initialization
 * performing exit for every init test case
 */
TEST(librsu_test2, test_valid_cpb0)
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

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)4;
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


	mock_full.mock_cpb_full[0].mock_cpb.header.magic_number = CPB_MAGIC_NUMBER;
	mock_full.mock_cpb_full[0].mock_cpb.header.header_size = CPB_HEADER_SIZE;
	mock_full.mock_cpb_full[0].mock_cpb.header.cpb_size = (RSU_OSAL_S32)4096;

	ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
	ASSERT_EQ(ret,0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}


/*
 * test case to check init with both valid CPB0, CPB1 tables
 * expecting it to verify both as same
 * performing exit for every init test case
 */
TEST(librsu_test2, test_valid_both_cpb)
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

	mock_full.mock_spt_full[1].mock_spt.partitions = (RSU_OSAL_U32)4;
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

	mock_full.mock_cpb_full[1].mock_cpb.header.magic_number = CPB_MAGIC_NUMBER;
	mock_full.mock_cpb_full[1].mock_cpb.header.header_size = CPB_HEADER_SIZE;
	mock_full.mock_cpb_full[1].mock_cpb.header.cpb_size = (RSU_OSAL_S32)4096;

	mock_full.mock_cpb_full[0].mock_cpb.header.magic_number = CPB_MAGIC_NUMBER;
	mock_full.mock_cpb_full[0].mock_cpb.header.header_size = CPB_HEADER_SIZE;
	mock_full.mock_cpb_full[0].mock_cpb.header.cpb_size = (RSU_OSAL_S32)4096;

	ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
	ASSERT_EQ(ret,0);

	librsu_exit();

	memset(&mock_full, 0, sizeof(struct full));
}
