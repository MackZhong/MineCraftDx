#pragma once
#include "nbt.h"
#include <string>
#include <cassert>
#include <memory>

namespace MineCraft {
	const size_t BUFFER_SIZE = 1024 * 1024;

	using UInt = unsigned int;

	using Byte8 = char;
	using Short16 = int16_t;
	using Int32 = int32_t;
	using Long64 = int64_t;
	using Float32 = float;
	using Double64 = double;
	using StringW = std::wstring;

	std::wstring UTF8ToWString(const Byte8* srcString, unsigned int srcLength);

	int UTF8ToWString(wchar_t** ppDstString, const Byte8* srcString, unsigned int srcLength);

	int WStringToUTF8(const std::wstring& str, char* outStr);

	__interface IByteReader {
		Byte8 ReadByte();
		int ReadBytes(UInt length, Byte8* buffer);
		void Reset();
	};

	using ByteReader = IByteReader * ;
}
