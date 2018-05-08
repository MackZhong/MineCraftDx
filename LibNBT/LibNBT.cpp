#include "stdafx.h"
#include "NbtTag.h"
#include "ByteBuffer.h"
#include "GzipByteReader.h"
#include "MemoryByteReader.h"
#include "NbtReader.h"

namespace MineCraft {
	void test() {
		ByteTag tag;
	}


	NbtTag* NbtTag::FromType(NbtTagType type, const wchar_t* name) {
		switch (type) {
		case NbtTagType::End:
			return 0;
		case NbtTagType::Byte:
			return new ByteTag(name);
		case NbtTagType::Short:
			return new ShortTag(name);
		case NbtTagType::Int:
			return new IntTag(name);
		case NbtTagType::Long:
			return new LongTag(name);
		case NbtTagType::Float:
			return new FloatTag(name);
		case NbtTagType::Double:
			return new DoubleTag(name);
		case NbtTagType::ByteArray:
			return new ByteArrayTag(name);
		case NbtTagType::String:
			return new StringTag(name);
		case NbtTagType::List:
			return new ListTag(name);
		case NbtTagType::Compound:
			return new CompoundTag(name);
		case NbtTagType::IntArray:
			return new IntArrayTag(name);
		case NbtTagType::LongArray:
			return new LongArrayTag(name);
		}

		return nullptr;
	}

	//NbtTag* NbtTag::FromType(NbtTagType type, int count) {
	//	return nullptr;
	//}

}