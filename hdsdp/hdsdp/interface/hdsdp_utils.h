/** @file hdsdp\_utils.h
 *  @brief Define utilities for HDSDP
 *  @date 11/24/2022
 */

#ifndef hdsdp_utils_h
#define hdsdp_utils_h

#include "hdsdp.h"

/* Define macros */
#define HDSDP_FREE(var) do {free((var)); (var) = NULL;} while (0)
#define HDSDP_INIT(var, type, size) (var) = (type *) calloc(size, sizeof(type))
#define HDSDP_MEMCPY(dst, src, type, size) memcpy(dst, src, sizeof(type) * (size))
#define HDSDP_ZERO(var, type, size) memset(var, 0, sizeof(type) * (size))

#define POTLP_MAX(x, y) (x) >= (y) ? (x) : (y);
#define POTLP_MIN(x, y) (x) <= (y) ? (x) : (y);

#endif /* hdsdp_utils_h */
