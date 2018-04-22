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
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/archive/binary_iarchive_impl.hpp>
#include <boost/archive/binary_oarchive_impl.hpp>
//using ByteStream = boost::archive::binary_iarchive;
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>
namespace FS = boost::filesystem;
namespace IO = boost::iostreams;

//#pragma comment(lib, "libboost_zlib-vc141-mt-gd-x64-1_67.lib")
#include <boost/iostreams/filtering_stream.hpp>
using IFilteringStream = boost::iostreams::filtering_istream;
using OFilteringStream = boost::iostreams::filtering_ostream;
#include <fstream>
#include <iostream>

// https://docs.oracle.com/javase/7/docs/api/java/io/DataInput.html#modified-utf-8

namespace MC {
	using ByteArray = std::vector<__int8>;
	using ByteBuffer = std::shared_ptr<__int8[]>;
	using IntArray = std::vector<__int32>;

	class NbtReader
	{
		std::shared_ptr<std::streambuf> m_Buffer;

	public:
		NbtReader(const IFilteringStream& stream) : m_Buffer(stream.rdbuf())
		{}
		NbtReader(const std::ifstream& stream) :m_Buffer(stream.rdbuf())
		{}

		ByteBuffer read(size_t size) {
			ByteBuffer ptr = std::make_unique<__int8[]>(size);
			m_Buffer->sgetn(ptr.get(), size);
			return ptr;
		}
		__int8 readByte() {
			__int8 v;
			m_Buffer->sgetn((char*)&v, sizeof(v));

			return v;
		}
		__int16 readShort() {
			__int8 v[2];
			m_Buffer->sgetn(v, sizeof(v));

			return (short)((v[0] << 8) | (v[1] & 0xff));
		};
		__int32 readInt() {
			__int8 v[4];
			m_Buffer->sgetn(v, sizeof(v));

			return (__int32)(((v[0] & 0xff) << 24) | ((v[1] & 0xff) << 16) |
				((v[2] & 0xff) << 8) | (v[3] & 0xff));
		};
		__int64 readLong() {
			__int8 v[8];
			m_Buffer->sgetn(v, sizeof(v));

			return (__int64)(((__int64)(v[0] & 0xff) << 56) |
				((__int64)(v[1] & 0xff) << 48) |
				((__int64)(v[2] & 0xff) << 40) |
				((__int64)(v[3] & 0xff) << 32) |
				((long)(v[4] & 0xff) << 24) |
				((long)(v[5] & 0xff) << 16) |
				((long)(v[6] & 0xff) << 8) |
				((long)(v[7] & 0xff)));
		};
		float readFloat() {
			__int32 v = readInt();
			return *(float*)&v;
		};
		double readDouble() {
			__int64 v = readLong();
			return *(double*)&v;
		}
		std::wstring readUTF() {
			std::wstring str;
			__int16 length = readShort();
			if (1 > length) {
				return L"";
			}
			auto data = std::make_unique<__int8[]>(length);
			//auto pos1 = m_Buffer->pubseekpos(0, BOOST_IOS::_Seekcur);
			auto readed = m_Buffer->sgetn(data.get(), length);
			//auto pos2 = m_Buffer->pubseekpos(0, BOOST_IOS::_Seekcur);
			for (int i = 0; i < length; i++) {
				unsigned __int8 a = data[i];
				unsigned __int8 b = data[i + 1];
				unsigned __int8 c = data[i + 2];
				if (a >> 7 == 0) {
					str += a;
				}
				else if (a >> 5 == 0x6 && b >> 6 == 0x2) {
					str += (wchar_t)(((a & 0x1F) << 6) | (b & 0x3F));
					i++;
				}
				else if (a >> 4 == 0xe && b >> 6 == 0x2 && c >> 6 == 0x2) {
					str += (wchar_t)(((a & 0x0F) << 12) | ((b & 0x3F) << 6) | (c & 0x3F));
					i += 2;
				}
				else {
					throw "Invalid UTF string";
					break;
				}
			}
			//__super::load(readed);
			//char buf[10];
			//__super::load_binary(buf, 10);
			std::wcout << str << std::endl;
			return str;
		}
	};

	class NbtWriter {
		//ByteStream m_Archive;
		std::shared_ptr<std::streambuf> m_Buffer;

	public:
		NbtWriter(const OFilteringStream& stream) : m_Buffer(stream.rdbuf())
		{}
		NbtWriter(const std::ofstream& stream) : m_Buffer(stream.rdbuf())
		{}

		void write(const ByteBuffer& buffer, size_t size) {
			m_Buffer->sputn(buffer.get(), size);
		};
		void writeByte(__int8 value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		}
		void writeShort(__int16 value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		}
		void writeInt(__int32 value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeLong(__int64 value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeFloat(float value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeDouble(double value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeUTF(const std::wstring& str) {
		}
	};
}