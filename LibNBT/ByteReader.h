#pragma once
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

	std::wstring UTF8ToWString(const Byte8* srcString, unsigned int srcLength) {
		int dstLength = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, NULL, 0);
		std::unique_ptr<wchar_t[]> pwBuf = std::make_unique<wchar_t[]>(dstLength + 1);
		int length = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, pwBuf.get(), dstLength);
		return std::wstring(pwBuf.get());
	};

	int UTF8ToWString(wchar_t** ppDstString, const Byte8* srcString, unsigned int srcLength) {
		if (nullptr != *ppDstString) {
			delete[] * ppDstString;
			*ppDstString = nullptr;
		}
		int dstLength = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, NULL, 0);
		*ppDstString = new wchar_t[dstLength + 1];
		return MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, *ppDstString, dstLength);
	};

	int WStringToUTF8(const std::wstring& str, char* outStr) {
		int srcLength = (int)str.length();
		int dstLength = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), srcLength, NULL, 0, NULL, NULL); ;
		if (nullptr != outStr) {
			assert(_msize(outStr) > dstLength);
			WideCharToMultiByte(CP_UTF8, 0, str.c_str(), srcLength, outStr, dstLength, NULL, NULL);
		}
		return dstLength;
	};

	__interface IByteReader {
		Byte8 ReadByte();
		int ReadBytes(UInt length, Byte8* buffer);
		void Reset();
	};

	using ByteReader = IByteReader * ;
}
