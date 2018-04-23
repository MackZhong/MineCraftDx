#pragma once
#include <string>
#include <vector>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
//using ByteDevice = boost::iostreams::basic_array_source<__int8>;
//using ByteBuffer = boost::iostreams::stream_buffer<ByteDevice>;
#include <boost/iostreams/stream.hpp>
//#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <boost/locale/utf8_codecvt.hpp>
#include <sstream>

namespace MC {
	namespace FS = boost::filesystem;
	namespace IO = boost::iostreams;
	using IFilteringStream = boost::iostreams::filtering_istream;
	using OFilteringStream = boost::iostreams::filtering_ostream;

	extern const int MCREGION_VERSION_ID;
	extern const int ANVIL_VERSION_ID;
	extern const wchar_t* NETHER_FOLDER;
	extern const wchar_t* ENDER_FOLDER;

	using BoolArray = std::vector<bool>;
	using ByteArray = std::vector<__int8>;
	using ByteBuffer = std::shared_ptr<__int8[]>;
	using IntArray = std::vector<__int32>;
	using LongArray = std::vector<__int64>;

	inline int DebugMessage(const wchar_t* _Format, ...) {
		int _Result = 0;
#ifdef _DEBUG
		static wchar_t _Buffer[128];
		va_list _ArgList;
		__crt_va_start(_ArgList, _Format);
		_Result = __vswprintf_l(_Buffer, _Format, NULL, _ArgList);
		__crt_va_end(_ArgList);
		std::_Debug_message(_Buffer, __FILEW__, __LINE__);
#endif
		return _Result;
	}
}