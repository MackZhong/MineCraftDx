#pragma once
#include <memory>
#include <string>
#include "ByteBuffer.h"
#include "NbtEntry.h"

namespace MineCraft {
	enum NbtCommpressType {
		Uncompressed = 0,
		GzipCommpressed = 1,
		ZlibCompressed = 2
	};
	const char* TypeName(NbtTagType type) {
		switch (type) {
		case NbtTagType::End:
			return "End";
		case NbtTagType::Byte:
			return "Byte";
		case NbtTagType::Short:
			return "Short";
		case NbtTagType::Int:
			return "Int";
		case NbtTagType::Long:
			return "Long";
		case NbtTagType::Float:
			return "Float";
		case NbtTagType::Double:
			return "Double";
		case NbtTagType::ByteArray:
			return "Byte Array";
		case NbtTagType::String:
			return "String";
		case NbtTagType::List:
			return "List";
		case NbtTagType::Compound:
			return "Compound";
		case NbtTagType::IntArray:
			return "Int Array";
		case NbtTagType::LongArray:
			return "Long Array";
		}
		return "Error";
	}

	using TagPtr = class NbtTag*;

	class  LIB_NBT_EXPORT NbtTag {
	private:
		wchar_t* m_Name{ nullptr };
		NbtTagType m_Type{ NbtTagType::Null };

	protected:
		size_t m_Size{ 0 };

	private:
		void SetName(const wchar_t* name) {
			size_t length = wcslen(name);
			m_Name = new wchar_t[length + 1];
			wcscpy_s(m_Name, length, name);
		}
		void Clear() {
			if (nullptr != m_Name) {
				delete[] m_Name;
				m_Name = nullptr;
			}
		}

	protected:
		virtual void ClearValues() = 0;

	public:
		~NbtTag() {
			this->Clear();
			this->ClearValues();
		}
		NbtTag() { }
		NbtTag(NbtTagType type) : m_Type(type) {}
		NbtTag(const wchar_t* name) {
			SetName(name);
		}
		NbtTag(const wchar_t* name, NbtTagType type) : m_Type(type) {
			SetName(name);
		}
		NbtTag(const StringW& name, NbtTagType type) : m_Type(type) {
			SetName(name.c_str());
		}

		const wchar_t* Name() const { return m_Name; };
		const NbtTagType& Type() const { return m_Type; }
		const char* TypeName() const { return MineCraft::TypeName(m_Type); };

		virtual int Read(ByteBuffer* buffer) = 0;

		virtual void* GetValue() const = 0;
		size_t GetSize() const { return m_Size; }
		static TagPtr FromType(NbtTagType type, const StringW& name);
	};

	template<typename T, NbtTagType TYPE> class  LIB_NBT_EXPORT NbtTagBasic :public NbtTag {
	private:
		T m_Value;

	public:
		NbtTagBasic() : NbtTag(TYPE) { m_Size = sizeof(T); }
		NbtTagBasic(const wchar_t* name) : NbtTag(name, TYPE), m_Size(sizeof(T)) {}

		virtual int Read(ByteBuffer* buffer) override {
			switch (TYPE) {
				break;
			case NbtTagType::Byte:
				m_Value = buffer->ReadByte();
				break;
			case NbtTagType::Short:
				m_Value = buffer->ReadShort();
				break;
			case NbtTagType::Int:
				m_Value = buffer->ReadInt();
				break;
			case NbtTagType::Long:
				m_Value = buffer->ReadLong();
				break;
			case NbtTagType::Float:
				m_Value = buffer->ReadFloat();
				break;
			case NbtTagType::Double:
				m_Value = buffer->ReadDouble();
				return sizeof(m_Value);
			case NbtTagType::String:
				m_Value = buffer->ReadString();
				return m_Value.size();
			case NbtTagType::End:
				break;
			default:
				throw "Error tag type";
			}
		}

		virtual void* GetValue() const override {
			return (void*)&m_Value;
		};

		virtual void ClearValues() override {};
	};
	using ByteTag = NbtTagBasic<Byte8, NbtTagType::Byte>;
	using ShortTag = NbtTagBasic<Short16, NbtTagType::Short>;
	using IntTag = NbtTagBasic<Int32, NbtTagType::Int>;
	using LongTag = NbtTagBasic<Long64, NbtTagType::Long>;
	using FloatTag = NbtTagBasic<Float32, NbtTagType::Float>;
	using DoubleTag = NbtTagBasic<Double64, NbtTagType::Double>;
	using StringTag = NbtTagBasic<StringW, NbtTagType::Double>;

