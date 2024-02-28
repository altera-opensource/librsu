/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <gtest/gtest.h>
#include <libRSU.h>

TEST(librsu_test4, test_init_slot_copy_verify)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"fip");
	ASSERT_EQ(ret, 9);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P3");
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_copy_to_file(4, (RSU_OSAL_CHAR *)"app_image.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file(4, (RSU_OSAL_CHAR *)"app_image.rpd");
	ASSERT_EQ(ret, 0);

	librsu_exit();
}

TEST(librsu_test4, test_init_slot_create_program)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_copy_to_file(4, (RSU_OSAL_CHAR *)"app_image1.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_delete(4);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 3);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"P4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P4");
	ASSERT_EQ(ret, 9);

	ret = rsu_slot_erase(9);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_file(9, (RSU_OSAL_CHAR *)"app_image1.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file(9, (RSU_OSAL_CHAR *)"app_image1.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	librsu_exit();
}

TEST(librsu_test4, test_init_slot_rename)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_rename(4, (RSU_OSAL_CHAR *)"P5");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P5");
	ASSERT_EQ(ret, 4);

	librsu_exit();
}

TEST(librsu_test4, test_init_slot_parameters)
{
	int ret = 0, slot;
	struct rsu_slot_info info;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P3");
	ASSERT_EQ(ret, 4);

	slot = ret;

	ret = rsu_slot_priority(slot);
	ASSERT_EQ(ret, 3);

	ret = rsu_slot_size(slot);
	ASSERT_EQ(ret, 1048576); // check output_file_jic.map

	ret = rsu_slot_get_info(slot, &info);
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(info.name, "P3");
	ASSERT_EQ(info.priority, 3);
	ASSERT_EQ(info.size, 1048576);

	librsu_exit();
}

TEST(librsu_test4, test_init_slot_raw_program_verify)
{
	int ret = 0, slot;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_copy_to_file(4, (RSU_OSAL_CHAR *)"app_image_raw.bin");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"raw4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"raw4");
	ASSERT_EQ(ret, 10);

	slot = 10;

	ret = rsu_slot_erase(slot);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_file_raw(slot, (RSU_OSAL_CHAR *)"app_image_raw.bin");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file_raw(slot, (RSU_OSAL_CHAR *)"app_image_raw.bin");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 5);

	librsu_exit();
}

TEST(librsu_test4, test_init_slot_priority)
{
	int ret = 0, slot2, slot3;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	slot2 = rsu_slot_by_name((RSU_OSAL_CHAR *)"P2");
	ASSERT_EQ(slot2, 3);

	slot3 = rsu_slot_by_name((RSU_OSAL_CHAR *)"P3");
	ASSERT_EQ(slot3, 4);

	ret = rsu_slot_priority(slot2);
	ASSERT_EQ(ret, 2);

	ret = rsu_slot_priority(slot3);
	ASSERT_EQ(ret, 3);

	ret = rsu_slot_disable(slot2);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_priority(slot3);
	ASSERT_EQ(ret, 2);

	ret = rsu_slot_enable(slot2);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_priority(slot2);
	ASSERT_EQ(ret, 1);

	ret = rsu_slot_priority(slot3);
	ASSERT_EQ(ret, 3);

	ret = rsu_slot_enable(slot2);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_priority(slot2);
	ASSERT_EQ(ret, 1);

	ret = rsu_slot_priority(slot3);
	ASSERT_EQ(ret, 3);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P2");
	ASSERT_EQ(ret, 3);

	librsu_exit();
}

/**
 * app1 was created by quartus_pfg tool for relocatable image.
 * quartus_pfg -c factory.sof app1.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o start_address=0x00000
 * -o bitswap=OFF
 */
TEST(librsu_test4, test_init_slot_app1)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"P4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P4");
	ASSERT_EQ(ret, 10);

	ret = rsu_slot_erase(10);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_file(10, (RSU_OSAL_CHAR *)"dependency/app1.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file(10, (RSU_OSAL_CHAR *)"dependency/app1.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 5);

	librsu_exit();
}

/**
 * app2 was created by quartus_pfg tool for absolute image
 * quartus_pfg -c factory.sof app2.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o start_address=0x5000000
 * -o bitswap=OFF
 */
TEST(librsu_test4, test_init_slot_app2)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"P4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P4");
	ASSERT_EQ(ret, 10);

	ret = rsu_slot_erase(10);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_file(10, (RSU_OSAL_CHAR *)"dependency/app2.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file(10, (RSU_OSAL_CHAR *)"dependency/app2.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 5);

	librsu_exit();
}

/**
 * app3 was created by quartus_pfg tool for relative or location-invariant image
 * quartus_pfg -c factory.sof app1.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o bitswap=OFF
 */
