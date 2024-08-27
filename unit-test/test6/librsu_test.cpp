/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <gtest/gtest.h>
#include <libRSU.h>
#include <mock.h>

static void create_initial_qspi_image(void)
{
	/*make a copy of the test rpd file for testing, use the copy of unit-test operation*/
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

static FILE *fptr;

static int file_callback_init(void)
{
	fptr = NULL;
	fptr = fopen("dependency/app.rpd", "rb");
	return -errno;
}

static void file_callback_exit(void)
{
	fclose(fptr);
}

static RSU_OSAL_INT data_callback_file(RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	return fread(buf, 1, size, fptr);
}

TEST(librsu_test6, test_callback_program_verify)
{
	create_initial_qspi_image();
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"P4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P4");
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_erase(4);
	ASSERT_EQ(ret, 0);

	ret = file_callback_init();
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_callback(4, data_callback_file);
	ASSERT_EQ(ret, 0);

	file_callback_exit();

	ret = file_callback_init();
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_callback(4, data_callback_file);
	ASSERT_EQ(ret, 0);

	file_callback_exit();

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 5);

	librsu_exit();
}

static uint8_t *buffer;
static uint32_t buf_size;
static uint32_t offset;

static int buf_callback_init(void)
{
	int ret;
	offset = 0;
	FILE *fp = fopen("dependency/fip.bin", "rb");
	if (errno) {
		return -errno;
	}
	fseek(fp, 0, SEEK_END);
	buf_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer = (uint8_t *)malloc(buf_size);
	if (buffer == NULL) {
		return -ENOMEM;
	}
	ret = fread(buffer, 1, buf_size, fp);
	if(ret < 0) {
		return ret;
	}
	fclose(fp);
	return 0;
}

static void buf_callback_exit(void)
{
	free(buffer);
}

static RSU_OSAL_INT data_callback_buf(RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	if (buf_size == offset) {
		return 0;
	} else if ((offset + size) <= buf_size) {
		memcpy(buf, (buffer + offset), size);
		offset += size;
		return size;
	} else {
		memcpy(buf, (buffer + offset), (buf_size - offset));
		offset += (buf_size - offset);
		return (buf_size - offset);
	}
}

TEST(librsu_test6, test_callback_program_verify_raw_buf)
{
	create_initial_qspi_image();
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"fip1", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"fip1");
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_erase(4);
	ASSERT_EQ(ret, 0);

	ret = buf_callback_init();
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_callback_raw(4, data_callback_buf);
	ASSERT_EQ(ret, 0);

	buf_callback_exit();

	ret = buf_callback_init();
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_callback_raw(4, data_callback_buf);
	ASSERT_EQ(ret, 0);

	buf_callback_exit();

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 5);

	librsu_exit();
}
