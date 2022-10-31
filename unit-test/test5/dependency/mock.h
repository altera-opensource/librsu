/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef __MOCK_H_
#define __MOCK_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <hal/RSU_plat_mailbox.h>

#define RPD_FILE_NAME		"dependency/output_file_jic.rpd"
#define RPD_FILE_TEST_NAME	"dependency/output_file_jic_test.rpd"

extern struct mbox_status_info gdata;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
