/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU_cfg.h>
#include <libRSU.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <libRSU_ops.h>
#include <libRSU_misc.h>
#include <string.h>

#define STATE_DCIO_CORRUPTED	  (0xF004D00FUL)
#define STATE_CPB0_CORRUPTED	  (0xF004D010UL)
#define STATE_CPB0_CPB1_CORRUPTED (0xF004D011UL)

#define SPT_SIZE	    ((RSU_OSAL_SIZE)4096)
#define SPT_CHECKSUM_OFFSET 0x0C

#define CPB_SIZE	     4096
#define CPB_IMAGE_PTR_OFFSET 32
#define CPB_IMAGE_PTR_NSLOTS 508

#define FACTORY_IMAGE_NAME "FACTORY_IMAGE"

#define ERASED_ENTRY ((RSU_OSAL_U64)(-1))
#define SPENT_ENTRY  ((RSU_OSAL_U64)(0))

static struct database *plat_database = NULL;

static RSU_OSAL_INT read_dev(RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *buf, RSU_OSAL_INT len)
{
	if (buf == NULL || len == 0) {
		RSU_LOG_ERR("Error in the arguments");
		return -EINVAL;
	}
	struct librsu_ll_intf *intf = plat_database->hal;

	return intf->qspi.read(offset, buf, len);
}

static RSU_OSAL_INT write_dev(RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *buf, RSU_OSAL_INT len)
{
	if (buf == NULL || len == 0) {
		return -EINVAL;
	}
	struct librsu_ll_intf *intf = plat_database->hal;

	return intf->qspi.write(offset, buf, len);
}

static RSU_OSAL_INT erase_dev(RSU_OSAL_OFFSET offset, RSU_OSAL_INT len)
{

	if (len == 0) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret;
	struct librsu_ll_intf *intf = plat_database->hal;

	ret = intf->qspi.erase(offset, len);
	if (ret < 0) {
		RSU_LOG_ERR("error: Erase error (errno=%i)", ret);
		return ret;
	}

	return 0;
}

/**
 * The SPT offset entry is the partition offset within the flash.  The MTD
 * device node maps a region starting with SPT0 which is not at the beginning
 * of flash. This is done so that data below the SPT0 in flash is not
 * exposed to librsu.  This function finds the partition offset within
 * the device file on linux.
 */
static RSU_OSAL_INT get_part_offset(RSU_OSAL_INT part_num, RSU_OSAL_OFFSET *offset)
{
	if (offset == NULL) {
		return -EINVAL;
	}

	if (part_num < 0 ||
	    part_num >= (RSU_OSAL_INT)plat_database->spt
				->partitions) { /*|| mtd_part_offset == 0 is removed, as this is
						   applicable in zephyr, check load_spt0_offset*/
		return -EINVAL;
	}

	if ((RSU_OSAL_S64)plat_database->spt->partition[part_num].offset <
	    (RSU_OSAL_S64)plat_database->mtd_part_offset) { /* why is this signed*/
		return -EFAULT;
	}

	*offset = (RSU_OSAL_OFFSET)(plat_database->spt->partition[part_num].offset -
				    plat_database->mtd_part_offset);

	return 0;
}

static RSU_OSAL_INT read_part(RSU_OSAL_INT part_num, RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *buf,
			      RSU_OSAL_INT len)
{
	if (buf == NULL || len == 0) {
		return -EINVAL;
	}

	RSU_OSAL_OFFSET part_offset;
	RSU_OSAL_INT ret;

	ret = get_part_offset(part_num, &part_offset);
	if (ret) {
		return ret;
	}

	if (offset < 0 || len < 0 ||
	    (offset + len) > plat_database->spt->partition[part_num].length) {
		return -ESPIPE;
	}

	return read_dev(part_offset + (RSU_OSAL_OFFSET)offset, buf, len);
}

static RSU_OSAL_INT write_part(RSU_OSAL_S32 part_num, RSU_OSAL_OFFSET offset, RSU_OSAL_VOID *buf,
			       RSU_OSAL_INT len)
{
	if (buf == NULL || len == 0) {
		return -EINVAL;
	}

	RSU_OSAL_OFFSET part_offset;
	RSU_OSAL_INT ret;

	ret = get_part_offset(part_num, &part_offset);
	if (ret) {
		return ret;
	}

	if (offset < 0 || len < 0 ||
	    (offset + len) > plat_database->spt->partition[part_num].length) {
		return -ESPIPE;
	}

	return write_dev(part_offset + (RSU_OSAL_OFFSET)offset, buf, len);
}

static RSU_OSAL_INT erase_part(RSU_OSAL_S32 part_num)
{
	RSU_OSAL_OFFSET part_offset;
	RSU_OSAL_INT ret;

	ret = get_part_offset(part_num, &part_offset);
	if (ret) {
		return ret;
	}

	return erase_dev(part_offset, plat_database->spt->partition[part_num].length);
}

static RSU_OSAL_INT load_spt0_offset(RSU_OSAL_VOID)
{
	RSU_OSAL_U32 x;

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strncmp(plat_database->spt->partition[x].name, "SPT0",
			    SPT_PARTITION_NAME_LENGTH) == 0) {
			if (plat_database->spt->partition[x].offset ==
			    plat_database->spt_addr.spt0_address) {
				/*mtd_part_offset will be 0 for system that can access the whole
				 * qspi*/
				plat_database->mtd_part_offset = 0;
			} else {
				plat_database->mtd_part_offset =
					plat_database->spt->partition[x].offset;
			}
			return 0;
		}
	}
	return -ENOENT;
}

static RSU_OSAL_INT check_both_spt(RSU_OSAL_VOID)
{
	RSU_OSAL_INT ret;
	RSU_OSAL_CHAR *spt0_data;
	RSU_OSAL_CHAR *spt1_data;

	spt0_data = (RSU_OSAL_CHAR *)rsu_malloc(SPT_SIZE);
	if (!spt0_data) {
		RSU_LOG_ERR("failed to allocate spt0_data");
		return -ENOMEM;
	}

	spt1_data = (RSU_OSAL_CHAR *)rsu_malloc(SPT_SIZE);
	if (!spt1_data) {
		RSU_LOG_ERR("failed to allocate spt1_data");
		rsu_free(spt0_data);
		return -ENOMEM;
	}

	ret = read_dev(plat_database->spt_addr.spt0_address, spt0_data, SPT_SIZE);
	if (ret) {
		RSU_LOG_ERR("failed to read spt0_data");
		rsu_free(spt1_data);
		rsu_free(spt0_data);
		return -EPERM;
	}

	ret = read_dev(plat_database->spt_addr.spt1_address, spt1_data, SPT_SIZE);
	if (ret) {
		RSU_LOG_ERR("failed to read spt1_data");
		rsu_free(spt1_data);
		rsu_free(spt0_data);
		return -EPERM;
	}

	ret = memcmp(spt0_data, spt1_data, SPT_SIZE);

	rsu_free(spt1_data);
	rsu_free(spt0_data);

	return ret;
}

/**
 * Make sure the SPT names are '\0' terminated. Truncate last byte if the
 * name uses all available bytes.  Perform validity check on entries.
 */
