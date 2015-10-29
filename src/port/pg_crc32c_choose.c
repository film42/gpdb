#include "c.h"
#include "port/pg_crc32c.h"

/*
 * Detect if we have new Intel CPU instructions
 * This gets called the first time someone tries to use crc32c.
 */

#ifdef HAVE__GET_CPUID
#include <cpuid.h>
#endif

#ifdef HAVE__CPUID
#include <intrin.h>
#endif

pg_crc32
crc32c_detect_best_method(pg_crc32 crc, const void* data, int length)
{
	unsigned int exx[4] = {0, 0, 0, 0};

#if defined(HAVE__GET_CPUID)
	__get_cpuid(1, &exx[0], &exx[1], &exx[2], &exx[3]);
#elif defined(HAVE__CPUID)
	__cpuid(exx, 1);
#else
	#error cpuid instruction not available
#endif

    bool hasSSE42 = (exx[2] & (1 << 20)) != 0;	/* SSE 4.2 */

	if (hasSSE42)
	{
#ifdef __LP64__
		pg_crc32c = &crc32c_hardware_64;
#else
		pg_crc32c = &crc32c_hardware_32;
#endif
	}
	else
		pg_crc32c = &crc32c_slicing_by_8;

	return pg_crc32c(crc, data, length);
}

#if defined(USE_SSE42_CRC32C_WITH_RUNTIME_CHECK)
/* Function pointers to functions customized for the hardware platform */
crc32c_function_ptr pg_crc32c = &crc32c_detect_best_method;
#endif
