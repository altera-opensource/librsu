/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU.h>
#include <libRSU_OSAL.h>
#include <libRSU_context.h>
#include <libRSU_hl_intf.h>
#include <libRSU_ll_intf.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <libRSU_cfg.h>
#include <libRSU_misc.h>
#include <libRSU_cb.h>

#include <version.h>
#include <string.h>

#define RSU_NOTIFY_RESET_RETRY_COUNTER (1 << 16)
#define RSU_NOTIFY_CLEAR_ERROR_STATUS  (1 << 17)
#define RSU_NOTIFY_IGNORE_STAGE	       (1 << 18)
#define RSU_NOTIFY_VALUE_MASK	       (0xFFFF)

#define RSU_BUFFER_CHUNK_SIZE (0x1000)

#ifndef DEFAULT_CFG_FILENAME
#define DEFAULT_CFG_FILENAME "/etc/librsu.rc"
#endif

static struct rsu_context ctx;
static struct librsu_hl_intf *intf = NULL;

#define MUTEX_LOCK()   rsu_mutex_timedlock(&(ctx.mutex), RSU_TIME_FOREVER)
#define MUTEX_UNLOCK() rsu_mutex_unlock(&(ctx.mutex))

RSU_OSAL_U32 rsu_get_version(RSU_OSAL_VOID)
{
	return ((rsu_poc_verion_major & 0xFFFF) << 16) | ((rsu_poc_verion_minor & 0xFFFF));
}

RSU_OSAL_INT librsu_init(RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_CHAR *cfg_filename = NULL;
	RSU_OSAL_INT ret = 0;

	if (ctx.state != un_initialized) {
		RSU_LOG_ERR("RSU library already initialized or ongoing initialization");
		return -ELIB;
	}

	ctx.state = in_progress;

	if (!filename || filename[0] == '\0') {
		cfg_filename = DEFAULT_CFG_FILENAME;
	} else {
		cfg_filename = filename;
	}

	ret = rsu_mutex_init(&(ctx.mutex));
	if (ret < 0) {
		ctx.state = un_initialized;
		RSU_LOG_ERR("Error in initializing mutex %d", ret);
		return -ECFG;
	}

	ret = librsu_cfg_parse(cfg_filename, &intf);
	if (ret) {
		ctx.state = un_initialized;
		RSU_LOG_ERR("error in configuring libRSU %d", ret);
		return -ECFG;
	}

	ctx.state = initialized;
	RSU_LOG_DBG("libRSU initialization completed \n");

	return 0;
}

RSU_OSAL_VOID librsu_exit(RSU_OSAL_VOID)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return;
	}

	ctx.state = in_progress;
	RSU_LOG_DBG("libRSU exit started");
	rsu_mutex_destroy(&(ctx.mutex));

	intf->close();
	intf = NULL;
	librsu_cfg_reset();

	ctx.state = un_initialized;
	RSU_LOG_DBG("libRSU exit completed");
}

RSU_OSAL_INT rsu_notify(RSU_OSAL_INT value)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	RSU_OSAL_U32 notify_value;

	MUTEX_LOCK();

	notify_value = value & RSU_NOTIFY_VALUE_MASK;

	if (intf->misc_ops.notify_sdm(notify_value)) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_status_log(struct rsu_status_info *info)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (!info) {
		return -EARGS;
	}

	struct mbox_status_info data;
	MUTEX_LOCK();

	if (intf->misc_ops.rsu_status(&data)) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	info->version = data.version;
	info->state = data.state;
	info->current_image = data.current_image;
	info->fail_image = data.fail_image;
	info->error_location = data.error_location;
	info->error_details = data.error_details;
	info->retry_counter = 0;

	if (!RSU_VERSION_ACMF_VERSION(info->version) || !RSU_VERSION_DCMF_VERSION(info->version)) {
		MUTEX_UNLOCK();
		return 0;
	}
	info->retry_counter = data.retry_counter;

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_clear_error_status(RSU_OSAL_VOID)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	struct rsu_status_info info;
	RSU_OSAL_U32 notify_value;

	if (rsu_status_log(&info)) {
		return -EFILEIO;
	}

	if (!RSU_VERSION_ACMF_VERSION(info.version)) {
		return -EFILEIO;
	}

	notify_value = RSU_NOTIFY_IGNORE_STAGE | RSU_NOTIFY_CLEAR_ERROR_STATUS;
	MUTEX_LOCK();

	if (intf->misc_ops.notify_sdm(notify_value)) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_reset_retry_counter(RSU_OSAL_VOID)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	struct rsu_status_info info;
	RSU_OSAL_U32 notify_value;

	if (rsu_status_log(&info)) {
		return -EFILEIO;
	}

	if (!RSU_VERSION_ACMF_VERSION(info.version) || !RSU_VERSION_DCMF_VERSION(info.version)) {
		return -EFILEIO;
	}

	notify_value = RSU_NOTIFY_IGNORE_STAGE | RSU_NOTIFY_RESET_RETRY_COUNTER;
	MUTEX_LOCK();
	if (intf->misc_ops.notify_sdm(notify_value)) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_slot_count(RSU_OSAL_VOID)
{
	RSU_OSAL_INT partitions;
	RSU_OSAL_INT cnt = 0;
	RSU_OSAL_INT x;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	partitions = intf->partition.count();

	for (x = 0; x < partitions; x++) {
		if (librsu_misc_is_slot(intf, x)) {
			cnt++;
		}
	}

	MUTEX_UNLOCK();

	return cnt;
}