static RSU_OSAL_INT check_spt(RSU_OSAL_VOID)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_U32 y;
	RSU_OSAL_U32 max_len = SPT_PARTITION_NAME_LENGTH;
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_CHAR *spt_data;

	RSU_OSAL_BOOL spt0_found = false;
	RSU_OSAL_BOOL spt1_found = false;
	RSU_OSAL_BOOL cpb0_found = false;
	RSU_OSAL_BOOL cpb1_found = false;

	RSU_LOG_INF("MAX length of a name = %u bytes", max_len - 1);

	if (plat_database->spt->version > SPT_VERSION && librsu_cfg_spt_checksum_enabled()) {
		RSU_LOG_INF("check SPT checksum \n");
		spt_data = (RSU_OSAL_CHAR *)rsu_malloc(SPT_SIZE);
		if (!spt_data) {
			RSU_LOG_ERR("failed to allocate spt_data\n");
			return -ENOEXEC;
		}

		rsu_memcpy(spt_data, plat_database->spt, SPT_SIZE);
		rsu_memset(spt_data + SPT_CHECKSUM_OFFSET, 0, sizeof(plat_database->spt->checksum));

		/* calculate the checksum */
		swap_bits(spt_data, SPT_SIZE);
		calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)spt_data, SPT_SIZE);
		if (swap_endian32(plat_database->spt->checksum) != calc_crc) {
			RSU_LOG_ERR("Error, bad SPT checksum\n");
			rsu_free(spt_data);
			return -EBADF;
		}
		swap_bits(spt_data, SPT_SIZE);
		rsu_free(spt_data);
	}

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strnlen(plat_database->spt->partition[x].name, max_len) >= max_len) {
			plat_database->spt->partition[x].name[max_len - 1] = '\0';
		}

		RSU_LOG_INF("offset=0x%016llx, length=0x%08x\n",
			plat_database->spt->partition[x].offset,
			plat_database->spt->partition[x].length);

		/* check if the partition is overlap */
		RSU_OSAL_U64 s_start = plat_database->spt->partition[x].offset;
		RSU_OSAL_U64 s_end = plat_database->spt->partition[x].offset +
				     plat_database->spt->partition[x].length;

		for (y = 0; y < plat_database->spt->partitions; y++) {
			if (x == y) {
				continue;
			}

			/*
			 * don't allow the same partition name to appear
			 * more than once
			 */
			if (!(strncmp(plat_database->spt->partition[x].name,
				      plat_database->spt->partition[y].name,
				      SPT_PARTITION_NAME_LENGTH))) {
				RSU_LOG_ERR("partition name appears more than once");
				return -EEXIST;
			}

			RSU_OSAL_U64 d_start = plat_database->spt->partition[y].offset;
			RSU_OSAL_U64 d_end = plat_database->spt->partition[y].offset +
					     plat_database->spt->partition[y].length;

			if ((s_start < d_end) && (s_end > d_start)) {
				RSU_LOG_ERR("error: Partition overlap");
				return -ESPIPE;
			}
		}

		RSU_LOG_INF("%-16s %016llX - %016llX (%X)", plat_database->spt->partition[x].name,
			plat_database->spt->partition[x].offset,
			(plat_database->spt->partition[x].offset +
			 plat_database->spt->partition[x].length - 1),
			plat_database->spt->partition[x].flags);

		if (strncmp(plat_database->spt->partition[x].name, "SPT0",
			    SPT_PARTITION_NAME_LENGTH) == 0) {
			spt0_found = true;
		} else if (strncmp(plat_database->spt->partition[x].name, "SPT1",
				   SPT_PARTITION_NAME_LENGTH) == 0) {
			spt1_found = true;
		} else if (strncmp(plat_database->spt->partition[x].name, "CPB0",
				   SPT_PARTITION_NAME_LENGTH) == 0) {
			cpb0_found = true;
		} else if (strncmp(plat_database->spt->partition[x].name, "CPB1",
				   SPT_PARTITION_NAME_LENGTH) == 0) {
			cpb1_found = true;
		}
	}

	if (!spt0_found || !spt1_found || !cpb0_found || !cpb1_found) {
		RSU_LOG_ERR("Missing a critical entry in the SPT");
		return -ENXIO;
	}

	return 0;
}

/**
 * Check SPT1 and then SPT0. If they both pass checks, use SPT0.
 * If only one passes, retore the bad one. If both are bad, fail.
 */
static RSU_OSAL_INT load_spt(RSU_OSAL_VOID)
{
	struct SUB_PARTITION_TABLE *spt_ptr = plat_database->spt;
	RSU_OSAL_BOOL spt0_good = false;
	RSU_OSAL_BOOL spt1_good = false;
	RSU_OSAL_INT ret;
	struct librsu_ll_intf *intf = plat_database->hal;

	RSU_LOG_INF("reading SPT1");
	ret = read_dev(plat_database->spt_addr.spt1_address, spt_ptr,
		       sizeof(struct SUB_PARTITION_TABLE));
	if (ret != 0) {
		RSU_LOG_ERR("Failed to read qspi");
		return -EACCES;
	}

	if (spt_ptr->magic_number == SPT_MAGIC_NUMBER) {
		if (check_spt() == 0 && load_spt0_offset() == 0) {
			spt1_good = true;
		} else {
			RSU_LOG_ERR("SPT1 validity check failed");
		}
	} else {
		RSU_LOG_ERR("Bad SPT1 magic number 0x%08X", plat_database->spt->magic_number);
	}

	RSU_LOG_INF("reading SPT0");
	ret = read_dev(plat_database->spt_addr.spt0_address, spt_ptr,
		       sizeof(struct SUB_PARTITION_TABLE));
	if (ret != 0) {
		RSU_LOG_ERR("Failed to read qspi");
		return -EACCES;
	}

	if (spt_ptr->magic_number == SPT_MAGIC_NUMBER) {
		if (check_spt() == 0 && load_spt0_offset() == 0) {
			spt0_good = true;
		} else {
			RSU_LOG_ERR("SPT0 validity check failed");
		}
	} else {
		RSU_LOG_ERR("Bad SPT0 magic number 0x%08X", plat_database->spt->magic_number);
	}

	if (spt0_good && spt1_good) {
		if (check_both_spt()) {
			RSU_LOG_ERR("error: unmatched SPT0/1 data");
			plat_database->spt_corrupted = 1;
			return -EFAULT;
		}
		return 0;
	}

	if (spt0_good) {
		RSU_LOG_WRN("warning: Restoring SPT1");

		if (erase_dev(plat_database->spt_addr.spt1_address, 32 * 1024)) {
			RSU_LOG_ERR("error: Erase SPT1 region failed");
			return -EPERM;
		}

		plat_database->spt->magic_number = (RSU_OSAL_S32)0xFFFFFFFF;
		if (write_dev(plat_database->spt_addr.spt1_address, plat_database->spt,
			      sizeof(struct SUB_PARTITION_TABLE)) != 0) {
			RSU_LOG_ERR("error: Unable to write SPT1 table");
			return -EPERM;
		}

		plat_database->spt->magic_number = (RSU_OSAL_S32)SPT_MAGIC_NUMBER;
		if (intf->qspi.write(plat_database->spt_addr.spt1_address, plat_database->spt,
				     sizeof(plat_database->spt->magic_number)) != 0) {
			RSU_LOG_ERR("error: Unable to write SPT1 magic #");
			return -EPERM;
		}

		return 0;
	}

	if (spt1_good) {
		if (read_dev(plat_database->spt_addr.spt1_address, plat_database->spt,
			     sizeof(struct SUB_PARTITION_TABLE)) ||
		    plat_database->spt->magic_number != SPT_MAGIC_NUMBER || check_spt() ||
		    load_spt0_offset()) {
			RSU_LOG_ERR("error: Failed to load SPT1");
			return -EPERM;
		}

		RSU_LOG_WRN("warning: Restoring SPT0");

		if (erase_dev(plat_database->spt_addr.spt0_address, 32 * 1024)) {
			RSU_LOG_ERR("error: Erase SPT0 region failed");
			return -EPERM;
		}

		plat_database->spt->magic_number = (RSU_OSAL_S32)0xFFFFFFFF;
		if (write_dev(plat_database->spt_addr.spt0_address, plat_database->spt,
			      sizeof(struct SUB_PARTITION_TABLE)) != 0) {
			RSU_LOG_ERR("error: Unable to write SPT0 table");
			return -EPERM;
		}

		plat_database->spt->magic_number = (RSU_OSAL_S32)SPT_MAGIC_NUMBER;
		if (write_dev(plat_database->spt_addr.spt0_address, plat_database->spt,
			      sizeof(plat_database->spt->magic_number)) != 0) {
			RSU_LOG_ERR("error: Unable to write SPT0 magic #");
			return -EPERM;
		}

		return 0;
	}

	plat_database->spt_corrupted = true;
	RSU_LOG_ERR("error: No valid SPT0 or SPT1 found");
	return -EFAULT;
}

