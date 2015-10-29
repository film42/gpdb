#include "c.h"
#include "port/pg_crc32c.h"

#ifndef SUSE11SP2
#pragma GCC target ("sse4.2")
#endif
#include <nmmintrin.h>

#ifdef __APPLE__
/* Borrowed from https://github.com/qsnake/gfortran-osx/blob/bce58a7f491490ba9f312de7b2c98937ca9abcdc/usr/lib/gcc/x86_64-apple-darwin10.3.0/4.5.1/include/cpuid.h#L31 */
#define bit_SSE4_2 (1 << 20)
#endif

/* Hardware-accelerated CRC-32C (using CRC32 instruction) */
pg_crc32
crc32c_hardware_32(pg_crc32 crc, const void* data, int length)
{
	const char* p_buf = (const char*) data;
	/* alignment doesn't seem to help? */
	for (int i = 0; i < length / sizeof(uint32); i++)
	{
		crc = _mm_crc32_u32(crc, *(uint32*) p_buf);
		p_buf += sizeof(uint32);
	}

	/* This ugly switch is slightly faster for short strings than the straightforward loop */
	length &= sizeof(uint32) - 1;
	switch (length)
	{
	case 3:
		crc = _mm_crc32_u8(crc, *p_buf++);
	case 2:
		crc = _mm_crc32_u16(crc, *(uint16_t*) p_buf);
		break;
	case 1:
		crc = _mm_crc32_u8(crc, *p_buf);
		break;
	case 0:
		break;
	default:
		/* This should never happen */
		/*Assert(false);*/
		;
	}

	return crc;
}

/* Hardware-accelerated CRC-32C (using CRC32 instruction) */
pg_crc32
crc32c_hardware_64(pg_crc32 crc, const void* data, int length)
{
#ifndef __LP64__
    return crc32c_hardware_32(crc, data, length);
#else
    const char* p_buf = (const char*) data;
    uint64 crc64bit = crc;

    for (int i = 0; i < length / sizeof(uint64); i++)
    {
        crc64bit = _mm_crc32_u64(crc64bit, *(uint64*) p_buf);
        p_buf += sizeof(uint64);
    }

    uint32 crc32bit = (uint32) crc64bit;
    length &= 7;  /* same as mod 8 */

    /* Depending on remaining length, use a combination of 32, 16, and 8 bit crc32 instructions */
    switch (length)
    {
        case 7:
            crc32bit = _mm_crc32_u8(crc32bit, *p_buf++);
        case 6:
            crc32bit = _mm_crc32_u16(crc32bit, *(uint16*) p_buf);
            p_buf += 2;
        /* case 5 is below: 4 + 1 */
        case 4:
            crc32bit = _mm_crc32_u32(crc32bit, *(uint32*) p_buf);
            break;
        case 3:
            crc32bit = _mm_crc32_u8(crc32bit, *p_buf++);
        case 2:
            crc32bit = _mm_crc32_u16(crc32bit, *(uint16*) p_buf);
            break;
        case 5:
            crc32bit = _mm_crc32_u32(crc32bit, *(uint32*) p_buf);
            p_buf += 4;
        case 1:
            crc32bit = _mm_crc32_u8(crc32bit, *p_buf);
            break;
        case 0:
            break;
    }

    return crc32bit;
#endif
}

/*
 * Not every architecture supports a runtime check. This ensures
 * SSE4.2 can be enabled for those who have it, at complile time.
 */

#if defined(USE_SSE42_CRC32C)

#ifdef __LP64__
crc32c_function_ptr pg_crc32c = &crc32c_hardware_64;
#else
crc32c_function_ptr pg_crc32c = &crc32c_hardware_32;
#endif

#endif
