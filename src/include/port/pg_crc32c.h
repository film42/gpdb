#ifndef PG_CRC32C_H
#define PG_CRC32C_H

/** Pointer to a function that computes a CRC32C checksum.
@arg crc Previous CRC32C value, or crc32c_init().
@arg data Pointer to the data to be checksummed.
@arg length length of the data in bytes.
*/
typedef uint32 pg_crc32;
typedef pg_crc32 (*crc32c_function_ptr)(pg_crc32 crc, const void* data, int length);

#if defined(USE_SSE42_CRC32C)
extern pg_crc32 crc32c_hardware_32(pg_crc32 crc, const void* data, int length);
extern pg_crc32 crc32c_hardware_64(pg_crc32 crc, const void* data, int length);
#elif defined(USE_SSE42_CRC32C_WITH_RUNTIME_CHECK)
extern pg_crc32 crc32c_hardware_32(pg_crc32 crc, const void* data, int length);
extern pg_crc32 crc32c_hardware_64(pg_crc32 crc, const void* data, int length);
extern pg_crc32 crc32c_slicing_by_8(pg_crc32 crc, const void* data, int length);
extern pg_crc32 crc32c_detect_best_method(pg_crc32 crc, const void* data, int length);
#else
extern pg_crc32 crc32c_slicing_by_8(pg_crc32 crc, const void* data, int length);
#endif

extern crc32c_function_ptr pg_crc32c;

#endif   /* PG_CRC32C_H */