static RSU_OSAL_VOID rsu_qspi_close(RSU_OSAL_VOID)
{
	if (plat_database == NULL) {
		return;
	}

	if (plat_database->spt != NULL) {
		rsu_free(plat_database->spt);
	}

	if (plat_database->cpb != NULL) {
		rsu_free(plat_database->cpb);
	}

	rsu_free(plat_database);
	plat_database = NULL;
}

/**
 * check CPB other header value and image pointer
 */
static RSU_OSAL_INT check_cpb(RSU_OSAL_VOID)
{
	RSU_OSAL_U32 x, y;

	if (plat_database->cpb->header.header_size > CPB_HEADER_SIZE) {
		RSU_LOG_WRN("warning: CPB header is larger than expected");
	}

	for (x = 0; x < plat_database->cpb->header.image_ptr_slots; x++) {
		if (plat_database->cpb_slots[x] == ERASED_ENTRY ||
		    plat_database->cpb_slots[x] == SPENT_ENTRY) {
			continue;
		}

		for (y = 0; y < plat_database->spt->partitions; y++) {
			if (plat_database->cpb_slots[x] ==
			    plat_database->spt->partition[y].offset) {
				RSU_LOG_INF("cpb_slots[%u] = %s", x,
					plat_database->spt->partition[y].name);
				break;
			}
		}

		if (y >= plat_database->spt->partitions) {
			RSU_LOG_ERR("error: CPB is not included in SPT");
			RSU_LOG_ERR("cpb_slots[%u] = %016llX ???", x, plat_database->cpb_slots[x]);
			return -EFAULT;
		}

		if (plat_database->spt->partition[y].flags & SPT_FLAG_RESERVED) {
			RSU_LOG_ERR("CPB is included in SPT but reserved\n");
			return -EPERM;
		}
	}

	return 0;
}

static RSU_OSAL_INT check_both_cpb(RSU_OSAL_VOID)
{
	RSU_OSAL_INT ret;
	RSU_OSAL_CHAR *cpb0_data;
	RSU_OSAL_CHAR *cpb1_data;

	cpb0_data = (RSU_OSAL_CHAR *)rsu_malloc(CPB_SIZE);
	if (!cpb0_data) {
		RSU_LOG_ERR("failed to allocate cpb0_data");
		return -ENOMEM;
	}

	cpb1_data = (RSU_OSAL_CHAR *)rsu_malloc(CPB_SIZE);
	if (!cpb1_data) {
		RSU_LOG_ERR("failed to allocate cpb1_data");
		rsu_free(cpb0_data);
		return -ENOMEM;
	}

	ret = read_part(plat_database->cpb0_part, 0, cpb0_data, CPB_SIZE);
	if (ret) {
		RSU_LOG_ERR("failed to read cpb0_data");
		rsu_free(cpb1_data);
		rsu_free(cpb0_data);
		return ret;
	}

	ret = read_part(plat_database->cpb1_part, 0, cpb1_data, CPB_SIZE);
	if (ret) {
		RSU_LOG_ERR("failed to read cpb1_data");
		rsu_free(cpb1_data);
		rsu_free(cpb0_data);
		return ret;
	}

	ret = memcmp(cpb0_data, cpb1_data, CPB_SIZE);
	rsu_free(cpb1_data);
	rsu_free(cpb0_data);
	return ret;
}

/**
 * Check CPB1 and then CPB0. If they both pass checks, use CPB0.
 * If only one passes, retore the bad one. If both are bad, set
 * CPB_CORRUPTED flag to true.
 *
 * When CPB_CORRUPTED flag is true, all CPB operations are blocked
 * except restore_cpb and empty_cpb.
 */
static RSU_OSAL_INT load_cpb(RSU_OSAL_VOID)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT ret;
	RSU_OSAL_BOOL cpb0_good = false;
	RSU_OSAL_BOOL cpb1_good = false;

	struct mbox_status_info info;
	RSU_OSAL_BOOL cpb0_corrupted = false;
	struct librsu_ll_intf *intf = plat_database->hal;

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	ret = intf->mbox.get_rsu_status(&info);
	if (ret != 0) {
		RSU_LOG_ERR("Error in retriving rsu status");
		return -EACCES;
	}

	RSU_LOG_INF("state=0x%08llX\n", info.state);

	if (plat_database->cpb_fixed == false && info.state == STATE_CPB0_CPB1_CORRUPTED) {
		RSU_LOG_ERR("FW detects both CPBs corrupted\n");
		plat_database->cpb_corrupted = true;
		return -ECORRUPTED_CPB;
	}

	if (plat_database->cpb_fixed == false && info.state == STATE_CPB0_CORRUPTED) {
		RSU_LOG_ERR("FW detects corrupted CPB0, fine CPB1\n");
		cpb0_corrupted = true;
	}

	plat_database->cpb0_part = ~0;
	plat_database->cpb1_part = ~0;

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strncmp(plat_database->spt->partition[x].name, "CPB0",
			    SPT_PARTITION_NAME_LENGTH) == 0) {
			plat_database->cpb0_part = x;
		} else if (strncmp(plat_database->spt->partition[x].name, "CPB1",
				   SPT_PARTITION_NAME_LENGTH) == 0) {
			plat_database->cpb1_part = x;
		}

		if (plat_database->cpb0_part < plat_database->spt->partitions &&
		    plat_database->cpb1_part < plat_database->spt->partitions) {
			break;
		}
	}

	if (plat_database->cpb0_part > plat_database->spt->partitions ||
	    plat_database->cpb1_part > plat_database->spt->partitions) {
		RSU_LOG_ERR("error: Missing CPB0/1 partition");
		return -EFAULT;
	}

	if (read_part(plat_database->cpb1_part, 0, plat_database->cpb, CPB_BLOCK_SIZE) == 0 &&
	    plat_database->cpb->header.magic_number == CPB_MAGIC_NUMBER) {
		plat_database->cpb_slots =
			(CMF_POINTER *)&plat_database->cpb
				->data[plat_database->cpb->header.image_ptr_offset];
		if (check_cpb() == 0) {
			cpb1_good = true;
		}
	} else {
		RSU_LOG_ERR("Bad CPB1 is bad");
	}

	if (!cpb0_corrupted) {
		if (read_part(plat_database->cpb0_part, 0, plat_database->cpb, CPB_BLOCK_SIZE) ==
			    0 &&
		    plat_database->cpb->header.magic_number == CPB_MAGIC_NUMBER) {
			plat_database->cpb_slots =
				(CMF_POINTER *)&plat_database->cpb
					->data[plat_database->cpb->header.image_ptr_offset];
			if (check_cpb() == 0) {
				cpb0_good = true;
			}
		} else {
			RSU_LOG_ERR("CPB0 is bad");
		}
	}

	if (cpb0_good && cpb1_good) {
		if (check_both_cpb()) {
			RSU_LOG_ERR("error: unmatched CPB0/1 data");
			plat_database->cpb_corrupted = true;
			return -EBADF;
		}
		plat_database->cpb_slots =
			(CMF_POINTER *)&plat_database->cpb
				->data[plat_database->cpb->header.image_ptr_offset];
		return 0;
	}

	if (cpb0_good) {
		RSU_LOG_ERR("warning: Restoring CPB1");
		if (erase_part(plat_database->cpb1_part)) {
			RSU_LOG_ERR("error: Failed erase CPB1");
			return -EPERM;
		}

		plat_database->cpb->header.magic_number = (RSU_OSAL_S32)0xFFFFFFFF;
		if (write_part(plat_database->cpb1_part, 0, plat_database->cpb, CPB_BLOCK_SIZE)) {
			RSU_LOG_ERR("error: Unable to write CPB1 table");
			return -EPERM;
		}
		plat_database->cpb->header.magic_number = (RSU_OSAL_S32)CPB_MAGIC_NUMBER;
		if (write_part(plat_database->cpb1_part, 0, plat_database->cpb,
			       sizeof(plat_database->cpb->header.magic_number))) {
			RSU_LOG_ERR("error: Unable to write CPB1 magic number");
			return -EPERM;
		}

		plat_database->cpb_slots =
			(CMF_POINTER *)&plat_database->cpb
				->data[plat_database->cpb->header.image_ptr_offset];
		return 0;
	}

	if (cpb1_good) {
		if (read_part(plat_database->cpb1_part, 0, plat_database->cpb, CPB_BLOCK_SIZE) ||
		    plat_database->cpb->header.magic_number != CPB_MAGIC_NUMBER) {
			RSU_LOG_ERR("error: Unable to load CPB1");
			return -EACCES;
		}

		RSU_LOG_ERR("warning: Restoring CPB0");
		if (erase_part(plat_database->cpb0_part)) {
			RSU_LOG_ERR("error: Failed erase CPB0");
			return -EPERM;
		}

		plat_database->cpb->header.magic_number = (RSU_OSAL_S32)0xFFFFFFFF;
		if (write_part(plat_database->cpb0_part, 0, plat_database->cpb, CPB_BLOCK_SIZE)) {
			RSU_LOG_ERR("error: Unable to write CPB0 table");
			return -EPERM;
		}
		plat_database->cpb->header.magic_number = (RSU_OSAL_S32)CPB_MAGIC_NUMBER;
		if (write_part(plat_database->cpb0_part, 0, plat_database->cpb,
			       sizeof(plat_database->cpb->header.magic_number))) {
			RSU_LOG_ERR("error: Unable to write CPB0 magic number");
			return -EPERM;
		}

		plat_database->cpb_slots =
			(CMF_POINTER *)&plat_database->cpb
				->data[plat_database->cpb->header.image_ptr_offset];
		return 0;
	}

	plat_database->cpb_corrupted = true;
	RSU_LOG_ERR("error: found both corrupted CPBs");

	return -ECORRUPTED_CPB;
}

