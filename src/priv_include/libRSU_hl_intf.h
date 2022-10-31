/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef LIBRSU_HL_INTF_H
#define LIBRSU_HL_INTF_H

#include <libRSU_OSAL.h>
#include <libRSU_ll_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPT_MAGIC_NUMBER  (0x57713427)
#define SPT_VERSION	  (0)
#define SPT_FLAG_RESERVED (1)
#define SPT_FLAG_READONLY (2)

#define SPT_MAX_PARTITIONS	  (127)
#define SPT_PARTITION_NAME_LENGTH (16)
#define SPT_RSVD_LENGTH		  (4)

struct SUB_PARTITION_TABLE {
	RSU_OSAL_U32 magic_number;
	RSU_OSAL_U32 version;
	RSU_OSAL_U32 partitions;
	RSU_OSAL_U32 checksum;
	RSU_OSAL_U32 __RSVD[SPT_RSVD_LENGTH];

	struct {
		RSU_OSAL_CHAR name[SPT_PARTITION_NAME_LENGTH];
		RSU_OSAL_U64 offset;
		RSU_OSAL_U32 length;
		RSU_OSAL_U32 flags;
	} partition[SPT_MAX_PARTITIONS];
} __attribute__((__packed__));

#define CPB_MAGIC_NUMBER (0x57789609)
#define CPB_HEADER_SIZE	 (24)
#define CPB_BLOCK_SIZE	 (4 * 1024)

union CMF_POINTER_BLOCK {
	struct {
		RSU_OSAL_U32 magic_number;
		RSU_OSAL_U32 header_size;
		RSU_OSAL_U32 cpb_size;
		RSU_OSAL_U32 cpb_reserved;
		RSU_OSAL_U32 image_ptr_offset;
		RSU_OSAL_U32 image_ptr_slots;
	} header;
	RSU_OSAL_U8 data[CPB_BLOCK_SIZE];
} __attribute__((__packed__));

typedef RSU_OSAL_U64 CMF_POINTER;

struct partition {
	RSU_OSAL_INT (*count)(RSU_OSAL_VOID);
	RSU_OSAL_CHAR *(*name)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*offset)(RSU_OSAL_INT part_num, RSU_OSAL_U64 *offset);
	RSU_OSAL_INT (*factory_offset)(RSU_OSAL_U64 *factory_offset);
	RSU_OSAL_INT (*size)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*reserved)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*readonly)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*rename)(RSU_OSAL_INT part_num, RSU_OSAL_CHAR *name);
	RSU_OSAL_INT (*delete)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*create)(RSU_OSAL_CHAR *name, RSU_OSAL_U64 start, RSU_OSAL_SIZE size);
};

struct priority {
	RSU_OSAL_INT (*get)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*add)(RSU_OSAL_INT part_num);
	RSU_OSAL_INT (*remove)(RSU_OSAL_INT part_num);
};

struct data {
	RSU_OSAL_INT(*read)
	(RSU_OSAL_INT part_num, RSU_OSAL_INT offset, RSU_OSAL_INT bytes, RSU_OSAL_VOID *buf);
	RSU_OSAL_INT(*write)
	(RSU_OSAL_INT part_num, RSU_OSAL_INT offset, RSU_OSAL_INT bytes, RSU_OSAL_VOID *buf);
	RSU_OSAL_INT (*erase)(RSU_OSAL_INT part_num);
};

struct spt_ops {
	RSU_OSAL_INT (*restore_file)(RSU_OSAL_CHAR *name);
	RSU_OSAL_INT (*save_file)(RSU_OSAL_CHAR *name);
	RSU_OSAL_INT (*restore_buf)(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size);
	RSU_OSAL_INT (*save_buf)(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size);
	RSU_OSAL_INT (*corrupted)(RSU_OSAL_VOID);
};

struct cpb_ops {
	RSU_OSAL_INT (*empty)(RSU_OSAL_VOID);
	RSU_OSAL_INT (*restore_file)(RSU_OSAL_CHAR *name);
	RSU_OSAL_INT (*save_file)(RSU_OSAL_CHAR *name);
	RSU_OSAL_INT (*restore_buf)(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size);
	RSU_OSAL_INT (*save_buf)(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size);
	RSU_OSAL_INT (*corrupted)(RSU_OSAL_VOID);
};

struct misc_ops {
	RSU_OSAL_INT (*notify_sdm)(RSU_OSAL_U32);
	RSU_OSAL_INT (*rsu_status)(struct mbox_status_info *data);
	RSU_OSAL_INT (*rsu_set_address)(RSU_OSAL_U64 offset);
	RSU_OSAL_INT (*rsu_get_dcmf_status)(struct rsu_dcmf_status *data);
	RSU_OSAL_INT (*rsu_get_max_retry_count)(RSU_OSAL_U8 *rsu_max_retry);
	RSU_OSAL_INT (*rsu_get_dcmf_version)(struct rsu_dcmf_version *version);
};

struct files_ops {
	RSU_OSAL_FILE *(*open)(RSU_OSAL_CHAR *filename, RSU_filesys_flags_t flag);
	RSU_OSAL_INT (*read)(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file);
	RSU_OSAL_INT (*write)(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file);
	RSU_OSAL_INT(*fseek)
	(RSU_OSAL_OFFSET offset, RSU_filesys_whence_t whence, RSU_OSAL_FILE *file);
	RSU_OSAL_INT (*ftruncate)(RSU_OSAL_OFFSET length, RSU_OSAL_FILE *file);
	RSU_OSAL_INT (*close)(RSU_OSAL_FILE *file);
};
struct librsu_hl_intf {
	RSU_OSAL_VOID (*close)(RSU_OSAL_VOID);
	struct partition partition;
	struct priority priority;
	struct data data;
	struct spt_ops spt_ops;
	struct cpb_ops cpb_ops;
	struct misc_ops misc_ops;
	struct files_ops file;
};

struct database {
	RSU_OSAL_BOOL spt_corrupted;
	RSU_OSAL_BOOL cpb_corrupted;
	RSU_OSAL_BOOL cpb_fixed;
	RSU_OSAL_U32 cpb0_part;
	RSU_OSAL_U32 cpb1_part;
	/*mtd_part_offset will be 0 for systems which can access whole qspi  or will be changed to
	  offset of SPT0 for systems whose qspi start address points to SPT0*/
	RSU_OSAL_U64 mtd_part_offset;
	struct mbox_data_rsu_spt_address spt_addr;
	struct SUB_PARTITION_TABLE *spt;
	union CMF_POINTER_BLOCK *cpb;
	CMF_POINTER *cpb_slots;
	struct librsu_ll_intf *hal;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
