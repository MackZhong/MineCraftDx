#pragma once
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
			z_stream ds{ 0 };
			ds.next_in = (Bytef*)data;
			ds.avail_in = size;
			int result = inflateInit2(&ds, 0);
			if (Z_OK != result) {
				throw "zLib init failed.";
			}

			Bytef* unCompressed = (Bytef*)malloc(BUFFER_SIZE);
			while (ds.avail_in != 0) {
				ds.next_out = unCompressed;
				ds.avail_out = BUFFER_SIZE;
				result = inflate(&ds, Z_NO_FLUSH);
				switch (result) {
				case Z_STREAM_END:
				case Z_OK:
					if (ds.avail_in > 0) {
						unCompressed = (Bytef*)realloc(unCompressed, ds.total_out + BUFFER_SIZE);
						ds.next_out = unCompressed + ds.total_out;
						ds.avail_out = BUFFER_SIZE;
					}
					break;
				default:
					throw "zLib uncommpression error.";
					break;
				}
				MemoryByteReader((Byte8*)unCompressed, ds.total_out);
				free(unCompressed);
				unCompressed = nullptr;
			}
		};
	};
}