static RSU_OSAL_INT update_cpb(RSU_OSAL_INT slot, RSU_OSAL_U64 ptr)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT updates = 0;
	RSU_LOG_DBG("updating cpb");

	if (slot < 0 || (RSU_OSAL_U32)slot > plat_database->cpb->header.image_ptr_slots) {
		return -EINVAL;
	}

	if ((plat_database->cpb_slots[slot] & ptr) != ptr) {
		return -EINVAL;
	}

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	plat_database->cpb_slots[slot] = ptr;

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (!((strncmp(plat_database->spt->partition[x].name, "CPB0",
			       SPT_PARTITION_NAME_LENGTH) == 0) ||
		      (strncmp(plat_database->spt->partition[x].name, "CPB1",
			       SPT_PARTITION_NAME_LENGTH) == 0))) {
			continue;
		}

		if (write_part(x, 0, plat_database->cpb, CPB_BLOCK_SIZE)) {
			return -EIO;
		}
		updates++;
	}

	if (updates != 2) {
		RSU_LOG_ERR("error: Did not find two CPBs");
		return -EADDRNOTAVAIL;
	}

	return 0;
}

static RSU_OSAL_INT writeback_cpb(RSU_OSAL_VOID)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT updates = 0;
	RSU_OSAL_INT err;

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strncmp(plat_database->spt->partition[x].name, "CPB0",
			    SPT_PARTITION_NAME_LENGTH) &&
		    strncmp(plat_database->spt->partition[x].name, "CPB1",
			    SPT_PARTITION_NAME_LENGTH)) {
			continue;
		}

		err = erase_part(x);
		if (err) {
			RSU_LOG_ERR("error: Unable to ease CPBx");
			return err;
		}

		plat_database->cpb->header.magic_number = (RSU_OSAL_S32)0xFFFFFFFF;
		err = write_part(x, 0, plat_database->cpb, CPB_BLOCK_SIZE);
		if (err) {
			RSU_LOG_ERR("error: Unable to write CPBx table");
			return err;
		}

		plat_database->cpb->header.magic_number = (RSU_OSAL_S32)CPB_MAGIC_NUMBER;
		err = write_part(x, 0, plat_database->cpb,
				 sizeof(plat_database->cpb->header.magic_number));
		if (err) {
			RSU_LOG_ERR("error: Unable to write CPBx magic number");
			return err;
		}

		updates++;
	}

	if (updates != 2) {
		RSU_LOG_ERR("error: Did not find two CPBs");
		return -EADDRNOTAVAIL;
	}

	return 0;
}

static RSU_OSAL_INT writeback_spt(RSU_OSAL_VOID)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT updates = 0;
	RSU_OSAL_CHAR *spt_data;
	RSU_OSAL_U32 calc_crc;

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (!((strncmp(plat_database->spt->partition[x].name, "SPT0",
			       SPT_PARTITION_NAME_LENGTH) == 0) ||
		      (strncmp(plat_database->spt->partition[x].name, "SPT1",
			       SPT_PARTITION_NAME_LENGTH) == 0))) {
			continue;
		}

		if (erase_part(x)) {
			RSU_LOG_ERR("error: Unable to ease SPTx");
			return -EIO;
		}

		if (plat_database->spt->version > SPT_VERSION &&
		    librsu_cfg_spt_checksum_enabled()) {
			RSU_LOG_WRN("update SPT checksum \n");
			spt_data = (RSU_OSAL_CHAR *)rsu_malloc(SPT_SIZE);
			if (!spt_data) {
				RSU_LOG_ERR("failed to allocate spt_data\n");
				return -ENOMEM;
			}

			plat_database->spt->checksum = (RSU_OSAL_U32)0;
			/* calculate the new checksum */
			rsu_memcpy(spt_data, plat_database->spt, SPT_SIZE);
			swap_bits(spt_data, SPT_SIZE);
			calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)spt_data, SPT_SIZE);
			plat_database->spt->checksum = swap_endian32(calc_crc);
			rsu_free(spt_data);
		}

		plat_database->spt->magic_number = (RSU_OSAL_S32)0xFFFFFFFF;
		if (write_part(x, 0, plat_database->spt, SPT_SIZE)) {
			RSU_LOG_ERR("error: Unable to write SPTx table");
			return -EIO;
		}

		plat_database->spt->magic_number = (RSU_OSAL_S32)SPT_MAGIC_NUMBER;
		if (write_part(x, 0, &(plat_database->spt->magic_number),
			       sizeof(plat_database->spt->magic_number))) {
			RSU_LOG_ERR("error: Unable to write SPTx magic #");
			return -EIO;
		}

		updates++;
	}

	if (updates != 2) {
		RSU_LOG_ERR("error: Did not find two SPTs");
		return -EADDRNOTAVAIL;
	}

	return 0;
}

static RSU_OSAL_INT notify_sdm(RSU_OSAL_U32 value)
{
	return plat_database->hal->mbox.rsu_notify(value);
}

static RSU_OSAL_INT rsu_status(struct mbox_status_info *data)
{
	return plat_database->hal->mbox.get_rsu_status(data);
}

static RSU_OSAL_INT rsu_set_address(RSU_OSAL_U64 offset)
{
	return plat_database->hal->mbox.send_rsu_update(offset);
}

static RSU_OSAL_INT rsu_get_dcmf_status(struct rsu_dcmf_status *data)
{
	return plat_database->hal->misc.rsu_get_dcmf_status(data);
}

static RSU_OSAL_INT rsu_get_max_retry_count(RSU_OSAL_U8 *rsu_max_retry)
{
	return plat_database->hal->misc.rsu_get_max_retry_count(rsu_max_retry);
}

static RSU_OSAL_INT rsu_get_dcmf_version(struct rsu_dcmf_version *version)
{
	return plat_database->hal->misc.rsu_get_dcmf_version(version);
}

static RSU_OSAL_INT partition_count(RSU_OSAL_VOID)
{
	return plat_database->spt->partitions;
}

