#include "rsu_linux_utils.h"
#include <string.h>

RSU_OSAL_INT get_devattr(const RSU_OSAL_CHAR *rsu_dev, const RSU_OSAL_CHAR *attr, RSU_OSAL_U64 *const value)
{
	RSU_OSAL_FILE *attr_file;
	RSU_OSAL_CHAR *buf;
	RSU_OSAL_INT size = RSU_FILE_OP_BUF_SIZE;

	buf = (RSU_OSAL_CHAR *)malloc(size);
	if (!buf) {
		RSU_LOG_ERR("error: failed to alloc buf\n");
		return -ENOMEM;
	}

	strncpy(buf, rsu_dev, size - strlen(rsu_dev) - 1);
	strncat(buf, "/", size - strlen(rsu_dev) - 1);
	strncat(buf, attr, size - strlen(rsu_dev) - 1);

	attr_file = fopen(buf, "r");
	if (!attr_file) {
		RSU_LOG_ERR("error: Unable to open device attribute file '%s'", buf);
		free(buf);
		return -EBADF;
	}

	if (fgets(buf, size, attr_file)) {
		*value = strtol(buf, NULL, 0);
		fclose(attr_file);
		free(buf);
		return 0;
	}

	fclose(attr_file);
	free(buf);
	return -EACCES;
}

RSU_OSAL_INT put_devattr(const RSU_OSAL_CHAR *rsu_dev, const RSU_OSAL_CHAR *attr, RSU_OSAL_U64 value)
{
	RSU_OSAL_FILE *attr_file;
	RSU_OSAL_CHAR *buf;
	RSU_OSAL_INT size = RSU_FILE_OP_BUF_SIZE;

	buf = (RSU_OSAL_CHAR *)malloc(size);
	if (!buf) {
		RSU_LOG_ERR("error: failed to alloc buf\n");
		return -ENOMEM;
	}

	strncpy(buf, DEFAULT_RSU_DEV, size - strlen(rsu_dev) - 1);
	strncat(buf, "/", size - strlen(rsu_dev) - 1);
	strncat(buf, attr, size - strlen(rsu_dev) - 1);

	attr_file = fopen(buf, "w");
	if (!attr_file) {
		RSU_LOG_ERR("error: Unable to open device attribute file '%s'", buf);
		free(buf);
		return -EBADF;
	}

	snprintf(buf, size, "%lli", value);

	if (fputs(buf, attr_file) > 0) {
		fclose(attr_file);
		free(buf);
		return 0;
	}

	fclose(attr_file);
	free(buf);
	return -EACCES;
}
