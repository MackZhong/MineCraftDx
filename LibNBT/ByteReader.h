#pragma once
#include "nbt.h"
#include <string>
#include <cassert>
#include <memory>

namespace MineCraft {
	const size_t UNCOMPRESS_BUFFER_SIZE = 1024 * 1024;
	const size_t STRING_CONVERT_SIZE = 64;

	using UInt = unsigned int;

	using Byte8 = char;
	using Short16 = int16_t;
	using Int32 = int32_t;
	using Long64 = int64_t;
	using Float32 = float;
	using Double64 = double;

	template<typename T> class TypeConvert {
	protected:
		static TypeConvert* instance;
		char _Str[STRING_CONVERT_SIZE];
		virtual inline const char* _Fmt() const {
			throw "Not supported";
		}

	public:
		const char* toString(T _Val) {
			ZeroMemory(this->_Str, STRING_CONVERT_SIZE);
			sprintf_s(this->_Str, STRING_CONVERT_SIZE, this->_Fmt(), _Val);
			return (this->_Str);
		}
		static TypeConvert<T>* Instance() {
			if (nullptr == TypeConvert<T>::instance)
				TypeConvert<T>::instance = new TypeConvert<T>;
			return TypeConvert<T>::instance;
		}
	};
	inline const char * TypeConvert<Byte8>::_Fmt() const {
		return "%hi";
	};
	inline const char * TypeConvert<Short16>::_Fmt() const {
		return "%hi";
	};
	inline const char * TypeConvert<Int32>::_Fmt() const {
		return "%i";
	};
	inline const char * TypeConvert<Long64>::_Fmt() const {
		return "%lli";
	};
	inline const char * TypeConvert<Float32>::_Fmt() const {
		return "%g";
	};
	inline const char * TypeConvert<Double64>::_Fmt() const {
		return "%lg";
	};

	using StringW = std::wstring;

	StringW UTF8ToWString(const Byte8* srcString, unsigned int srcLength);

	int UTF8ToWString(wchar_t** ppDstString, const Byte8* srcString, unsigned int srcLength);

	int WStringToUTF8(const StringW& str, char* outStr);

	__interface IByteReader {
		Byte8 ReadByte();
		int ReadBytes(UInt length, Byte8* buffer);
		void Reset();
	};

	using ByteReader = IByteReader * ;

}
