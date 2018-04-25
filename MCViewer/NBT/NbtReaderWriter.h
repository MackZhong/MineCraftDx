#pragma once
#include "mc.h"

// https://docs.oracle.com/javase/7/docs/api/java/io/DataInput.html#modified-utf-8

namespace MC {

	class NbtReader
	{
		//std::shared_ptr<std::streambuf> m_Buffer;
		std::streambuf* m_Buffer;

	public:
		NbtReader(const IFilteringStream& stream) : m_Buffer(stream.rdbuf())
		{}
		NbtReader(const std::ifstream& stream) :m_Buffer(stream.rdbuf())
		{}
		NbtReader(std::streambuf* buf) :m_Buffer(buf)
		{}

		const std::shared_ptr<char> read(size_t size) {
			auto ptr = new char[size];
			m_Buffer->sgetn(ptr, size);
			return std::shared_ptr<char>(ptr);
		}
		__int8 readByte() {
			__int8 v;
			m_Buffer->sgetn((char*)&v, sizeof(v));

			return v;
		}
		__int16 readShort() {
			__int16 v;
			m_Buffer->sgetn(&v, sizeof(v));

			return (__int16)((v[0] << 8) | (v[1] & 0xff));
		};
		__int32 readInt() {
			__int8 v[4];
			m_Buffer->sgetn(v, sizeof(v));
			//v[3] = m_Buffer->sgetc();
			//v[2] = m_Buffer->sgetc();
			//v[1] = m_Buffer->sgetc();
			//v[0] = m_Buffer->sgetc();
			//return *(__int32*)v;
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

		void write(const char* buffer, size_t size) {
			m_Buffer->sputn(buffer, size);
		};
		void writeByte(__int8 value) {
			m_Buffer->sputn((char*)&value, sizeof(value));
		}
		void writeShort(__int16 value) {
			// TODO: little endian conversion
			m_Buffer->sputn((char*)&value, sizeof(value));
		}
		void writeInt(__int32 value) {
			// TODO: little endian conversion
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeLong(__int64 value) {
			// TODO: little endian conversion
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeFloat(float value) {
			// TODO: little endian conversion
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeDouble(double value) {
			// TODO: little endian conversion
			m_Buffer->sputn((char*)&value, sizeof(value));
		};
		void writeUTF(const std::wstring& str) {
			// TODO: write UTF8 string
			//using Utf8Conv = boost::locale::utf8_codecvt<wchar_t>;
			//Utf8Conv::state_type st;
			//Utf8Conv::to_unicode(Utf8Conv::to_unicode_state, )
			//	auto state = conv.initial_state(boost::locale::generic_codecvt_base::to_unicode_state);
			//auto buf = str.c_str(); boost::locale::utf8_codecvt
			//	auto pt = conv.to_unicode(state, );

		}
	};
}