static RSU_OSAL_CHAR *partition_name(RSU_OSAL_INT part_num)
{
	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		return "BAD";
	}

	return plat_database->spt->partition[part_num].name;
}

static RSU_OSAL_INT partition_offset(RSU_OSAL_INT part_num, RSU_OSAL_U64 *offset)
{
	if (offset == NULL ) {
		RSU_LOG_ERR("offset is NULL");
		return -EINVAL;
	}

	if (part_num < 0 ||
	    (RSU_OSAL_U32)part_num >= plat_database->spt->partitions) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	*offset = plat_database->spt->partition[part_num].offset;
	return 0;
}

/*
 * factory_offset() - get the offset of the factory image
 *
 * Return: offset on success, or -1 on error
 */
static RSU_OSAL_INT factory_offset(RSU_OSAL_U64 *factory_offset)
{
	RSU_OSAL_U32 x;

	if (factory_offset == NULL) {
		return -EINVAL;
	}

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EBADF;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strncmp(plat_database->spt->partition[x].name, FACTORY_IMAGE_NAME,
			    (SPT_PARTITION_NAME_LENGTH - 1)) == 0) {
			*factory_offset = plat_database->spt->partition[x].offset;
			RSU_LOG_INF("Got factory at %u with offset 0x%08llx", x, *factory_offset);
			return 0;
		}
	}

	RSU_LOG_ERR("Could not find the factory offset");
	return -EFAULT;
}

static RSU_OSAL_INT partition_size(RSU_OSAL_INT part_num)
{
	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	return plat_database->spt->partition[part_num].length;
}

static RSU_OSAL_INT partition_reserved(RSU_OSAL_INT part_num)
{
	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	return (plat_database->spt->partition[part_num].flags & SPT_FLAG_RESERVED) ? 1 : 0;
}

static RSU_OSAL_INT partition_readonly(RSU_OSAL_INT part_num)
{
	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	return (plat_database->spt->partition[part_num].flags & SPT_FLAG_READONLY) ? 1 : 0;
}

static RSU_OSAL_INT data_read(RSU_OSAL_INT part_num, RSU_OSAL_INT offset, RSU_OSAL_INT bytes,
			      RSU_OSAL_VOID *buf)
{
	return read_part(part_num, offset, buf, bytes);
}

static RSU_OSAL_INT data_write(RSU_OSAL_INT part_num, RSU_OSAL_INT offset, RSU_OSAL_INT bytes,
			       RSU_OSAL_VOID *buf)
{
	return write_part(part_num, offset, buf, bytes);
}

static RSU_OSAL_INT data_erase(RSU_OSAL_INT part_num)
{
	return erase_part(part_num);
}

static RSU_OSAL_INT partition_rename(RSU_OSAL_INT part_num, RSU_OSAL_CHAR *name)
{
	RSU_OSAL_U32 x;

	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	if (strnlen(name, SPT_PARTITION_NAME_LENGTH) >= SPT_PARTITION_NAME_LENGTH) {
		RSU_LOG_ERR("error: Partition name is too long - limited to %i",
			SPT_PARTITION_NAME_LENGTH - 1);
		return -EINVAL;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strncmp(plat_database->spt->partition[x].name, name,
			    SPT_PARTITION_NAME_LENGTH - 1) == 0) {
			RSU_LOG_ERR("error: Partition rename already in use");
			return -EINVAL;
		}
	}

	SAFE_STRCPY(plat_database->spt->partition[part_num].name, SPT_PARTITION_NAME_LENGTH, name,
		    SPT_PARTITION_NAME_LENGTH);

	if (writeback_spt()) {
		return -EPERM;
	}

	if (load_spt()) {
		return -EPERM;
	}

	return 0;
}

/*
 * partition_delete() - Delete a partition.
 * part_num: partition number
 *
 * Returns 0 on success, or Error Code
 */
static RSU_OSAL_INT partition_delete(RSU_OSAL_INT part_num)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT err;

	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("error: Invalid partition number");
		return -EINVAL;
	}

	for (x = part_num; (x < plat_database->spt->partitions) && ((x + 1) < SPT_MAX_PARTITIONS);
	     x++) {
		plat_database->spt->partition[x] = plat_database->spt->partition[x + 1];
	}

	plat_database->spt->partitions--;

	err = writeback_spt();
	if (err) {
		RSU_LOG_ERR("SPT write_back failed");
		return err;
	}

	err = load_spt();
	if (err) {
		RSU_LOG_ERR("SPT load failed");
		return err;
	}

	return 0;
}

/*
 * partition_create() - Create a new partition.
 * name: partition name
 * start: partition start address
 * size: partition size
 *
 * Returns 0 on success, or Error Code
 */
static RSU_OSAL_INT partition_create(RSU_OSAL_CHAR *name, RSU_OSAL_U64 start, RSU_OSAL_SIZE size)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_U64 end = start + size;
	RSU_OSAL_INT err;

	if (strnlen(name, SPT_PARTITION_NAME_LENGTH) >= SPT_PARTITION_NAME_LENGTH) {
		RSU_LOG_ERR("error: Partition name is too long - limited to %i",
			SPT_PARTITION_NAME_LENGTH - 1);
		return -EINVAL;
	}

	if (plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("bigger than max partition\n");
		return -EFBIG;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		if (strncmp(plat_database->spt->partition[x].name, name,
			    SPT_PARTITION_NAME_LENGTH - 1) == 0) {
			RSU_LOG_ERR("error: Partition name already in use");
			return -EINVAL;
		}
	}

	if (plat_database->spt->partitions == SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("error: Partition table is full");
		return -EADDRINUSE;
	}

	for (x = 0; x < plat_database->spt->partitions; x++) {
		RSU_OSAL_U64 pstart = plat_database->spt->partition[x].offset;
		RSU_OSAL_U64 pend = plat_database->spt->partition[x].offset +
				    plat_database->spt->partition[x].length;

		if ((start < pend) && (end > pstart)) {
			RSU_LOG_ERR("error: Partition overlap");
			return -1;
		}
	}

	SAFE_STRCPY(plat_database->spt->partition[plat_database->spt->partitions].name,
		    SPT_PARTITION_NAME_LENGTH, name, SPT_PARTITION_NAME_LENGTH);
	plat_database->spt->partition[plat_database->spt->partitions].offset = start;
	plat_database->spt->partition[plat_database->spt->partitions].length = size;
	plat_database->spt->partition[plat_database->spt->partitions].flags = 0;

	plat_database->spt->partitions++;

	err = writeback_spt();
	if (err) {
		RSU_LOG_ERR("SPT write_back failed");
		return err;
	}

	err = load_spt();
	if (err) {
		RSU_LOG_ERR("SPT load failed");
		return err;
	}

	return 0;
}

static RSU_OSAL_INT priority_get(RSU_OSAL_INT part_num)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT priority = 0;

	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	for (x = plat_database->cpb->header.image_ptr_slots; x > 0; x--) {
		if (plat_database->cpb_slots[x - 1] != ERASED_ENTRY &&
		    plat_database->cpb_slots[x - 1] != SPENT_ENTRY) {
			priority++;
			if (plat_database->cpb_slots[x - 1] ==
			    plat_database->spt->partition[part_num].offset) {
				return priority;
			}
		}
	}

	return 0;
}