RSU_OSAL_INT rsu_slot_by_name(RSU_OSAL_CHAR *name)
{
	RSU_OSAL_INT partitions;
	RSU_OSAL_INT x, cnt = 0;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (name == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	partitions = intf->partition.count();

	for (x = 0; x < partitions; x++) {
		if (librsu_misc_is_slot(intf, x)) {
			if (!strcmp(name, intf->partition.name(x))) {
				MUTEX_UNLOCK();
				return cnt;
			}
			cnt++;
		}
	}

	MUTEX_UNLOCK();

	return -ENAME;
}

RSU_OSAL_INT rsu_slot_get_info(RSU_OSAL_INT slot, struct rsu_slot_info *info)
{
	RSU_OSAL_INT part_num, ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (info == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	SAFE_STRCPY(info->name, sizeof(info->name), intf->partition.name(part_num),
		    sizeof(info->name));

	ret = intf->partition.offset(part_num, &info->offset);
	if (ret) {
		RSU_LOG_ERR("Error in getting the partition offset : %d", ret);
		MUTEX_UNLOCK();
		return -EBADF;
	}
	info->size = intf->partition.size(part_num);
	info->priority = intf->priority.get(part_num);

	MUTEX_UNLOCK();

	return 0;
}

RSU_OSAL_INT rsu_slot_size(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num;
	RSU_OSAL_INT size;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	size = intf->partition.size(part_num);

	MUTEX_UNLOCK();
	return size;
}

RSU_OSAL_INT rsu_slot_priority(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num, priority;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	priority = intf->priority.get(part_num);

	MUTEX_UNLOCK();
	return priority;
}

RSU_OSAL_INT rsu_slot_erase(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (librsu_cfg_writeprotected(slot)) {
		RSU_LOG_ERR("Trying to erase a write protected slot");
		MUTEX_UNLOCK();
		return -EWRPROT;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->priority.remove(part_num)) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	if (intf->data.erase(part_num)) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_slot_program_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (librsu_cb_buf_init(buf, size)) {
		RSU_LOG_ERR("Bad buf/size arguments");
		MUTEX_UNLOCK();
		return -EARGS;
	}

	rtn = librsu_cb_program_common(intf, slot, librsu_cb_buf, 0);

	librsu_cb_buf_cleanup();

	MUTEX_UNLOCK();
	return rtn;
}

/*
 * This API was added to force users to use the updated image handling
 * algorithm, introduced at the same time, which deals properly with both
 * regular and factory update images.
 */
RSU_OSAL_INT rsu_slot_program_factory_update_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf,
						 RSU_OSAL_INT size)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	rtn = rsu_slot_program_buf(slot, buf, size);

	return rtn;
}

RSU_OSAL_INT rsu_slot_program_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (librsu_cb_file_init(filename)) {
		RSU_LOG_ERR("Unable to open file '%s'", filename);
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	rtn = librsu_cb_program_common(intf, slot, librsu_cb_file, 0);

	librsu_cb_file_cleanup();

	MUTEX_UNLOCK();

	return rtn;
}

/*
 * This API was added to force users to use the updated image handling
 * algorithm, introduced at the same time, which deals properly with both
 * regular and factory update images.
 */
RSU_OSAL_INT rsu_slot_program_factory_update_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	rtn = rsu_slot_program_file(slot, filename);

	return rtn;
}

