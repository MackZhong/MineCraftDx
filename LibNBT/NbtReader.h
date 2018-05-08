#pragma once
#include "nbt.h"
#include "NbtTag.h"
#include "MemoryByteReader.h"
#include "GzipByteReader.h"
#include <iostream>
#include <fstream>
#include <memory>

namespace MineCraft {
	struct ChunkInformation {
		int relX;
		int relZ;
		Int32 offset;
		Int32 lastChange;
		Byte8 roundedSize;
	};
	namespace NbtReader {
		CompoundTag* LoadFromFile(const wchar_t* filePathName, NbtCommpressType* fileType = nullptr);

		CompoundTag* LoadFromData(const Byte8* data, UInt length, NbtCommpressType* fileType = nullptr);

		CompoundTag* LoadFromUncompressedData(ByteBuffer* buffer);

		CompoundTag* LoadRegionFile(const wchar_t* filePathName);

		CompoundTag* LoadRegionData(const Byte8* data, UInt length);
	}
}