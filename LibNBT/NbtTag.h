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

	private:
		void Clear() {
			if (nullptr != m_Name) {
				delete[] m_Name;
				m_Name = nullptr;
			}
		}
		void SetName(const wchar_t* name) {
			if (nullptr != m_Name) {
				delete[] m_Name;
			}
			if (nullptr == name) {
				m_Name = nullptr;
				return;
			}
			size_t length = wcslen(name);
			m_Name = new wchar_t[length + 1];
			wcscpy_s(m_Name, length, name);
		}

	protected:
		int m_Size{ 0 };

	protected:
		virtual void ClearValues() = 0;

	public:
		~NbtTag() {
			this->Clear();
			this->ClearValues();
		}
		NbtTag() { }
		NbtTag(NbtTagType type) : m_Type(type) {}
		NbtTag(const wchar_t* name, NbtTagType type) : m_Type(type) {
			SetName(name);
		}
		NbtTag(const StringW& name, NbtTagType type) : m_Type(type) {
			SetName(name.c_str());
		}

		// ��ȡ��ǩ����
		const wchar_t* Name() const { return m_Name; };
		// ��ȡ��ǩ����
		const NbtTagType& Type() const { return m_Type; }
		// ��ȡ��ǩ�����ַ���
		const char* TypeName() const { return MineCraft::TypeName(m_Type); };
		// ��ȡ����ָ�룬������������ת��
		virtual void* GetValue() const = 0;
		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		virtual int GetValue(void* value) const = 0;
		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size���������������ֽ�����
		virtual void SetValue(void* value, int size = 1) = 0;
		// ���ش洢�����ݸ�����������Ϊ1��
		int GetSize() const { return m_Size; }

		// ���ڴ�������ݵ���ǩ�У����ض�ȡ���ֽ�����
		virtual int Read(ByteBuffer* buffer) = 0;

		// �������ʹ��������ֵ�NBT��ǩ
		static TagPtr FromType(NbtTagType type, const wchar_t* name = nullptr);
		//// �������ʹ����������ֵ�NBT��ǩ����
		//static TagPtr FromType(NbtTagType type, int count);
	};

	template<typename T, NbtTagType TYPE> class  LIB_NBT_EXPORT NbtTagBasic :public NbtTag {
	private:
		T m_Value;

	protected:
		virtual void ClearValues() override {};

	public:
		NbtTagBasic() : NbtTag(TYPE) {
			m_Size = 1;
			assert(IsBasicType(TYPE));
		}
		NbtTagBasic(const wchar_t* name) : NbtTag(name, TYPE) {
			m_Size = 1;
			assert(IsBasicType(TYPE));
		}
		NbtTagBasic(const StringW& name) : NbtTag(name, TYPE) {
			m_Size = 1;
			assert(IsBasicType(TYPE));
		}

		// ���ڴ�������ݵ���ǩ�У����ض�ȡ���ַ�����
		virtual int Read(ByteBuffer* buffer) override {
			int readed = buffer->ReadData<T>(&m_Value);
			assert(readed == m_Size * sizeof(T));
			return readed;
		}

		// ��ȡ����ָ�룬������������ת����
		// ����	Int32 data = *(tag->GetValue());
		virtual void* GetValue() const override {
			return (void*)&m_Value;
		};

		// ��ȡ���ݣ��������Ϊ���ݵĵ�ַ�����ض�ȡ���ַ�����
		// ����	Int32 data;
		//		int size = tag->GetValue(&data);
		virtual int GetValue(void* value) const override {
			memcpy(value, &m_Value, sizeof(T));
			return m_Size * sizeof(T);
		};

		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size����ʹ��
		// ����	Int32 data;
		//		tag->SetValue((void*)&data);
		virtual void SetValue(void* value, int size = 1) override {
			memcpy(&m_Value, value, sizeof(T));
		};
	};

	using ByteTag = NbtTagBasic<Byte8, NbtTagType::Byte>;
	using ShortTag = NbtTagBasic<Short16, NbtTagType::Short>;
	using IntTag = NbtTagBasic<Int32, NbtTagType::Int>;
	using LongTag = NbtTagBasic<Long64, NbtTagType::Long>;
	using FloatTag = NbtTagBasic<Float32, NbtTagType::Float>;
	using DoubleTag = NbtTagBasic<Double64, NbtTagType::Double>;

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

	public:
		NbtTagArray() : NbtTag(TYPE) {
			assert(IsArrayType(TYPE));
		}
		NbtTagArray(const wchar_t* name) : NbtTag(name, TYPE) {
			assert(IsArrayType(TYPE));
		}
		NbtTagArray(const std::wstring& name) : NbtTag(name, TYPE) {
			assert(IsArrayType(TYPE));
		}

		// ���ڴ�������ݵ���ǩ�У����ض�ȡ���ֽ�����
		virtual int Read(ByteBuffer* buffer) override {
			ClearValues();
			m_Size = buffer->ReadInt();
			if (0 == m_Size) {
				return m_Size;
			}
			m_Values = new T[m_Size];

			return buffer->ReadData<T>(m_Values, m_Size);
		}

		// ��ȡ����ָ�룬������������ת����
		// ����Int32* data = (Int32*)tag->GetValue();
		virtual void* GetValue() const {
			return (void*)m_Values;
		};

		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		// ����	Int32* data = new Int32[tag->GetSize()];
		//		int size = tag->GetValue((void*)data);
		virtual int GetValue(void* value) const {
			if (nullptr == m_Values) {
				return 0;
			}
			memcpy(value, m_Values, m_Size * sizeof(T));
			return m_Size * sizeof(T);
		}

		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size�����ݵ�����
		// ����	Int32* data = new Int32[6];
		//		tag->SetValue((void*)data, 6);
		virtual void SetValue(void* value, int size = 1) override {
			this->ClearValues();
			m_Size = size;
			if (0 == size) {
				return;
			}
			m_Values = new T[size];
			memcpy(m_Values, value, size * sizeof(T));
		};
	};
	using ByteArrayTag = NbtTagArray<Byte8, NbtTagType::ByteArray>;
	using IntArrayTag = NbtTagArray<Int32, NbtTagType::IntArray>;
	using LongArrayTag = NbtTagArray<Long64, NbtTagType::LongArray>;

	class  LIB_NBT_EXPORT StringTag :public NbtTag {
	private:
		wchar_t * m_Values{ nullptr };

	private:
		virtual void ClearValues() override {
			if (nullptr != m_Values) {
				delete[] m_Values;
				m_Values = nullptr;
			}
			m_Size = 0;
		}

	public:
		StringTag() : NbtTag(NbtTagType::String) {
		}
		StringTag(const wchar_t* name) : NbtTag(name, NbtTagType::String) {
		}
		StringTag(const std::wstring& name) : NbtTag(name, NbtTagType::String) {
		}

		// ���ڴ�������ݵ���ǩ�С�
		virtual int Read(ByteBuffer* buffer) override {
			m_Size = buffer->ReadString(&m_Values, buffer->ReadShort());
			return m_Size;
		}

		// ��ȡ����ָ�룬������������ת����
		// ����	wchar_t* str = (wchar_t*)tag->GetValue();
		virtual void* GetValue() const {
			return (void*)m_Values;
		};

		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		// ע��NbtTag::GetSize()���ص����ַ���������ռ�Ҫ���ַ�����β����
		// ����	wchar_t* str = new wchar_t[tag->GetSize() + 1];
		//		int length = tag->GetValue((void*)str);
		virtual int GetValue(void* value) const {
			if (nullptr == m_Values) {
				return 0;
			}
			wcscpy_s((wchar_t*)value, m_Size, m_Values);
			return m_Size;
		}

		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ���ַ�����ָ��
		// size���ַ���
		// ����	wchar_t* str = L"Test";
		//		tag->SetValue((void*)str, wcslen(str));
		virtual void SetValue(void* value, int size = 1) override {
			this->ClearValues();
			m_Size = size;
			if (0 == size)
				return;

			m_Values = new wchar_t[size + 1];
			wcscpy_s(m_Values, size, (const wchar_t*)value);
		}
	};

	class  LIB_NBT_EXPORT ListTag :public NbtTag {
	private:
		TagPtr * m_Values{ nullptr };
		NbtTagType m_TagId{ NbtTagType::End };

	protected:
		virtual void ClearValues() override {
			if (nullptr != m_Values) {
				for (int i = 0; i < m_Size; i++) {
					if (nullptr != m_Values[i]) {
						delete m_Values[i];
					}
				}
				delete[] m_Values;
				m_Values = nullptr;
			}
			m_Size = 0;
			m_TagId = NbtTagType::End;
		}

		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size����ʹ��
		// ����	Int32* data = new Int32[6];
		//		tag->SetValue((void*)data, 6);
		virtual void SetValue(void* value, int size = 1) override {
			this->ClearValues();
			m_Size = size;
			if (0 == size) {
				return;
			}
			m_Values = new TagPtr[size];
			TagPtr* tags = (TagPtr*)value;
			for (int i = 0; i < size; i++) {
				*m_Values[i] = *tags[i];
			}
		}

	public:
		ListTag() : NbtTag(NbtTagType::List) {
		}
		ListTag(const wchar_t* name) : NbtTag(name, NbtTagType::List) {
		}
		ListTag(const std::wstring& name) : NbtTag(name, NbtTagType::List) {
		}

		// ���ڴ�������ݵ���ǩ�С�
		virtual int Read(ByteBuffer* buffer) override {
			ClearValues();
			m_TagId = static_cast<NbtTagType>(buffer->ReadByte());
			if (NbtTagType::End == m_TagId) {
				return m_Size;
			}
			m_Size = buffer->ReadInt();
			m_Values = new TagPtr[m_Size];
			for (int i = 0; i < m_Size; i++) {
				m_Values[i] = NbtTag::FromType(m_TagId);
				m_Values[i]->Read(buffer);
			}
			return m_Size;
		}

		// ��ȡ����ָ�룬������������ת����
		// ���ص���TagPtr��Ҳ��NbtTag*�����͵�ָ�����顣
		// ����	TagPtr* tags = (TagPtr*)tag->GetValue();
		// ��������һ��Ԫ�أ�wchar_t* name = tags[1]->Name();
		virtual void* GetValue() const {
			return (void*)m_Values;
		};

		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		// ����	TagPtr* tags = new TagPtr[tag->GetSize()];
		//		int count = tag->GetValue((void*)tags);
		// ע��ָ�������е�������ԭ��ǩ���棬�����и��ơ�
		virtual int GetValue(void* value) const {
			if (nullptr == m_Values) {
				return 0;
			}
			memcpy(value, m_Values, m_Size * sizeof(TagPtr));
			return m_Size * sizeof(TagPtr);
		}
	};

	class LIB_NBT_EXPORT CompoundTag : public NbtTag {
	private:
		TagPtr * m_Entries{ nullptr };

	protected:
		virtual void ClearValues() override {
			if (nullptr != m_Entries) {
				for (int i = 0; i < m_Size; i++) {
					if (nullptr != m_Entries[i]) {
						delete m_Entries[i];
					}
				}
				delete[] m_Entries;
				m_Entries = nullptr;
			}
			m_Size = 0;
		}

	public:
		CompoundTag() : NbtTag(NbtTagType::Compound) {};
		CompoundTag(const wchar_t* name) : NbtTag(name, NbtTagType::Compound) {};
		CompoundTag(const std::wstring& name) : NbtTag(name, NbtTagType::Compound) {};

		// ���ڴ�������ݵ���ǩ�С�
		virtual int Read(ByteBuffer* buffer) override {
			std::vector<TagPtr> entries;
			NbtTagType type;
			wchar_t* name = nullptr;
			while (NbtTagType::End != (type = static_cast<NbtTagType>(buffer->ReadByte()))) {
				buffer->ReadString(&name, buffer->ReadShort());
				TagPtr tag = NbtTag::FromType(type, name);
				if (nullptr == tag) {
					throw "Unknown type.";
				}
				tag->Read(buffer);
				entries.push_back(tag);
			}
			if (nullptr != name) {
				delete[] name;
			}
			this->SetValue((void*)entries.data(), (int)entries.size());
			return m_Size;
		}

		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size����ʹ��
		// ����	Int32* data = new Int32[6];
		//		tag->SetValue((void*)data, 6);
		virtual void SetValue(void* value, int size = 1) override {
			this->ClearValues();
			m_Size = size;
			if (0 == size) {
				return;
			}
			m_Entries = new TagPtr[size];
			TagPtr* entries = (TagPtr*)value;
			for (int i = 0; i < m_Size; i++) {
				*m_Entries[i] = *entries[i];
			}
		}

		// ��ȡ����ָ�룬������������ת����
		// ���ص���TagPtr��Ҳ��NbtTag*�����͵�ָ�����顣
		// ����	TagPtr* tags = (TagPtr*)tag->GetValue();
		// ��������һ��Ԫ�أ�wchar_t* name = tags[1]->Name();
		virtual void* GetValue() const {
			return (void*)m_Entries;
		};

		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		// ����	TagPtr* tags = new TagPtr[tag->GetSize()];
		//		int count = tag->GetValue((void*)tags);
		// ע��ָ�������е�������ԭ��ǩ���棬�����и��ơ�
		virtual int GetValue(void* value) const {
			if (nullptr == m_Entries) {
				return 0;
			}
			memcpy(value, m_Entries, m_Size * sizeof(TagPtr));
			return m_Size * sizeof(TagPtr);
		}

		// �������ֲ��ұ�ǩ
		TagPtr FindByName(const wchar_t* name)const {
			if (nullptr == name) {
				return nullptr;
			}
			for (size_t i = 0; i < m_Size; i++) {
				if (nullptr == m_Entries[i]->Name()) {
					continue;
				}
				if (_wcsnicmp(m_Entries[i]->Name(), name, m_Entries[i]->GetSize()) == 0) {
					return m_Entries[i];
				}
			}
			return nullptr;
		}

		// ����һ����ǩ���������֡�����
		NbtTag* Add(NbtTagType type, const wchar_t* name, void* value) {
			NbtTag* tag = NbtTag::FromType(type, name);
			tag->SetValue(value);

			return tag;
		}
	};

}