RSU_OSAL_INT rsu_slot_program_buf_raw(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (librsu_cb_buf_init(buf, size)) {
		RSU_LOG_ERR("Bad buf/size arguments");
		MUTEX_UNLOCK();
		return -EARGS;
	}

	rtn = librsu_cb_program_common(intf, slot, librsu_cb_buf, 1);

	librsu_cb_buf_cleanup();

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_program_file_raw(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (librsu_cb_file_init(filename)) {
		RSU_LOG_ERR("Unable to open file '%s'", filename);
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	rtn = librsu_cb_program_common(intf, slot, librsu_cb_file, 1);

	librsu_cb_file_cleanup();

	MUTEX_UNLOCK();
	return rtn;
}

RSU_OSAL_INT rsu_slot_verify_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (librsu_cb_buf_init(buf, size)) {
		RSU_LOG_ERR("Bad buf/size arguments");
		MUTEX_UNLOCK();
		return -EARGS;
	}

	rtn = librsu_cb_verify_common(intf, slot, librsu_cb_buf, 0);

	librsu_cb_buf_cleanup();

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_verify_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (librsu_cb_file_init(filename)) {
		RSU_LOG_ERR("Unable to open file '%s'", filename);
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	rtn = librsu_cb_verify_common(intf, slot, librsu_cb_file, 0);

	librsu_cb_file_cleanup();

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_verify_buf_raw(RSU_OSAL_INT slot, RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (librsu_cb_buf_init(buf, size)) {
		RSU_LOG_ERR("Bad buf/size arguments");
		MUTEX_UNLOCK();
		return -EARGS;
	}

	rtn = librsu_cb_verify_common(intf, slot, librsu_cb_buf, 1);

	librsu_cb_buf_cleanup();

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_verify_file_raw(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (librsu_cb_file_init(filename)) {
		RSU_LOG_ERR("Unable to open file '%s'", filename);
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	rtn = librsu_cb_verify_common(intf, slot, librsu_cb_file, 1);

	librsu_cb_file_cleanup();

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_program_callback(RSU_OSAL_INT slot, rsu_data_callback callback)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	rtn = librsu_cb_program_common(intf, slot, callback, 0);

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_program_callback_raw(RSU_OSAL_INT slot, rsu_data_callback callback)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	rtn = librsu_cb_program_common(intf, slot, callback, 1);

	MUTEX_UNLOCK();
	return rtn;
}

RSU_OSAL_INT rsu_slot_verify_callback(RSU_OSAL_INT slot, rsu_data_callback callback)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	rtn = librsu_cb_verify_common(intf, slot, callback, 0);

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_verify_callback_raw(RSU_OSAL_INT slot, rsu_data_callback callback)
{
	RSU_OSAL_INT rtn;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	rtn = librsu_cb_verify_common(intf, slot, callback, 1);

	MUTEX_UNLOCK();

	return rtn;
}

RSU_OSAL_INT rsu_slot_copy_to_file(RSU_OSAL_INT slot, RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT part_num;
	RSU_OSAL_FILE *df;
	RSU_OSAL_INT offset;
	RSU_OSAL_CHAR *buf = NULL;
	RSU_OSAL_CHAR *fill = NULL;
	RSU_OSAL_INT last_write;
	RSU_OSAL_INT x;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	buf = rsu_malloc(RSU_BUFFER_CHUNK_SIZE);
	if (buf == NULL) {
		MUTEX_UNLOCK();
		return -ENOMEM;
	}

	fill = rsu_malloc(RSU_BUFFER_CHUNK_SIZE);
	if (fill == NULL) {
		rsu_free(buf);
		MUTEX_UNLOCK();
		return -ENOMEM;
	}

	if (filename == NULL) {
		RSU_LOG_ERR("filename is NULL");
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -EARGS;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		RSU_LOG_ERR("slot is not usable");
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (intf->priority.get(part_num) <= 0) {
		RSU_LOG_ERR("Trying to read an erased slot");
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -EERASE;
	}

	df = intf->file.open(filename, RSU_FILE_WRITE);
	if (df == NULL) {
		RSU_LOG_ERR("Unable to open output file '%s'", filename);
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	if (intf->file.ftruncate(0, df) < 0) {
		RSU_LOG_ERR("Unable to truncate file '%s' to length zero", filename);
		intf->file.close(df);
		rsu_free(buf);
		rsu_free(fill);
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	offset = 0;
	last_write = 0;

	rsu_memset(fill, 0xff, RSU_BUFFER_CHUNK_SIZE);

	/* Read buf sized chunks from slot and write to file */
	while (offset < intf->partition.size(part_num)) {
		/* Read a buffer size chunk from slot */
		if (intf->data.read(part_num, offset, RSU_BUFFER_CHUNK_SIZE, buf)) {
			RSU_LOG_ERR("Unable to rd slot %i, offs 0x%08x, cnt %i", slot,
				    (RSU_OSAL_U32)offset, RSU_BUFFER_CHUNK_SIZE);
			intf->file.close(df);
			rsu_free(buf);
			rsu_free(fill);
			MUTEX_UNLOCK();
			return -ELOWLEVEL;
		}
		/* Scan buffer to see if we have all 0xff's. Don't write to
		 * file if we do.  If we skipped some chunks because they
		 * were all 0xff's and then find one that is not, we fill
		 * the file with 0xff chunks up to the current position.
		 */
		for (x = 0; x < RSU_BUFFER_CHUNK_SIZE; x++) {
			if (buf[x] != (RSU_OSAL_CHAR)0xFF) {
				break;
			}
		}

		if (x < RSU_BUFFER_CHUNK_SIZE) {
			while (last_write < offset) {
				if (intf->file.write(fill, RSU_BUFFER_CHUNK_SIZE, df) !=
				    RSU_BUFFER_CHUNK_SIZE) {
					RSU_LOG_ERR("Unable to wr to '%s'", filename);
					intf->file.close(df);
					rsu_free(buf);
					rsu_free(fill);
					MUTEX_UNLOCK();
					return -EFILEIO;
				}
				last_write += RSU_BUFFER_CHUNK_SIZE;
			}

			if (intf->file.write(buf, RSU_BUFFER_CHUNK_SIZE, df) !=
			    RSU_BUFFER_CHUNK_SIZE) {
				RSU_LOG_ERR("Unable to wr to file '%s'", filename);
				intf->file.close(df);
				rsu_free(buf);
				rsu_free(fill);
				MUTEX_UNLOCK();
				return -EFILEIO;
			}

			last_write += RSU_BUFFER_CHUNK_SIZE;
		}

		offset += RSU_BUFFER_CHUNK_SIZE;
	}

	intf->file.close(df);
	rsu_free(buf);
	rsu_free(fill);
	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_slot_disable(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->priority.remove(part_num)) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_slot_enable(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->priority.remove(part_num)) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	if (intf->priority.add(part_num)) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	MUTEX_UNLOCK();

	return 0;
}

RSU_OSAL_INT rsu_slot_load_after_reboot(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num, ret;
	RSU_OSAL_U64 offset;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	ret = intf->partition.offset(part_num, &offset);
	if (ret < 0) {
		RSU_LOG_ERR("Errro in getting the partition offset : %d", ret);
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->misc_ops.rsu_set_address(offset) != 0) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	MUTEX_UNLOCK();

	return 0;
}

RSU_OSAL_INT rsu_slot_load_factory_after_reboot(RSU_OSAL_VOID)
{
	RSU_OSAL_INT part_num, ret;
	RSU_OSAL_INT partitions;
	RSU_OSAL_U64 offset;
	const RSU_OSAL_CHAR name[] = "FACTORY_IMAGE";

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	partitions = intf->partition.count();

	for (part_num = 0; part_num < partitions; part_num++) {
		if (!strcmp(name, intf->partition.name(part_num))) {
			break;
		}
	}

	if (part_num >= partitions) {
		RSU_LOG_ERR("No FACTORY_IMAGE partition defined");
		MUTEX_UNLOCK();
		return -EFORMAT;
	}

	ret = intf->partition.offset(part_num, &offset);
	if (ret < 0) {
		RSU_LOG_ERR("Errro in getting the partition offset : %d", ret);
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->misc_ops.rsu_set_address(offset) != 0) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_slot_rename(RSU_OSAL_INT slot, RSU_OSAL_CHAR *name)
{
	RSU_OSAL_INT part_num;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (name == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (librsu_misc_is_rsvd_name(name)) {
		RSU_LOG_ERR("error: Partition rename uses a reserved name");
		MUTEX_UNLOCK();
		return -ENAME;
	}

	if (intf->partition.rename(part_num, name)) {
		MUTEX_UNLOCK();
		return -ENAME;
	}

	MUTEX_UNLOCK();
	return 0;
}

/*
 * rsu_slot_delete() - Delete the selected slot.
 * slot: slot number
 *
 * Returns 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_delete(RSU_OSAL_INT slot)
{
	RSU_OSAL_INT part_num;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (librsu_cfg_writeprotected(slot)) {
		RSU_LOG_ERR("Trying to delete a write protected slot");
		MUTEX_UNLOCK();
		return -EWRPROT;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	if (intf->priority.remove(part_num)) {
		MUTEX_UNLOCK();
		RSU_LOG_ERR("Failed to remove priority");
		return -ELOWLEVEL;
	}

	if (intf->data.erase(part_num)) {
		MUTEX_UNLOCK();
		RSU_LOG_ERR("Failed to erase partition");
		return -ELOWLEVEL;
	}

	if (intf->partition.delete(part_num)) {
		MUTEX_UNLOCK();
		RSU_LOG_ERR("Failed to delete partition");
		return -ELOWLEVEL;
	}

	MUTEX_UNLOCK();

	return 0;
}

/*
 * rsu_slot_create() - Create a new slot.
 * name: slot name
 * address: slot start address
 * size: slot size
 *
 * Returns 0 on success, or Error Code
 */
RSU_OSAL_INT rsu_slot_create(RSU_OSAL_CHAR *name, RSU_OSAL_U64 address, RSU_OSAL_U32 size)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (name == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (librsu_misc_is_rsvd_name(name)) {
		RSU_LOG_ERR("error: Partition create uses a reserved name");
		MUTEX_UNLOCK();
		return -ENAME;
	}

	if (intf->partition.create(name, address, size)) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	MUTEX_UNLOCK();

	return 0;
}

RSU_OSAL_INT rsu_restore_spt(RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (filename == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	ret = intf->spt_ops.restore_file(filename);

	MUTEX_UNLOCK();

	return ret;
}

RSU_OSAL_INT rsu_save_spt(RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (filename == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	ret = intf->spt_ops.save_file(filename);

	MUTEX_UNLOCK();

	return ret;
}

/**
 * rsu_create_empty_cpb() - create a empty cpb
 *
 * This function is used to create a empty cpb with the header field only
 *
 * Returns: 0 on success, or error code
 */
RSU_OSAL_INT rsu_create_empty_cpb(RSU_OSAL_VOID)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	ret = intf->cpb_ops.empty();

	MUTEX_UNLOCK();

	return ret;
}


RSU_OSAL_INT rsu_restore_cpb(RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (filename == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	ret = intf->cpb_ops.restore_file(filename);

	MUTEX_UNLOCK();

	return ret;
}

RSU_OSAL_INT rsu_save_cpb(RSU_OSAL_CHAR *filename)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (filename == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	ret = intf->cpb_ops.save_file(filename);

	MUTEX_UNLOCK();

	return ret;
}

RSU_OSAL_INT rsu_running_factory(RSU_OSAL_INT *factory)
{
	RSU_OSAL_U64 factory_offset;
	RSU_OSAL_INT ret;
	struct mbox_status_info data;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (factory == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	ret = intf->partition.factory_offset(&factory_offset);
	if (ret < 0) {
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	if (intf->misc_ops.rsu_status(&data)) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	RSU_LOG_INF("factory offset is 0x%08llx and current image is 0x%08llx\n", factory_offset,
		    data.current_image);
	*factory = ((factory_offset == data.current_image) ? 1 : 0);

	MUTEX_UNLOCK();

	return 0;
}

/*
 * rsu_dcmf_version() - retrieve the decision firmware version
 * @versions: pointer to where the four DCMF versions will be stored
 *
 * This function is used to retrieve the version of each of the four DCMF copies
 * in flash.
 *
 * Returns: 0 on success, or error code
 */
RSU_OSAL_INT rsu_dcmf_version(RSU_OSAL_U32 *versions)
{
	RSU_OSAL_INT ret;
	struct rsu_dcmf_version ver;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (versions == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	ret = intf->misc_ops.rsu_get_dcmf_version(&ver);
	if (ret < 0) {
		RSU_LOG_ERR("Error while getting dcmf version");
		MUTEX_UNLOCK();
		return ret;
	}

	for (RSU_OSAL_INT i = 0; i < 4; i++) {
		versions[i] = ver.dcmf[i];
	}

	MUTEX_UNLOCK();

	return ret;
}

/*
 * rsu_max_retry() - retrieve the max_retry parameter
 * @value: pointer to where the max_retry will be stored
 *
 * This function is used to retrieve the max_retry parameter from flash.
 *
 * Returns: 0 on success, or error code
 */
RSU_OSAL_INT rsu_max_retry(RSU_OSAL_U8 *value)
{
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (value == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->misc_ops.rsu_get_max_retry_count(value)) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	MUTEX_UNLOCK();

	return 0;
}

/*
 * rsu_dcmf_status() - retrieve the decision firmware status
 * @status: pointer to where the status values will be stored
 *
 * This function is used to determine whether decision firmware copies are
 * corrupted in flash, with the currently used decision firmware being used as
 * reference. The status is an array of 4 values, one for each decision
 * firmware copy. A 0 means the copy is fine, anything else means the copy is
 * corrupted.
 *
 * Returns: 0 on success, or error code
 */
RSU_OSAL_INT rsu_dcmf_status(RSU_OSAL_INT *status)
{
	struct rsu_dcmf_status dcmf_status;
	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (status == NULL) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->misc_ops.rsu_get_dcmf_status(&dcmf_status) < 0) {
		MUTEX_UNLOCK();
		return -EFILEIO;
	}

	for (RSU_OSAL_INT i = 0; i < 4; i++) {
		status[i] = dcmf_status.dcmf[i];
	}

	MUTEX_UNLOCK();

	return 0;
}

RSU_OSAL_INT rsu_slot_copy_to_buf(RSU_OSAL_INT slot, RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_INT part_num;
	RSU_OSAL_INT ret;
	RSU_OSAL_INT part_size;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	MUTEX_LOCK();

	if (buffer == NULL) {
		RSU_LOG_ERR("buffer is NULL");
		MUTEX_UNLOCK();
		return -EARGS;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		RSU_LOG_ERR("slot is not usable");
		MUTEX_UNLOCK();
		return -ESLOTNUM;
	}

	part_size = intf->partition.size(part_num);
	if (part_size < 0) {
		RSU_LOG_ERR("error while reading the part size");
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	if ((RSU_OSAL_SIZE)part_size > size || size == 0) {
		RSU_LOG_ERR("buffer size is not adequate");
		MUTEX_UNLOCK();
		return -EARGS;
	}

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	if (intf->priority.get(part_num) <= 0) {
		RSU_LOG_ERR("Trying to read an erased slot");
		MUTEX_UNLOCK();
		return -EERASE;
	}

	ret = intf->data.read(part_num, 0, part_size, buffer);
	if (ret < 0) {
		RSU_LOG_ERR("Error in reading data from QSPI");
		MUTEX_UNLOCK();
		return -ELOWLEVEL;
	}

	MUTEX_UNLOCK();
	return 0;
}

RSU_OSAL_INT rsu_save_spt_to_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (buffer == NULL || size == 0) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->spt_ops.corrupted()) {
		RSU_LOG_ERR("corrupted SPT");
		MUTEX_UNLOCK();
		return -ECORRUPTED_SPT;
	}

	ret = intf->spt_ops.save_buf(buffer, size);

	MUTEX_UNLOCK();

	return ret;
}

RSU_OSAL_INT rsu_restore_spt_from_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (buffer == NULL || size == 0) {
		return -EARGS;
	}

	MUTEX_LOCK();

	ret = intf->spt_ops.restore_buf(buffer, size);

	MUTEX_UNLOCK();

	return ret;
}

RSU_OSAL_INT rsu_save_cpb_to_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (buffer == NULL || size == 0) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	ret = intf->cpb_ops.save_buf(buffer, size);

	MUTEX_UNLOCK();

	return ret;
}

RSU_OSAL_INT rsu_restore_cpb_from_buf(RSU_OSAL_VOID *buffer, RSU_OSAL_SIZE size)
{
	RSU_OSAL_INT ret;

	if (ctx.state != initialized) {
		RSU_LOG_ERR("Library not initialized");
		return -ELIB;
	}

	if (buffer == NULL || size == 0) {
		return -EARGS;
	}

	MUTEX_LOCK();

	if (intf->cpb_ops.corrupted()) {
		RSU_LOG_ERR("corrupted CPB");
		MUTEX_UNLOCK();
		return -ECORRUPTED_CPB;
	}

	ret = intf->cpb_ops.restore_buf(buffer, size);

	MUTEX_UNLOCK();

	return ret;
}