static RSU_OSAL_INT priority_add(RSU_OSAL_INT part_num)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_U32 y;
	RSU_OSAL_INT err;

	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		RSU_LOG_ERR("Invalid part number");
		return -EINVAL;
	}

	for (x = 0; x < plat_database->cpb->header.image_ptr_slots; x++) {
		if (plat_database->cpb_slots[x] == ERASED_ENTRY) {
			err = update_cpb(x, plat_database->spt->partition[part_num].offset);
			if (err) {
				RSU_LOG_ERR("error in updating cpb");
				load_cpb();
				return err;
			}
			return load_cpb();
		}
	}

	RSU_LOG_INF("Compressing CPB");

	for (x = 0, y = 0; x < plat_database->cpb->header.image_ptr_slots; x++) {
		if (plat_database->cpb_slots[x] != ERASED_ENTRY &&
		    plat_database->cpb_slots[x] != SPENT_ENTRY) {
			plat_database->cpb_slots[y++] = plat_database->cpb_slots[x];
		}
	}

	if (y < plat_database->cpb->header.image_ptr_slots) {
		plat_database->cpb_slots[y++] = plat_database->spt->partition[part_num].offset;
	} else {
		return -EFAULT;
	}

	while (y < plat_database->cpb->header.image_ptr_slots) {
		plat_database->cpb_slots[y++] = ERASED_ENTRY;
	}

	err = writeback_cpb();
	if (err) {
		RSU_LOG_ERR("CPB write_back failed");
		return err;
	}

	err = load_cpb();
	if (err) {
		RSU_LOG_ERR("CPB load failed");
		return err;
	}

	return 0;
}

static RSU_OSAL_INT priority_remove(RSU_OSAL_INT part_num)
{
	RSU_OSAL_U32 x;
	RSU_OSAL_INT err = 0;

	if (part_num < 0 || (RSU_OSAL_U32)part_num >= plat_database->spt->partitions ||
	    plat_database->spt->partitions > SPT_MAX_PARTITIONS) {
		return -EINVAL;
	}

	for (x = 0; x < plat_database->cpb->header.image_ptr_slots; x++) {
		if (plat_database->cpb_slots[x] == plat_database->spt->partition[part_num].offset) {
			err = update_cpb(x, SPENT_ENTRY);
			if (err) {
				load_cpb();
				return err;
			}
			break;
		}
	}
	err = load_cpb();

	return err;
}

static RSU_OSAL_FILE *rsu_fopen(RSU_OSAL_CHAR *filename, RSU_filesys_flags_t flag)
{
	return plat_database->hal->file.open(filename, flag);
}

static RSU_OSAL_INT rsu_read(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file)
{
	return plat_database->hal->file.read(buf, len, file);
}

static RSU_OSAL_INT rsu_write(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len, RSU_OSAL_FILE *file)
{
	return plat_database->hal->file.write(buf, len, file);
}

static RSU_OSAL_INT rsu_fseek(RSU_OSAL_OFFSET offset, RSU_filesys_whence_t whence, RSU_OSAL_FILE *file)
{
	return plat_database->hal->file.fseek(offset, whence, file);
}

static RSU_OSAL_INT rsu_ftruncate(RSU_OSAL_OFFSET length, RSU_OSAL_FILE *file)
{
	return plat_database->hal->file.ftruncate(length, file);
}

static RSU_OSAL_INT rsu_close(RSU_OSAL_FILE *file)
{
	return plat_database->hal->file.close(file);
}

static RSU_OSAL_INT restore_spt_from_file(RSU_OSAL_CHAR *name)
{
	RSU_OSAL_FILE *fp;
	RSU_OSAL_CHAR *spt_data;
	RSU_OSAL_U32 crc_from_saved_file;
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_U32 magic_number;
	RSU_OSAL_INT ret;

	fp = rsu_fopen(name, RSU_FILE_READ);
	if (!fp) {
		RSU_LOG_ERR("failed to open file for restoring SPT");
		return -EFAULT;
	}

	spt_data = (RSU_OSAL_CHAR *)rsu_malloc(SPT_SIZE);
	if (spt_data == NULL) {
		RSU_LOG_ERR("failed to allocate spt_data");
		rsu_close(fp);
		return -ENOMEM;
	}

	ret = rsu_read(spt_data, SPT_SIZE, fp);
	if (ret < 0) {
		RSU_LOG_ERR("failed to read spt_data");
		rsu_free(spt_data);
		rsu_close(fp);
		return ret;
	}
	RSU_LOG_INF("read size is %d", ret);
	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)spt_data, SPT_SIZE);
	rsu_fseek(SPT_SIZE, RSU_SEEK_SET, fp);
	ret = rsu_read(&crc_from_saved_file, sizeof(crc_from_saved_file), fp);
	if (ret < 0) {
		RSU_LOG_ERR("failed to read spt_data");
		rsu_free(spt_data);
		rsu_close(fp);
		return ret;
	}
	RSU_LOG_INF("read size is %d", ret);

	if (crc_from_saved_file != calc_crc) {
		RSU_LOG_ERR("saved file is corrupted");
		rsu_free(spt_data);
		rsu_close(fp);
		return -EBADF;
	}

	rsu_memcpy(&magic_number, spt_data, sizeof(magic_number));
	if (magic_number != SPT_MAGIC_NUMBER) {
		RSU_LOG_ERR("failure due to mismatch magic number\n");
		rsu_free(spt_data);
		rsu_close(fp);
		return -EFAULT;
	}

	rsu_memcpy(plat_database->spt, spt_data, SPT_SIZE);

	if (load_spt0_offset()) {
		RSU_LOG_ERR("failure to determine SPT0 offset");
		rsu_free(spt_data);
		rsu_close(fp);
		return -EPERM;
	}

	ret = writeback_spt();
	if (ret < 0) {
		RSU_LOG_ERR("failed to write back spt\n");
		return ret;
	}

	plat_database->spt_corrupted = false;

	/* try to reload CPB, as we have a new SPT */
	plat_database->cpb_corrupted = false;
	if (load_cpb() && !plat_database->cpb_corrupted) {
		RSU_LOG_ERR("failed to load CPB after restoring SPT\n");
	}

	rsu_free(spt_data);
	rsu_close(fp);
	return ret;
}

static RSU_OSAL_INT save_spt_to_file(RSU_OSAL_CHAR *name)
{
	RSU_OSAL_FILE *fp;
	RSU_OSAL_CHAR *spt_data;
	RSU_OSAL_INT ret;
	RSU_OSAL_INT write_size;
	RSU_OSAL_U32 calc_crc;

	fp = rsu_fopen(name, RSU_FILE_WRITE);
	if (fp == NULL) {
		RSU_LOG_ERR("failed to open file for saving SPT");
		return -EFAULT;
	}

	spt_data = (RSU_OSAL_CHAR *)rsu_malloc(SPT_SIZE);
	if (spt_data == NULL) {
		RSU_LOG_ERR("failed to allocate spt_data");
		rsu_close(fp);
		return -ENOMEM;
	}

	ret = read_dev(plat_database->spt_addr.spt0_address, spt_data, SPT_SIZE);
	if (ret < 0) {
		RSU_LOG_ERR("failed to read spt_data");
		rsu_free(spt_data);
		rsu_close(fp);
		return ret;
	}

	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)spt_data, SPT_SIZE);
	RSU_LOG_INF("calc_crc is 0x%x", calc_crc);

	write_size = rsu_write(spt_data, SPT_SIZE, fp);
	if (write_size != SPT_SIZE) {
		RSU_LOG_ERR("failed to write %lu SPT data", SPT_SIZE);
		rsu_free(spt_data);
		rsu_close(fp);
		return -EPERM;
	}

	write_size = rsu_write(&calc_crc, sizeof(calc_crc), fp);
	if (write_size != sizeof(calc_crc)) {
		RSU_LOG_ERR("failed to write %lu calc_crc", sizeof(calc_crc));
		rsu_free(spt_data);
		rsu_close(fp);
		return -EPERM;
	}

	rsu_free(spt_data);
	rsu_close(fp);
	return ret;
}

static RSU_OSAL_INT save_spt_to_buf(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size)
{

	RSU_OSAL_INT ret;
	RSU_OSAL_U32 calc_crc;

	if (buffer == NULL) {
		RSU_LOG_ERR("Buffer is NULL");
		return -EFAULT;
	}

	if (size < (SPT_SIZE + sizeof(calc_crc))) {
		RSU_LOG_ERR("size is inadequate");
		return -EFAULT;
	}

	ret = read_dev(plat_database->spt_addr.spt0_address, buffer, SPT_SIZE);
	if (ret < 0) {
		RSU_LOG_ERR("failed to read spt_data");
		return ret;
	}

	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)buffer, SPT_SIZE);
	RSU_LOG_INF("calc_crc is 0x%x", calc_crc);
	rsu_memcpy((buffer + SPT_SIZE), &calc_crc, sizeof(calc_crc));

	return 0;
}

