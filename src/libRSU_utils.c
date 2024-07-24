/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <utils/RSU_utils.h>
/*
 * split_line() - Split a line buffer into words, leaving the words null
 *                terminated in place in the buffer.
 * line - Pointer to line buffer
 * words - array of pointers to store start of words
 * cnt - number of pointers in the words array
 *
 * Returns the number of words found
 */
RSU_OSAL_U32 split_line(RSU_OSAL_CHAR *line, RSU_OSAL_CHAR **words, RSU_OSAL_U32 cnt)
{
	if(line == NULL || words == NULL || cnt == 0 ) {
		return 0;
	}

	RSU_OSAL_CHAR *c = line;
	RSU_OSAL_U32 y = 0;

	while ((*c != '\0') && (y < cnt)) {
		/* Look for start of a word */
		while (*c <= ' ') {
			if (*c == '\0')
				return y;
			c++;
		}

		/* Remember start address of word and look for the end */
		words[y++] = c;
		while (*c > ' ')
			c++;

		/* '\0' terminate words that are not at the end of the str */
		if (*c != '\0') {
			*c = '\0';
			c++;
		}
	}
	return y;
}
