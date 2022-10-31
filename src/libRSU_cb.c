/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU_cfg.h>
#include <libRSU_cb.h>
#include <libRSU_image.h>
#include <libRSU_misc.h>
#include <utils/RSU_logging.h>

static RSU_OSAL_FILE *cb_datafile;

RSU_OSAL_INT librsu_cb_file_init(RSU_OSAL_CHAR *filename)
{

	struct librsu_ll_intf *hal = librsu_get_ll_inf();
	if (hal == NULL) {
		RSU_LOG_ERR("Invalid argument");
		return -EINVAL;
	}

	if (cb_datafile != NULL) {
		RSU_LOG_DBG("cb_datafile is not NULL, close the file and make cb_datafile as NULL");
		hal->file.close(cb_datafile);
		cb_datafile = NULL;
	}

	if (filename == NULL) {
		return -EEXIST;
	}

	cb_datafile = hal->file.open(filename, RSU_FILE_READ);

	if (cb_datafile == NULL) {
		return -ENFILE;
	}

	return 0;
}

RSU_OSAL_VOID librsu_cb_file_cleanup(RSU_OSAL_VOID)
{
	struct librsu_ll_intf *hal = librsu_get_ll_inf();
	if (hal == NULL) {
		return;
	}

	if (cb_datafile != NULL) {
		hal->file.close(cb_datafile);
	}

	cb_datafile = NULL;
}

RSU_OSAL_INT librsu_cb_file(RSU_OSAL_VOID *buf, RSU_OSAL_INT len)
{
	struct librsu_ll_intf *hal = librsu_get_ll_inf();
	if (hal == NULL) {
		return -EINVAL;
	}

	return hal->file.read(buf, len, cb_datafile);
}

static RSU_OSAL_CHAR *cb_buffer;
static RSU_OSAL_INT cb_buffer_togo;

RSU_OSAL_INT librsu_cb_buf_init(RSU_OSAL_VOID *buf, RSU_OSAL_INT size)
{
	if (!buf || size <= 0) {
		return -EINVAL;
	}

	cb_buffer = (RSU_OSAL_CHAR *)buf;
	cb_buffer_togo = size;

	return 0;
}

RSU_OSAL_VOID librsu_cb_buf_cleanup(RSU_OSAL_VOID)
{
	cb_buffer = NULL;
	cb_buffer_togo = -1;
}

RSU_OSAL_INT librsu_cb_buf(RSU_OSAL_VOID *buf, RSU_OSAL_INT len)
{
	RSU_OSAL_INT read_len;

	if (!cb_buffer_togo) {
		return 0;
	}

	if (!cb_buffer || cb_buffer_togo < 0 || !buf || len < 0) {
		return -1;
	}

	if (cb_buffer_togo < len) {
		read_len = cb_buffer_togo;
	} else {
		read_len = len;
	}

	rsu_memcpy(buf, cb_buffer, read_len);

	cb_buffer += read_len;
	cb_buffer_togo -= read_len;

	if (!cb_buffer_togo) {
		cb_buffer = NULL;
	}

	return read_len;
}

RSU_OSAL_INT librsu_cb_program_common(struct librsu_hl_intf *intf, RSU_OSAL_INT slot,
				      rsu_data_callback callback, RSU_OSAL_INT rawdata)
{
	RSU_OSAL_INT part_num;
	RSU_OSAL_INT offset, c, size;
	RSU_OSAL_U8 *buf;
	RSU_OSAL_U8 *vbuf;
	RSU_OSAL_U32 cnt, done;
	RSU_OSAL_U32 x;
	struct rsu_slot_info info;
	struct rsu_image_state state;

	if (!intf) {
		return -ELIB;
	}

	if (librsu_cfg_writeprotected(slot)) {
		RSU_LOG_ERR("Trying to program a write protected slot");
		return -EWRPROT;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		return -ESLOTNUM;
	}

	SAFE_STRCPY(info.name, sizeof(info.name), intf->partition.name(part_num),
		    sizeof(info.name));

	if (intf->partition.offset(part_num, &info.offset) < 0) {
		RSU_LOG_ERR("Error in getting the partition offset");
		return -EBADF;
	}

	info.size = intf->partition.size(part_num);
	info.priority = intf->priority.get(part_num);

	if (intf->priority.get(part_num) > 0) {
		RSU_LOG_ERR("Trying to program a slot already in use");
		return -EPROGRAM;
	}

	if (!callback) {
		return -EARGS;
	}

	offset = 0;
	done = 0;

	if (librsu_image_block_init(&state)) {
		return -ELIB;
	}

	vbuf = rsu_malloc(IMAGE_BLOCK_SZ);
	if (vbuf == NULL) {
		RSU_LOG_ERR("Error in allocating memory");
		return -EARGS;
	}

	buf = rsu_malloc(IMAGE_BLOCK_SZ);
	if (buf == NULL) {
		rsu_free(vbuf);
		RSU_LOG_ERR("Error in allocating memory");
		return -EARGS;
	}

	while (!done) {
		cnt = 0;
		while (cnt < IMAGE_BLOCK_SZ) {
			c = callback(buf + cnt, IMAGE_BLOCK_SZ - cnt);
			if (c == 0) {
				done = 1;
				break;
			} else if (c < 0) {
				rsu_free(vbuf);
				rsu_free(buf);
				return -ECALLBACK;
			}

			cnt += c;
		}

		if (cnt == 0) {
			break;
		}

		if (!rawdata) {
			RSU_LOG_INF("Programming bit stream block");
			if (librsu_image_block_process(&state, buf, NULL, &info)) {
				rsu_free(vbuf);
				rsu_free(buf);
				return -EPROGRAM;
			}
		}

		size = intf->partition.size(part_num);
		if (size < 0) {
			RSU_LOG_ERR("Error in getting the slot size");
			rsu_free(vbuf);
			rsu_free(buf);
			return -ELOWLEVEL;
		}

		if ((offset + cnt) > (RSU_OSAL_U32)size) {
			RSU_LOG_ERR("Trying to program too much data into slot");
			rsu_free(vbuf);
			rsu_free(buf);
			return -ESIZE;
		}

		if (intf->data.write(part_num, offset, cnt, buf)) {
			RSU_LOG_ERR("Error in writing to slot");
			rsu_free(vbuf);
			rsu_free(buf);
			return -ELOWLEVEL;
		}

		if (intf->data.read(part_num, offset, cnt, vbuf)) {
			RSU_LOG_ERR("Error in reading from slot");
			rsu_free(vbuf);
			rsu_free(buf);
			return -ELOWLEVEL;
		}

		for (x = 0; x < cnt; x++) {
			if (vbuf[x] != buf[x]) {
				RSU_LOG_ERR("Expect %02X, got %02X @ 0x%08X", buf[x], vbuf[x],
					offset + x);
				rsu_free(vbuf);
				rsu_free(buf);
				return -ECMP;
			}
		}

		offset += cnt;
	}

	if (!rawdata && intf->priority.add(part_num)) {
		rsu_free(vbuf);
		rsu_free(buf);
		return -ELOWLEVEL;
	}
	rsu_free(vbuf);
	rsu_free(buf);
	return 0;
}

