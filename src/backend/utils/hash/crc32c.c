/*
 * crc32c.c
 *
 *  Created on: Mar 3, 2011
 *      Author: cmcdevitt
 */

#include "c.h"
#include "utils/pg_crc.h"

/* Tables used in Slicing-by-8 calculation of CRC32C.  Not needed for hardware crc32c */

/* Tables generated with code like the following:

#define CRCPOLY 0x82f63b78 // reversed 0x1EDC6F41
#define CRCINIT 0xFFFFFFFF

void init() {
	for (uint32 i = 0; i <= 0xFF; i++) {
		uint32 x = i;
		for (uint32 j = 0; j < 8; j++)
			x = (x>>1) ^ (CRCPOLY & (-(int32)(x & 1)));
		g_crc_slicing[0][i] = x;
	}

	for (uint32 i = 0; i <= 0xFF; i++) {
		uint32 c = g_crc_slicing[0][i];
		for (uint32 j = 1; j < 8; j++) {
			c = g_crc_slicing[0][c & 0xFF] ^ (c >> 8);
			g_crc_slicing[j][i] = c;
		}
	}
}
*/

#if (defined(__X86__) || defined(__i386__) || defined(i386) || defined(_M_IX86) || defined(__386__) || defined(__x86_64__) || defined(_M_X64))

#include "port/pg_crc32c.h"

pg_crc32
crc32c_port_bridge(pg_crc32 crc, const void* data, int length)
{
	return pg_crc32c(crc, data, length);
}

CRC32CFunctionPtr crc32c = &crc32c_port_bridge;

#else

/*
 * If not x86, we don't have hardware crc.  We might even not be little-endian.
 * play it safe, and use the stupid slow but 100% portable code.
 */
extern pg_crc32
crc32cSimple(pg_crc32 crc, const void* data, int length);

pg_crc32
crc32cSimple(pg_crc32 crc, const void* data, int length)
{
    const char* p_buf = (const char*) data;
    const char* p_end = p_buf + length;

    while (p_buf < p_end) {
    crc = crc_tableil8_o32[(crc ^ *p_buf++) & 0x000000FF] ^ (crc >> 8);
    }

    return crc;
}

CRC32CFunctionPtr crc32c = &crc32cSimple;
#endif
