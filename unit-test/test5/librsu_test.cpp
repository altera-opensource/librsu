/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <gtest/gtest.h>
#include <libRSU.h>
#include <mock.h>

#define STATE_DCIO_CORRUPTED	  (0xF004D00FUL)
#define STATE_CPB0_CORRUPTED	  (0xF004D010UL)
#define STATE_CPB0_CPB1_CORRUPTED (0xF004D011UL)

static void corrupt_qspi(uint64_t offset)
{
	uint32_t random = 0x12345678;
	FILE *qspi_file = fopen(RPD_FILE_TEST_NAME, "rb+");
	fseek(qspi_file, offset, SEEK_SET);
	fwrite(&random, 1, sizeof(random), qspi_file);
	fclose(qspi_file);
}

static void create_initial_qspi_image(void)
{
	/*make a copy of the test rpd file for testing, use the copy of unit-test operation*/
	memset(&gdata, 0, sizeof(gdata));
	FILE *file = fopen(RPD_FILE_NAME, "rb");
	FILE *qspi_file = fopen(RPD_FILE_TEST_NAME, "wb");
	char buff[4096];
	int n;

	while ((n = fread(buff, 1, 4096, file)) > 0) {
		fwrite(buff, 1, n, qspi_file);
	}

	fclose(file);
	fclose(qspi_file);
}

TEST(librsu_test5, no_spt_corruption)
{
	create_initial_qspi_image();

	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	librsu_exit();
}

TEST(librsu_test5, corrupt_spt1)
{
	create_initial_qspi_image();
	corrupt_qspi(0x00318000);
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	librsu_exit();
}

TEST(librsu_test5, corrupt_spt0)
{
	create_initial_qspi_image();
	corrupt_qspi(0x00310000);
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	librsu_exit();
}

TEST(librsu_test5, corrupt_spt0_spt1)
{
	create_initial_qspi_image();
	corrupt_qspi(0x00310000);
	corrupt_qspi(0x00318000);
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_NE(ret, 4);

	librsu_exit();
}

TEST(librsu_test5, save_restore_spt)
{
	create_initial_qspi_image();
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_save_spt((RSU_OSAL_CHAR *)"save.spt");
	ASSERT_EQ(ret, 0);

	librsu_exit();

	corrupt_qspi(0x00310000);
	corrupt_qspi(0x00318000);

	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_NE(ret, 4);

	ret = rsu_restore_spt((RSU_OSAL_CHAR *)"save.spt");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	librsu_exit();
}

TEST(librsu_test5, save_restore_empty_cpb)
{
	create_initial_qspi_image();
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_save_cpb((RSU_OSAL_CHAR *)"save.cpb");
	ASSERT_EQ(ret, 0);

	struct rsu_slot_info info;
	ret = rsu_slot_get_info(3, &info);
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(info.name, "P2");
	ASSERT_EQ(info.priority, 2);

	ret = rsu_create_empty_cpb();
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_get_info(3, &info);
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(info.name, "P2");
	ASSERT_EQ(info.priority, 0);

	librsu_exit();

	gdata.state = STATE_CPB0_CPB1_CORRUPTED;

	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_get_info(3, &info);
	ASSERT_NE(ret, 0);

	ret = rsu_restore_cpb((RSU_OSAL_CHAR *)"save.cpb");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_get_info(3, &info);
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(info.name, "P2");

	librsu_exit();
}

TEST(librsu_test5, save_restore_spt_buf)
{
	create_initial_qspi_image();
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	RSU_OSAL_SIZE size = (4096 + 4);
	RSU_OSAL_U8 *spt = (RSU_OSAL_U8 *)malloc(size);
	ASSERT_NE(spt, (RSU_OSAL_U8 *)NULL);

	ret = rsu_save_spt_to_buf(spt, size);
	ASSERT_EQ(ret, 0);

	librsu_exit();

	corrupt_qspi(0x00310000);
	corrupt_qspi(0x00318000);

	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_NE(ret, 4);

	ret = rsu_restore_spt_from_buf(spt, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	free(spt);

	librsu_exit();
}

TEST(librsu_test5, save_restore_empty_cpb_buf)
{
	create_initial_qspi_image();
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	RSU_OSAL_SIZE size = (4096 + 4);
	RSU_OSAL_U8 *cpb = (RSU_OSAL_U8 *)malloc(size);
	ASSERT_NE(cpb, (RSU_OSAL_U8 *)NULL);

	ret = rsu_save_cpb_to_buf(cpb, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_create_empty_cpb();
	ASSERT_EQ(ret, 0);

	librsu_exit();

	gdata.state = STATE_CPB0_CORRUPTED;

	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_restore_cpb_from_buf(cpb, size);
	ASSERT_EQ(ret, 0);

	librsu_exit();
}