RSU_OSAL_INT librsu_cb_verify_common(struct librsu_hl_intf *intf, RSU_OSAL_INT slot,
				     rsu_data_callback callback, RSU_OSAL_INT rawdata)
{
	RSU_OSAL_INT part_num;
	RSU_OSAL_U32 offset;
	RSU_OSAL_U8 *buf;
	RSU_OSAL_U8 *vbuf;
	RSU_OSAL_INT cnt, c, done;
	RSU_OSAL_INT x;
	struct rsu_slot_info info;
	struct rsu_image_state state;

	if (!intf) {
		return -ELIB;
	}

	part_num = librsu_misc_slot2part(intf, slot);
	if (part_num < 0) {
		return -ESLOTNUM;
	}

	SAFE_STRCPY(info.name, sizeof(info.name), intf->partition.name(part_num),
		    sizeof(info.name));

	if (intf->partition.offset(part_num, &info.offset) < 0) {
		RSU_LOG_ERR("Error in getting the partition offset");
		return -EBADF;
	}

	info.size = intf->partition.size(part_num);
	info.priority = intf->priority.get(part_num);

	if (!rawdata && intf->priority.get(part_num) <= 0) {
		RSU_LOG_ERR("Trying to verify a slot not in use");
		return -EERASE;
	}

	if (!callback) {
		return -EARGS;
	}

	offset = 0;
	done = 0;

	if (librsu_image_block_init(&state)) {
		return -ELIB;
	}

	vbuf = rsu_malloc(IMAGE_BLOCK_SZ);
	if (vbuf == NULL) {
		RSU_LOG_ERR("Error in allocating memory");
		return -EARGS;
	}

	buf = rsu_malloc(IMAGE_BLOCK_SZ);
	if (buf == NULL) {
		rsu_free(vbuf);
		RSU_LOG_ERR("Error in allocating memory");
		return -EARGS;
	}

	while (!done) {
		cnt = 0;
		while (cnt < IMAGE_BLOCK_SZ) {
			c = callback(buf + cnt, IMAGE_BLOCK_SZ - cnt);
			if (c == 0) {
				done = 1;
				break;
			} else if (c < 0) {
				rsu_free(vbuf);
				rsu_free(buf);
				return -ECALLBACK;
			}

			cnt += c;
		}

		if (cnt == 0) {
			break;
		}

		if (intf->data.read(part_num, offset, cnt, vbuf)) {
			rsu_free(vbuf);
			rsu_free(buf);
			return -ELOWLEVEL;
		}

		if (!rawdata) {
			if (librsu_image_block_process(&state, buf, vbuf, &info)) {
				rsu_free(vbuf);
				rsu_free(buf);
				return -ECMP;
			}
			offset += cnt;
			continue;
		}

		for (x = 0; x < cnt; x++) {
			if (vbuf[x] != buf[x]) {
				RSU_LOG_ERR("Expect %02X, got %02X @ 0x%08X", buf[x], vbuf[x],
					offset + x);
				rsu_free(vbuf);
				rsu_free(buf);
				return -ECMP;
			}
		}

		offset += cnt;
	}
	rsu_free(vbuf);
	rsu_free(buf);
	return 0;
}