static RSU_OSAL_INT restore_spt_from_buf(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_U32 crc_from_saved_buf;
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_U32 magic_number;
	RSU_OSAL_INT ret;

	if (buffer == NULL) {
		RSU_LOG_ERR("Buffer is NULL");
		return -EFAULT;
	}

	if (size < (SPT_SIZE + sizeof(calc_crc))) {
		RSU_LOG_ERR("size is inadequate");
		return -EFAULT;
	}

	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)buffer, SPT_SIZE);
	rsu_memcpy(&crc_from_saved_buf, (buffer + SPT_SIZE), sizeof(crc_from_saved_buf));

	if (crc_from_saved_buf != calc_crc) {
		RSU_LOG_ERR("saved file is corrupted");
		return -EBADF;
	}

	rsu_memcpy(&magic_number, buffer, sizeof(magic_number));
	if (magic_number != SPT_MAGIC_NUMBER) {
		RSU_LOG_ERR("failure due to mismatch magic number\n");
		return -EFAULT;
	}

	rsu_memcpy(plat_database->spt, buffer, SPT_SIZE);

	if (load_spt0_offset()) {
		RSU_LOG_ERR("failure to determine SPT0 offset");
		return -EPERM;
	}

	ret = writeback_spt();
	if (ret < 0) {
		RSU_LOG_ERR("failed to write back spt\n");
		return ret;
	}

	plat_database->spt_corrupted = false;

	/* try to reload CPB, as we have a new SPT */
	plat_database->cpb_corrupted = false;
	if (load_cpb() && !plat_database->cpb_corrupted) {
		RSU_LOG_ERR("failed to load CPB after restoring SPT\n");
	}

	return ret;
}

static RSU_OSAL_INT corrupted_spt(RSU_OSAL_VOID)
{
	return plat_database->spt_corrupted;
}

static RSU_OSAL_INT empty_cpb(RSU_OSAL_VOID)
{
	RSU_OSAL_INT ret;
	struct cpb_header {
		RSU_OSAL_S32 magic_number;
		RSU_OSAL_S32 header_size;
		RSU_OSAL_S32 cpb_size;
		RSU_OSAL_S32 cpb_reserved;
		RSU_OSAL_S32 image_ptr_offset;
		RSU_OSAL_S32 image_ptr_slots;
	};

	struct cpb_header *c_header;

	if (plat_database->spt_corrupted) {
		RSU_LOG_ERR("corrupted SPT");
		return -ECORRUPTED_CPB;
	}

	c_header = (struct cpb_header *)rsu_malloc(sizeof(struct cpb_header));
	if (c_header == NULL) {
		RSU_LOG_ERR("failed to allocate cpb_header");
		return -ENOMEM;
	}
	rsu_memset(c_header, 0, sizeof(struct cpb_header));

	c_header->magic_number = CPB_MAGIC_NUMBER;
	c_header->header_size = CPB_HEADER_SIZE;
	c_header->cpb_size = CPB_SIZE;
	c_header->cpb_reserved = 0;
	c_header->image_ptr_offset = CPB_IMAGE_PTR_OFFSET;
	c_header->image_ptr_slots = CPB_IMAGE_PTR_NSLOTS;

	rsu_memset(plat_database->cpb, -1, CPB_SIZE);
	rsu_memcpy(plat_database->cpb, c_header, (RSU_OSAL_U32)sizeof(struct cpb_header));

	ret = writeback_cpb();
	if (ret) {
		RSU_LOG_ERR("failed to write back cpb\n");
		rsu_free(c_header);
		return ret;
	}

	plat_database->cpb_slots = (CMF_POINTER *)&plat_database->cpb
					   ->data[plat_database->cpb->header.image_ptr_offset];
	plat_database->cpb_corrupted = false;
	plat_database->cpb_fixed = true;

	rsu_free(c_header);
	return ret;
}

static RSU_OSAL_INT restore_cpb_from_file(RSU_OSAL_CHAR *name)
{
	RSU_OSAL_FILE *fp;
	RSU_OSAL_CHAR *cpb_data;
	RSU_OSAL_U32 crc_from_saved_file;
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_U32 magic_number;
	RSU_OSAL_INT ret;

	if (plat_database->spt_corrupted) {
		RSU_LOG_ERR("corrupted SPT");
		return -ECORRUPTED_SPT;
	}

	fp = rsu_fopen(name, RSU_FILE_READ);
	if (!fp) {
		RSU_LOG_ERR("failed to open file for restoring CPB");
		return -EFAULT;
	}

	cpb_data = (RSU_OSAL_CHAR *)rsu_malloc(CPB_SIZE);
	if (!cpb_data) {
		RSU_LOG_ERR("failed to allocate cpb_data");
		rsu_close(fp);
		return -ENOMEM;
	}

	ret = rsu_read(cpb_data, CPB_SIZE, fp);
	if (!ret) {
		RSU_LOG_ERR("failed to read");
		rsu_free(cpb_data);
		rsu_close(fp);
		return -EPERM;
	}

	RSU_LOG_INF("read size is %d", ret);
	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)cpb_data, CPB_SIZE);
	rsu_fseek(CPB_SIZE, RSU_SEEK_SET, fp);
	ret = rsu_read(&crc_from_saved_file, sizeof(crc_from_saved_file), fp);
	if (!ret) {
		RSU_LOG_ERR("failed to read");
		rsu_free(cpb_data);
		rsu_close(fp);
		return -EPERM;
	}
	RSU_LOG_INF("read size is %d", ret);

	if (crc_from_saved_file != calc_crc) {
		RSU_LOG_ERR("saved file is corrupted");
		rsu_free(cpb_data);
		rsu_close(fp);
		return -EBADF;
	}

	rsu_memcpy(&magic_number, cpb_data, sizeof(magic_number));
	if (magic_number != CPB_MAGIC_NUMBER) {
		RSU_LOG_ERR("failure due to mismatch magic number");
		rsu_free(cpb_data);
		rsu_close(fp);
		return -EFAULT;
	}

	rsu_memcpy(plat_database->cpb, cpb_data, CPB_SIZE);
	ret = writeback_cpb();
	if (ret) {
		RSU_LOG_ERR("failed to write back cpb\n");
		rsu_free(cpb_data);
		rsu_close(fp);
		return ret;
	}

	plat_database->cpb_slots = (CMF_POINTER *)&plat_database->cpb
					   ->data[plat_database->cpb->header.image_ptr_offset];
	plat_database->cpb_corrupted = false;
	plat_database->cpb_fixed = true;

	rsu_free(cpb_data);
	rsu_close(fp);
	return ret;
}

static RSU_OSAL_INT save_cpb_to_file(RSU_OSAL_CHAR *name)
{
	RSU_OSAL_FILE *fp;
	RSU_OSAL_CHAR *cpb_data;
	RSU_OSAL_INT ret;
	RSU_OSAL_INT write_size;
	RSU_OSAL_U32 calc_crc;

	fp = rsu_fopen(name, RSU_FILE_WRITE);
	if (!fp) {
		RSU_LOG_ERR("failed to open file for saving CPB");
		return -EFAULT;
	}

	cpb_data = (RSU_OSAL_CHAR *)rsu_malloc(CPB_SIZE);
	if (!cpb_data) {
		RSU_LOG_ERR("failed to allocate cpb_data");
		rsu_close(fp);
		return -ENOMEM;
	}

	ret = read_part(plat_database->cpb0_part, 0, cpb_data, CPB_SIZE);
	if (ret) {
		RSU_LOG_ERR("failed to read CPB data");
		rsu_free(cpb_data);
		rsu_close(fp);
		return ret;
	}

	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)cpb_data, CPB_SIZE);
	RSU_LOG_INF("calc_crc is 0x%x", calc_crc);

	write_size = rsu_write(cpb_data, CPB_SIZE, fp);
	if (write_size != CPB_SIZE) {
		RSU_LOG_ERR("failed to write %d CPB data", CPB_SIZE);
		rsu_free(cpb_data);
		rsu_close(fp);
		return -EPERM;
	}
	write_size = rsu_write(&calc_crc, sizeof(calc_crc), fp);
	if (write_size != sizeof(calc_crc)) {
		RSU_LOG_ERR("failed to write %lu calc_crc", sizeof(calc_crc));
		rsu_free(cpb_data);
		rsu_close(fp);
		return -EPERM;
	}

	rsu_free(cpb_data);
	rsu_close(fp);
	return ret;
}