TEST(librsu_test4, test_init_slot_app3)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"P4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P4");
	ASSERT_EQ(ret, 10);

	ret = rsu_slot_erase(10);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_file(10, (RSU_OSAL_CHAR *)"dependency/app3.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file(10, (RSU_OSAL_CHAR *)"dependency/app3.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 5);

	librsu_exit();
}

TEST(librsu_test, rsu_max_retry)
{
	int ret = 0;
	RSU_OSAL_U8 retry;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);
	ret = rsu_max_retry(&retry);
	ASSERT_EQ(ret, 0);
	librsu_exit();
}

TEST(librsu_test, rsu_running_factory)
{
	int ret = 0;
	RSU_OSAL_INT factory = 1;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_running_factory(&factory);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(factory, 0);
	librsu_exit();
}

TEST(librsu_test, rsu_slot_load_factory_after_reboot)
{
	int ret = 0;
	RSU_OSAL_U8 retry;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P3");
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_load_after_reboot(4);
	ASSERT_EQ(ret, 0);
	librsu_exit();
}

TEST(librsu_test, rsu_slot_load_after_reboot)
{
	int ret = 0;
	RSU_OSAL_U8 retry;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ret = rsu_slot_load_factory_after_reboot();
	ASSERT_EQ(ret, 0);
	librsu_exit();
}

TEST(librsu_test4, test_init_slot_create_program_buf)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	RSU_OSAL_SIZE size = rsu_slot_size(4);
	ASSERT_GT(size, 0);

	RSU_OSAL_U8 *buffer = (RSU_OSAL_U8 *)malloc(size);
	ASSERT_NE(buffer, (RSU_OSAL_U8 *)NULL);

	ret = rsu_slot_copy_to_buf(4, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_delete(4);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 3);

	ret = rsu_slot_create((RSU_OSAL_CHAR *)"P4", 0x640000, 1048576);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"P4");
	ASSERT_EQ(ret, 9);

	ret = rsu_slot_erase(9);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_buf(9, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_buf(9, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	free(buffer);

	librsu_exit();
}

TEST(librsu_test4, test_init_slot_create_program_raw_buf)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_by_name((RSU_OSAL_CHAR *)"fip");
	ASSERT_EQ(ret, 9);

	FILE *fp = fopen("dependency/fip.bin", "rb+");
	ASSERT_NE(fp, (FILE *)NULL);

	fseek(fp, 0, SEEK_END);
	RSU_OSAL_SIZE size = ftell(fp);

	RSU_OSAL_U8 *buffer = (RSU_OSAL_U8 *)malloc(size);
	ASSERT_NE(buffer, (RSU_OSAL_U8 *)NULL);

	fseek(fp, 0, SEEK_SET);
	ret = fread(buffer, size, 1, fp);
	ASSERT_GT(ret,0);
	fclose(fp);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_erase(9);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_buf_raw(9, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_buf_raw(9, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	free(buffer);

	librsu_exit();
}

/**
 * generate factory_upgrade image using quartus
 * quartus_pfg -c factory.sof factory_update.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o bitswap=OFF
 * -o rsu_upgrade=ON
 */

TEST(librsu_test4, program_factory_upgrade_buf)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	FILE *fp = fopen("dependency/factory_update.rpd", "rb+");
	ASSERT_NE(fp, (FILE *)NULL);

	fseek(fp, 0, SEEK_END);
	RSU_OSAL_SIZE size = ftell(fp);

	RSU_OSAL_U8 *buffer = (RSU_OSAL_U8 *)malloc(size);
	ASSERT_NE(buffer, (RSU_OSAL_U8 *)NULL);

	fseek(fp, 0, SEEK_SET);
	ret = fread(buffer, size, 1, fp);
	ASSERT_GT(ret,0);
	fclose(fp);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_erase(3);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_factory_update_buf(3, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_buf(3, buffer, size);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	free(buffer);

	librsu_exit();
}

/**
 * generate factory_upgrade image using quartus
 * quartus_pfg -c factory.sof factory_update.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o bitswap=OFF
 * -o rsu_upgrade=ON
 */

TEST(librsu_test4, program_factory_upgrade_file)
{
	int ret = 0;
	ret = librsu_init((RSU_OSAL_CHAR *)"librsu_config.rc");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	ret = rsu_slot_erase(3);
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_program_factory_update_file(3, (RSU_OSAL_CHAR *)(RSU_OSAL_CHAR *)"dependency/factory_update.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_verify_file(3, (RSU_OSAL_CHAR *)"dependency/factory_update.rpd");
	ASSERT_EQ(ret, 0);

	ret = rsu_slot_count();
	ASSERT_EQ(ret, 4);

	librsu_exit();
}