	template<typename T, NbtTagType TYPE> class  LIB_NBT_EXPORT NbtTagArray :public NbtTag {
	private:
		T * m_Values{ nullptr };

	private:
		virtual void ClearValues() override {
			if (nullptr != m_Values) {
				delete[] m_Values;
				m_Values = nullptr;
			}
			m_Size = 0;
		}
		void SetValues(T* values, size_t size) {
			this->ClearValues();
			m_Size = size;
			m_Values = new T[size];
			memcpy(m_Values, values, size);
		}

	public:
		NbtTagArray() : NbtTag(TYPE) {}
		NbtTagArray(const wchar_t* name) : NbtTag(name, TYPE) {}

		virtual int Read(ByteBuffer* buffer) override {
			m_Size = buffer->ReadInt();
			m_Values = new T[m_Size];

			switch (TYPE) {
			case NbtTagType::ByteArray:
				return buffer->ReadBytes(size, m_Values);
			case NbtTagType::IntArray:
				std::vector<Int32> values;
				for (UInt i = 0; i < size; i++) {
					values.push_back(buffer->ReadInt());
				}
				memcpy(m_Values, values.data(), size * sizeof(Int32));
				return values.size();
			case NbtTagType::LongArray:
				std::vector<Long64> values;
				for (UInt i = 0; i < size; i++) {
					values.push_back(buffer->ReadLong());
				}
				memcpy(m_Values, values.data(), size * sizeof(Long64));
				return values.size();
			}
			throw "Error tag type";
		}
		virtual void* GetValue() const {
			return (void*)m_Value;
		};
	};
	using ByteArrayTag = NbtTagArray<Byte8, NbtTagType::ByteArray>;
	using IntArrayTag = NbtTagArray<Int32, NbtTagType::IntArray>;
	using LongArrayTag = NbtTagArray<Long64, NbtTagType::LongArray>;

	class  LIB_NBT_EXPORT NbtTagList :public NbtTag {

	};

	class LIB_NBT_EXPORT NbtTagCompoent : public NbtTag {
	private:
		TagPtr * m_Entries{ nullptr };
		size_t m_Size{ 0 };

		virtual void ClearValues() override {
			if (nullptr != m_Entries) {
				delete[] m_Entries;
				m_Entries = nullptr;
			}
			m_Size = 0;
		}

	public:
		NbtTagCompoent() : NbtTag(L"", NbtTagType::Compound) {};

		virtual int Read(ByteBuffer* buffer) override {
			NbtTagType type = static_cast<NbtTagType>(buffer->ReadByte());
			std::vector<TagPtr> entries;
			while (NbtTagType::End != type) {
				StringW name = buffer->ReadString();
				TagPtr tag = NbtTag::FromType(type, name);
				if (nullptr == tag) {
					throw "Unknown type.";
				}
				tag->Read(buffer);
				entries.push_back(tag);

				type = static_cast<NbtTagType>(buffer->ReadByte());
			}
			this->SetEntries(entries.data(), entries.size());
		}

		void SetEntries(TagPtr* entries, size_t size) {
			this->ClearValues();
			m_Size = size;
			m_Entries = new TagPtr[size];
			memcpy(m_Entries, entries, size * sizeof(TagPtr));
		}

		TagPtr FindByName(StringW name)const {
			for (size_t i = 0; i < m_Size; i++) {
				if (m_Entries[i]->Name() == name) {
					return m_Entries[i];
				}
			}
			return nullptr;
		}

		const TagPtr* GetEntiries() const {
			return m_Entries;
		}
	};

}