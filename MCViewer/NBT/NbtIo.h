#pragma once
#include "NbtReaderWriter.h"
#include "NbtTag.h"

namespace MC {
	class NbtIo
	{
	public:
		//static class CompoundTag* readCompressed(FS::ifstream& in);

		//static void writeCompressed(const CompoundTag* tag, FS::ofstream& out);

		//static class CompoundTag* decompress(const char* buffer, size_t size, COMPRESSION_SCHEME scheme = COMPRESSION_SCHEME_GZIP);

		//static UniquePtr compress(CompoundTag* tag);

		//inline static void writeTo(const CompoundTag* tag, NbtWriter* pdos) {
		//	NbtTag::writeNamedTag(tag, pdos);
		//}

		//static void write(const CompoundTag* tag, FS::path& file);

		//static void safeWrite(const CompoundTag* tag, FS::path file);

		//static CompoundTag* readFrom(NbtReader* pdis);

		//static CompoundTag* read(FS::path& file);
	};
}