static RSU_OSAL_INT save_cpb_to_buf(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_INT ret;
	RSU_OSAL_U32 calc_crc;

	if (buffer == NULL) {
		RSU_LOG_ERR("Buffer is NULL");
		return -EFAULT;
	}

	if (size < (CPB_SIZE + sizeof(calc_crc))) {
		RSU_LOG_ERR("size is inadequate");
		return -EFAULT;
	}

	ret = read_part(plat_database->cpb0_part, 0, buffer, CPB_SIZE);
	if (ret) {
		RSU_LOG_ERR("failed to read CPB data");
		return ret;
	}

	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)buffer, CPB_SIZE);
	RSU_LOG_INF("calc_crc is 0x%x", calc_crc);

	rsu_memcpy((buffer + CPB_SIZE), &calc_crc, sizeof(calc_crc));

	return 0;
}

static RSU_OSAL_INT restore_cpb_from_buf(RSU_OSAL_U8 *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_U32 crc_from_saved_buf;
	RSU_OSAL_U32 calc_crc;
	RSU_OSAL_U32 magic_number;
	RSU_OSAL_INT ret;

	if (plat_database->spt_corrupted) {
		return -ECORRUPTED_SPT;
	}

	if (buffer == NULL) {
		RSU_LOG_ERR("Buffer is NULL");
		return -EFAULT;
	}

	if (size < (CPB_SIZE + sizeof(calc_crc))) {
		RSU_LOG_ERR("size is inadequate");
		return -EFAULT;
	}

	calc_crc = rsu_crc32(0, (RSU_OSAL_VOID *)buffer, CPB_SIZE);
	rsu_memcpy(&crc_from_saved_buf, (buffer + CPB_SIZE), sizeof(crc_from_saved_buf));

	if (crc_from_saved_buf != calc_crc) {
		RSU_LOG_ERR("saved file is corrupted");
		return -EBADF;
	}

	rsu_memcpy(&magic_number, buffer, sizeof(magic_number));
	if (magic_number != CPB_MAGIC_NUMBER) {
		RSU_LOG_ERR("failure due to mismatch magic number");
		return -EFAULT;
	}

	rsu_memcpy(plat_database->cpb, buffer, CPB_SIZE);
	ret = writeback_cpb();
	if (ret) {
		RSU_LOG_ERR("failed to write back cpb\n");
		return ret;
	}

	plat_database->cpb_slots = (CMF_POINTER *)&plat_database->cpb
					   ->data[plat_database->cpb->header.image_ptr_offset];
	plat_database->cpb_corrupted = false;
	plat_database->cpb_fixed = true;

	return 0;
}

static RSU_OSAL_INT corrupted_cpb(RSU_OSAL_VOID)
{
	return plat_database->cpb_corrupted;
}

static struct librsu_hl_intf hl_intf = {
	.close = rsu_qspi_close,

	.file.open = rsu_fopen,
	.file.read = rsu_read,
	.file.write = rsu_write,
	.file.fseek = rsu_fseek,
	.file.ftruncate = rsu_ftruncate,
	.file.close = rsu_close,

	.partition.count = partition_count,
	.partition.name = partition_name,
	.partition.offset = partition_offset,
	.partition.factory_offset = factory_offset,
	.partition.size = partition_size,
	.partition.reserved = partition_reserved,
	.partition.readonly = partition_readonly,
	.partition.rename = partition_rename,
	.partition.delete = partition_delete,
	.partition.create = partition_create,

	.priority.get = priority_get,
	.priority.add = priority_add,
	.priority.remove = priority_remove,

	.data.read = data_read,
	.data.write = data_write,
	.data.erase = data_erase,

	.spt_ops.restore_file = restore_spt_from_file,
	.spt_ops.save_file = save_spt_to_file,
	.spt_ops.save_buf = save_spt_to_buf,
	.spt_ops.restore_buf = restore_spt_from_buf,
	.spt_ops.corrupted = corrupted_spt,

	.cpb_ops.empty = empty_cpb,
	.cpb_ops.restore_file = restore_cpb_from_file,
	.cpb_ops.save_file = save_cpb_to_file,
	.cpb_ops.restore_buf = restore_cpb_from_buf,
	.cpb_ops.save_buf = save_cpb_to_buf,
	.cpb_ops.corrupted = corrupted_cpb,

	.misc_ops.notify_sdm = notify_sdm,
	.misc_ops.rsu_status = rsu_status,
	.misc_ops.rsu_set_address = rsu_set_address,
	.misc_ops.rsu_get_dcmf_status = rsu_get_dcmf_status,
	.misc_ops.rsu_get_dcmf_version = rsu_get_dcmf_version,
	.misc_ops.rsu_get_max_retry_count = rsu_get_max_retry_count,
};

RSU_OSAL_INT rsu_qspi_open(struct librsu_ll_intf *intf, struct librsu_hl_intf **hl_ptr)
{
	if (intf == NULL || hl_ptr == NULL) {
		RSU_LOG_ERR("Invalid arguments");
		return -EINVAL;
	}

	if (plat_database != NULL) {
		RSU_LOG_ERR("plat_database not NULL");
		return -EADDRINUSE;
	}

	plat_database = rsu_malloc(sizeof(struct database));
	if (plat_database == NULL) {
		RSU_LOG_ERR("Error in allocating memory");
		return -ENOMEM;
	}

	rsu_memset(plat_database, (RSU_OSAL_U32)0, sizeof(struct database));

	plat_database->hal = intf; /*Attach the ll intf to the plat_database*/

	plat_database->spt = rsu_malloc(sizeof(struct SUB_PARTITION_TABLE));
	if (plat_database->spt == NULL) {
		RSU_LOG_ERR("Error in allocating  spt memory");
		rsu_free(plat_database);
		return -ENOMEM;
	}

	plat_database->cpb = rsu_malloc(CPB_BLOCK_SIZE);
	if (plat_database->cpb == NULL) {
		RSU_LOG_ERR("Error in allocating cpb memory");
		rsu_free(plat_database->spt);
		rsu_free(plat_database);
		return -ENOMEM;
	}

	RSU_LOG_INF("opening qspi flash and access spt and cpb tables");

	RSU_OSAL_INT ret;

	ret = intf->mbox.get_spt_addresses(&(plat_database->spt_addr));
	if (ret != 0) {
		RSU_LOG_ERR("error in retrieving spt address");
		rsu_free(plat_database->spt);
		rsu_free(plat_database->cpb);
		rsu_free(plat_database);
		return -ENODEV;
	}

	RSU_LOG_INF("SPT0 address is 0x%llx", plat_database->spt_addr.spt0_address);
	RSU_LOG_INF("SPT1 address is 0x%llx", plat_database->spt_addr.spt1_address);

	if (load_spt() && !plat_database->spt_corrupted) {
		RSU_LOG_ERR("error: Bad SPT");
		rsu_qspi_close();
		return -EFAULT;
	}

	if (plat_database->spt_corrupted) {
		plat_database->cpb_corrupted = true;
	} else if (load_cpb() && !plat_database->cpb_corrupted) {
		RSU_LOG_ERR("error: Bad CPB");
		rsu_qspi_close();
		return -EFAULT;
	}

	RSU_LOG_INF("finished reading qspi flash");

	*hl_ptr = &hl_intf;

	return 0;
}
