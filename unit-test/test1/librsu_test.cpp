/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <gtest/gtest.h>
#include <libRSU.h>
#include <hal/RSU_plat_qspi.h>
#include <hal/RSU_plat_mailbox.h>
#include <hal/RSU_plat_file.h>

#include <version.h>


/*
 * dummy test case
 */
TEST(librsu_test, test_pass)
{
    ASSERT_EQ(1,1);
}

/*
 * test case to check major and minor verion
 */
TEST(librsu_test, test_version)
{
    ASSERT_EQ(RSU_MAJOR_VERSION(rsu_get_version()), RSU_POC_VERSION_MAJOR);
    ASSERT_EQ(RSU_MINOR_VERSION(rsu_get_version()), RSU_POC_VERSION_MINOR);
}

/*
 * test case to check exit without init
 * void API(no return to validate) but exit should fail if performed without initialization
 */
TEST(librsu_test, test_exit_only)
{
    librsu_exit();
}

/*
 * test case to do initialization by passing rc file as a parameter
 * performing exit for every init test case
 */
TEST(librsu_test, test_init)
{
    int ret = 0;
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_EQ(ret,0);

    librsu_exit();

}

/*
 * test case to do initialization without passing rc file as a parameter
 * performing exit for every init test case
 */
TEST(librsu_test, test_init1)
{
    int ret = 0;
    ret = librsu_init((RSU_OSAL_CHAR*)"");
    ASSERT_NE(ret,0);

    librsu_exit();

}

/*
 * test case to check double initialization
 * 2nd initialization should fail(as we cant do init again if already initialized)
 * performing exit for every init test case
 */
TEST(librsu_test, test_double_init)
{
    int ret = 0;
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_EQ(ret,0);

    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_NE(ret,0);

    librsu_exit();
}

/*
 * test case to report HPS software execution stage as a 16bit number
 * checking notify without librsu init, it should fail
 */
TEST(librsu_test, test_notify)
{
    int ret = 0;
    errno=0;
    char *endptr;
    long val = strtoul("0x5A",&endptr,16);
    ret = rsu_notify((int)val);
    ASSERT_EQ(ret,-1);

    // librsu_exit();
}

/*
 * test case to report HPS software execution stage as a 16bit number
 * checking notify with librsu init, it should notify without any errors
 * performing exit for every init test case
 */
TEST(librsu_test, test_notify1)
{
    int ret = 0;
    errno=0;
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_EQ(ret,0);
    char *endptr;
    long val = strtoul("0x5A",&endptr,16);
    ret = rsu_notify((int)val);
    ASSERT_EQ(ret,0);

    librsu_exit();
}

/*
 * test case to Copy the SDM status log to info struct
  * checking status log without librsu init, it should fail
 */
TEST(librsu_test, test_status_log)
{
    int ret = 0;
    struct rsu_status_info test ;
    ret = rsu_status_log(&test);
    ASSERT_EQ(ret,-1);
}

/*
 * test case to Copy the SDM status log to info struct
  * checking status log with librsu init, it should fill info struct
  * performing exit for every init test case
 */
TEST(librsu_test, test_status_log1)
{
    int ret = 0;
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_EQ(ret,0);
    struct rsu_status_info test ;
    ret = rsu_status_log(&test);
    ASSERT_EQ(ret,0);

    librsu_exit();
}

/*
 * test case to reset the retry counter
  * checking reset without librsu init, it should fail
 */
TEST(librsu_test, test_reset_retry)
{
    int ret = 0;
    ret = rsu_reset_retry_counter();
    ASSERT_NE(ret,0);
}

/*
 * test case to reset the retry counter
  * checking reset with librsu init, it should reset th4e retry value
  * performing exit for every init test case
 */
TEST(librsu_test, test_reset_retry1)
{
    int ret = 0;
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_EQ(ret,0);
    ret = rsu_reset_retry_counter();
    ASSERT_EQ(ret,0);

    librsu_exit();
}

/*
 * test case to clear errors from the current status log
  * checking clear error without librsu init, it should fail
 */
TEST(librsu_test, test_clear_error_status)
{
    int ret = 0;
    ret = rsu_clear_error_status();
    ASSERT_NE(ret,0);
}

/*
 * test case to clear errors from the current status log
  * checking clear error with librsu init, it should clear the error status log
 */
TEST(librsu_test, test_clear_error_status1)
{
    int ret = 0;
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ASSERT_EQ(ret,0);
    ret = rsu_clear_error_status();
    ASSERT_EQ(ret,0);

    librsu_exit();
}

TEST(librsu_test, dcmf_status)
{
    int ret = 0;
    RSU_OSAL_INT status[4];
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ret = rsu_dcmf_status(status);
    ASSERT_EQ(ret,0);
    librsu_exit();
}

TEST(librsu_test, dcmf_version)
{
    int ret = 0;
    RSU_OSAL_U32 version[4];
    ret = librsu_init((RSU_OSAL_CHAR*)"librsu_config.rc");
    ret = rsu_dcmf_version(version);
    ASSERT_EQ(ret,0);
    librsu_exit();
}
