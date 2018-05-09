#pragma once
#include "nbt.h"
#include "MemoryByteReader.h"
#include <zlib.h>
#pragma comment(lib, "zlibwapi.lib")
#include <malloc.h>

namespace MineCraft {
	class LIB_NBT_EXPORT GzipByteReader : public MemoryByteReader {
	private:
		bool m_IsGzip{ true };

	public:
		GzipByteReader() = delete;
		GzipByteReader(const Byte8* data, UInt size, bool gzip = true)
			: m_IsGzip(gzip)
		{
			z_stream ds;
			ds.zalloc = Z_NULL;
			ds.zfree = Z_NULL;
			ds.opaque = Z_NULL;
			ds.avail_in = 0;
			ds.next_in = Z_NULL;

			int result = inflateInit2(&ds, gzip ? (MAX_WBITS | 16) : MAX_WBITS);
			// windowBits can also be greater than 15 for optional gzip decoding.  Add
			// 32 to windowBits to enable zlib and gzip decoding with automatic header
			// 	detection, or add 16 to decode only the gzip format(the zlib format will
			// 		return a Z_DATA_ERROR).
			if (Z_OK != result) {
				OutputDebugStringA(ds.msg);
				OutputDebugStringA("\n");
				throw "zLib init failed.";
			}

			ds.next_in = (Bytef*)data;
			ds.avail_in = size;
			Bytef* deCompressed = (Bytef*)malloc(UNCOMPRESS_BUFFER_SIZE);
			while (ds.avail_in != 0) {
				ds.next_out = deCompressed;
				ds.avail_out = UNCOMPRESS_BUFFER_SIZE;
				result = inflate(&ds, Z_NO_FLUSH);
				OutputDebugStringA(ds.msg);
				OutputDebugStringA("\n");
				switch (result) {
				case Z_DATA_ERROR:
					result = inflateEnd(&ds);
					result = inflateSync(&ds);
					throw ds.msg;
					break;
				case Z_NEED_DICT:
					throw ds.msg;
					break;
				case Z_STREAM_ERROR:
					throw ds.msg;
					break;
				case Z_MEM_ERROR:
					throw ds.msg;
					break;
				case Z_BUF_ERROR:
					throw ds.msg;
					break;
				case Z_STREAM_END:
					break;
				case Z_OK:
					break;
				default:
					throw ds.msg;
					break;
				}
				if (ds.avail_in > 0) {
					assert(0 == ds.avail_out);
					deCompressed = (Bytef*)realloc(deCompressed, ds.total_out + UNCOMPRESS_BUFFER_SIZE);
					ds.next_out = deCompressed + ds.total_out;
					ds.avail_out = UNCOMPRESS_BUFFER_SIZE;
				}
			}
			m_Length = ds.total_out;
			m_Offset = 0;
			m_Data = new Byte8[m_Length];
			memcpy(m_Data, deCompressed, m_Length);

			free(deCompressed);
			deCompressed = nullptr;
		};
	};
}