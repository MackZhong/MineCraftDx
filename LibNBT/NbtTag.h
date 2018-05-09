#pragma once
#include "nbt.h"
#include <memory>
#include <string>
#include "ByteBuffer.h"
#include "NbtEntry.h"
#include <iostream>

namespace MineCraft {
	enum NbtCommpressType {
		Uncompressed = 0,
		GzipCommpressed = 1,
		ZlibCompressed = 2
	};

	inline const char* TypeName(NbtTagType type) {
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

	class LIB_NBT_EXPORT NbtTag/* : public IPayload*/ {
	private:
		wchar_t* m_Name{ nullptr };

	protected:
		int m_Size{ 0 };
		NbtTagType m_Type{ NbtTagType::Null };

	protected:
		void ClearName() {
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
			wcscpy_s(m_Name, length + 1, name);
		}

		virtual void ClearValues() {};

	public:
		virtual ~NbtTag() {
			this->ClearName();
			this->ClearValues();
		};

		NbtTag() { }

		NbtTag(NbtTagType type) : m_Type(type) {}

		NbtTag(const wchar_t* name, NbtTagType type) : m_Type(type) {
			SetName(name);
		}

		NbtTag(const StringW& name, NbtTagType type) : m_Type(type) {
			SetName(name.c_str());
		}

		virtual NbtTag* Clone() const = 0;
		// ��ȡ��ǩ����
		const wchar_t* Name() const { return m_Name; };
		// ��ȡ��ǩ����
		const NbtTagType& Type() const { return m_Type; }
		// ��ȡ��ǩ�����ַ���
		const char* TypeName() const { return MineCraft::TypeName(m_Type); };
		// ��ȡ����ָ�룬������������ת��
		virtual void* Value() const = 0;
		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		virtual int GetValue(void* value) const = 0;
		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size���������������ֽ�����
		virtual void SetValue(void* value, int size = 1) = 0;
		// ���ش洢�����ݸ�����������Ϊ1��
		inline int Size() const {
			return m_Size;
		}

		// ���ڴ�������ݵ���ǩ�У����ض�ȡ���ֽ�����
		virtual int Read(ByteBuffer* buffer) = 0;

		// �������ʹ��������ֵ�NBT��ǩ
		static NbtTag* FromType(NbtTagType type, const wchar_t* name = nullptr);
		//// �������ʹ����������ֵ�NBT��ǩ����
		//static TagPtr FromType(NbtTagType type, int count);

		virtual std::wostream& OutString(std::wostream& out) const = 0;

		virtual std::wostream& OutValueString(std::wostream& out) const = 0;
	};

	using TagPtr = NbtTag * ;

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
		NbtTagBasic(const NbtTagBasic& rhs) { *this = rhs; }

		NbtTagBasic& operator=(const NbtTagBasic& rhs) {
			this->SetName(rhs.Name());
			this->m_Type = rhs.m_Type;
			this->m_Size = rhs.m_Size;
			this->m_Value = rhs.m_Value;

			return *this;
		}

		virtual TagPtr Clone() const override {
			return new NbtTagBasic<T, TYPE>(*this);
		};

		// ���ڴ�������ݵ���ǩ�У����ض�ȡ���ַ�����
		virtual int Read(ByteBuffer* buffer) override {
			int readed = buffer->ReadData<T>(&m_Value);
			assert(readed == m_Size * sizeof(T));
			return readed;
		}

		// ��ȡ����ָ�룬������������ת����
		// ����	Int32 data = *(tag->GetValue());
		virtual void* Value() const override {
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

		friend std::wostream& operator<<(std::wostream& out, const NbtTagBasic& tag) {
			return tag.OutString(out);
		}

		// operator=������ʵ��
		virtual std::wostream& OutString(std::wostream& out) const override {
			out << "(" << this->TypeName() << ")";
			if (nullptr == this->Name()) {
				out << "NONAME";
			}
			else {
				out << "\"" << this->Name() << "\"";
			}
			out << " = ";
			OutValueString(out);
			out << std::endl;

			return out;
		}
		virtual std::wostream& OutValueString(std::wostream& out) const override {
			out << MineCraft::TypeConvert<T>::Instance()->toString(this->m_Value);
			return out;
		}
	};

	using ByteTag = NbtTagBasic<Byte8, NbtTagType::Byte>;
	using ShortTag = NbtTagBasic<Short16, NbtTagType::Short>;
	using IntTag = NbtTagBasic<Int32, NbtTagType::Int>;
	using LongTag = NbtTagBasic<Long64, NbtTagType::Long>;
	using FloatTag = NbtTagBasic<Float32, NbtTagType::Float>;
	using DoubleTag = NbtTagBasic<Double64, NbtTagType::Double>;

	template<typename T, NbtTagType TYPE> class  LIB_NBT_EXPORT NbtTagArray :public NbtTag {
	private:
		int m_Capacity{ 0 };

	protected:
		T * m_Values{ nullptr };

	protected:
		void AllocCapacity(int size) {
			this->ClearValues();
			m_Size = size;
			m_Capacity = ((m_Size + 15) >> 4) << 4;
			m_Values = new T[m_Capacity];
		}

		void Expand(UInt size) {
			int newCapcity = ((m_Size + 15 + size) >> 4) << 4;
			if (newCapcity <= m_Capacity) {
				return;
			}
			m_Capacity = newCapcity;

			T * oldEntries = m_Values;
			m_Values = new T[m_Capacity];
			memcpy(m_Values, oldEntries, m_Size * sizeof(T));
			delete[] oldEntries;
		}

		void Shrink() {
			int newCapcity = ((m_Size + 15) >> 4) << 4;
			if (newCapcity >= m_Capacity) {
				return;
			}
			m_Capacity = newCapcity;

			T * oldEntries = m_Values;
			m_Values = new T[m_Capacity];
			memcpy(m_Values, oldEntries, m_Size * sizeof(T));
			delete[] oldEntries;
		}

		virtual void ClearValues() override {
			if (nullptr != m_Values) {
				delete[] m_Values;
				m_Values = nullptr;
			}
			m_Size = 0;
			m_Capacity = 0;
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
		NbtTagArray(const NbtTagArray& rhs) { *this = rhs; }

		NbtTagArray& operator=(const NbtTagArray& rhs) {
			this->SetName(rhs.Name());
			this->m_Type = rhs.m_Type;
			this->SetValue(rhs.m_Values, rhs.m_Size);

			return *this;
		}

		virtual TagPtr Clone() const override {
			return new NbtTagArray<T, TYPE>(*this);
		};

		// ���ڴ�������ݵ���ǩ�У����ض�ȡ���ֽ�����
		virtual int Read(ByteBuffer* buffer) override {
			Int32 size = buffer->ReadInt();
			if (0 == size) {
				return size;
			}
			AllocCapacity(size);

			return buffer->ReadData<T>(m_Values, m_Size);
		}

		// ��ȡ����ָ�룬������������ת����
		// ����Int32* data = (Int32*)tag->GetValue();
		virtual void* Value() const override {
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
			if (0 == size) {
				return;
			}
			AllocCapacity(size);

			memcpy(m_Values, value, size * sizeof(T));
		};

		friend std::wostream& operator<<(std::wostream& out, const NbtTagArray& tag) {
			return tag.OutString(out);
		}

		// operator=������ʵ��
		virtual std::wostream& OutString(std::wostream& out) const override {
			out << "(" << this->TypeName() << ")";
			if (nullptr == this->Name()) {
				out << "NONAME";
			}
			else {
				out << "\"" << this->Name() << "\"";
			}
			out << ", " << this->Size() << " entries:" << std::endl;
			OutValueString(out);
			out << std::endl;

			return out;
		}

		virtual std::wostream& OutValueString(std::wostream& out) const override {
			for (int i = 0; i < this->Size(); i++) {
				out << MineCraft::TypeConvert<T>::Instance()->toString(this->m_Values[i]) << ",";
			}

			return out;
		}

		// ����һ����ǩ���������֡�����
		// ע����������顢�б����������ͣ�ֵӦΪָ��ĵ�ַ
		virtual T& Add(void* value) {
			if (m_Size + 1 > m_Capacity) {
				Expand(1);
			}
			T& data = *(T*)value;
			m_Values[m_Size++] = data;

			return data;
		}

	};
	using ByteArrayTag = NbtTagArray<Byte8, NbtTagType::ByteArray>;
	using IntArrayTag = NbtTagArray<Int32, NbtTagType::IntArray>;
	using LongArrayTag = NbtTagArray<Long64, NbtTagType::LongArray>;

	class  LIB_NBT_EXPORT StringTag :public NbtTag {
	private:
		wchar_t * m_Values{ nullptr };

	protected:
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
		StringTag(const StringTag& rhs) { *this = rhs; }

		StringTag& operator=(const StringTag& rhs) {
			this->SetName(rhs.Name());
			this->m_Type = NbtTagType::String;
			this->SetValue(rhs.m_Values, rhs.m_Size);

			return *this;
		}

		virtual TagPtr Clone() const override {
			return new StringTag(*this);
		};

		// ���ڴ�������ݵ���ǩ�С�
		virtual int Read(ByteBuffer* buffer) override {
			m_Size = buffer->ReadString(&m_Values, buffer->ReadShort());
			return m_Size;
		}

		// ��ȡ����ָ�룬������������ת����
		// ����	wchar_t* str = (wchar_t*)tag->GetValue();
		virtual void* Value() const override {
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
			wcscpy_s((wchar_t*)value, m_Size + 1, m_Values);
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
			wcscpy_s(m_Values, size + 1, (const wchar_t*)value);
		}

		friend std::wostream& operator<<(std::wostream& out, const StringTag& tag) {
			return tag.OutString(out);
		}

		// operator=������ʵ��
		virtual std::wostream& OutString(std::wostream& out) const override {
			out << "(" << this->TypeName() << ")";
			if (nullptr == this->Name()) {
				out << "NONAME";
			}
			else {
				out << "\"" << this->Name() << "\"";
			}
			out << " = \"";
			OutValueString(out);;
			out << "\"" << std::endl;

			return out;
		}
		virtual std::wostream& OutValueString(std::wostream& out) const override {
			if (nullptr != this->m_Values) {
				out << this->m_Values;
			}
			return out;
		}
	};

	using StringTagPtr = StringTag * ;

	class  LIB_NBT_EXPORT ListTag :public NbtTagArray<TagPtr, NbtTagType::List> {
	private:
		using super = NbtTagArray;
		NbtTagType m_TagId{ NbtTagType::End };

	protected:
		virtual void ClearValues() override {
			for (int i = 0; i < m_Size; i++) {
				if (nullptr != m_Values[i]) {
					delete m_Values[i];
				}
			}

			super::ClearValues();
		}

	public:
		ListTag() { }
		ListTag(const wchar_t* name) : super(name) { }
		ListTag(const std::wstring& name) : super(name) { }
		ListTag(const ListTag& rhs) { *this = rhs; }

		ListTag& operator=(const ListTag& rhs) {
			this->m_TagId = rhs.m_TagId;
			super::operator=(rhs);
			//this->SetName(rhs.Name());
			//this->m_Type = NbtTagType::List;
			//this->SetValue(rhs.m_Values, rhs.m_Size);

			return *this;
		}

		virtual TagPtr Clone() const override {
			return new ListTag(*this);
		};

		// ���ڴ�������ݵ���ǩ�С�
		virtual int Read(ByteBuffer* buffer) override {
			m_TagId = static_cast<NbtTagType>(buffer->ReadByte());
			int size = buffer->ReadInt();
			if (NbtTagType::End == m_TagId) {
				assert(0 == size);
				return m_Size;
			}
			if (0 == size) {
				assert(NbtTagType::End == m_TagId);
				return m_Size;
			}
			AllocCapacity(size);

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
		virtual void* Value() const override {
			return (void*)m_Values;
		};

		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		// ����	TagPtr* tags = new TagPtr[tag->GetSize()];
		//		int count = tag->GetValue((void*)tags);
		// ע��ָ�������е�������ԭ��ǩ���棬�����и��ơ�
		virtual int GetValue(void* value) const {
			return NbtTagArray::GetValue(value);
			//if (nullptr == m_Values) {
			//	return 0;
			//}
			//memcpy(value, m_Values, m_Size * sizeof(TagPtr));
			//return m_Size * sizeof(TagPtr);
		}

		// ���ñ�ǩ��ֵ��Ҫ��ָ֤��ָ����ڴ��������������ʹ��������һ�¡�
		// value��ָ�����ݵ�ָ��
		// size����ʹ��
		// ����	TagPtr tag = new NbtTag;
		//		tag->SetValue((void*)&tag, 1);
		// ����	TagPtr* tags = new NbtTag[10];
		//		tag->SetValue((void*)tags, 10);
		virtual void SetValue(void* value, int size = 1) override {
			if (0 == size) {
				return;
			}
			AllocCapacity(size);

			TagPtr* tags = (TagPtr*)value;
			for (int i = 0; i < size; i++) {
				m_Values[i] = tags[i]->Clone();
			}
		}

		friend std::wostream& operator<<(std::wostream& out, const ListTag& tag) {
			return tag.OutString(out);
		}

		// operator=������ʵ��
		virtual std::wostream& OutValueString(std::wostream& out) const override {
			for (int i = 0; i < this->Size(); i++) {
				out << "\t";
				this->m_Values[i]->OutValueString(out);
				out << " ";
			}

			return out;
		}

		TagPtr Get(int index) const {
			if (index < 0 || index >= m_Size) {
				throw "Overflow";
			}
			return m_Values[index];
		}

		template<typename T> T GetInternalValue(int index) const {
			if (index < 0 || index >= m_Size) {
				throw "Overflow";
			}
			return *(T*)m_Values[index]->Value();
		}
	};

	using ListTagPtr = ListTag * ;

	class LIB_NBT_EXPORT CompoundTag : public ListTag {// NbtTagArray<TagPtr, NbtTagType::Compound> {
	private:
		using super = ListTag;

	//protected:
	//	virtual void ClearValues() override {
	//		for (int i = 0; i < m_Size; i++) {
	//			if (nullptr != m_Values[i]) {
	//				delete m_Values[i];
	//			}
	//		}
	//		super::ClearValues();
	//	}

	public:
		CompoundTag() { this->m_Type = NbtTagType::Compound; };
		CompoundTag(const wchar_t* name) : super(name) { this->m_Type = NbtTagType::Compound; };
		CompoundTag(const std::wstring& name) : super(name) { this->m_Type = NbtTagType::Compound; };
		CompoundTag(const CompoundTag& rhs) { *this = rhs; }

		//CompoundTag& operator=(const CompoundTag& rhs) {
		//	this->SetName(rhs.Name());
		//	this->m_Type = NbtTagType::Compound;
		//	this->SetValue((void*)rhs.m_Values, rhs.m_Size);

		//	return *this;
		//}

		virtual TagPtr Clone() const override {
			return new CompoundTag(*this);
		};

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
		//virtual void SetValue(void* value, int size = 1) override {
		//	if (0 == size) {
		//		return;
		//	}
		//	AllocCapacity(size);

		//	TagPtr* entries = (TagPtr*)value;
		//	for (int i = 0; i < m_Size; i++) {
		//		m_Values[i] = entries[i]->Clone();
		//	}
		//}

		// ��ȡ����ָ�룬������������ת����
		// ���ص���TagPtr��Ҳ��NbtTag*�����͵�ָ�����顣
		// ����	TagPtr* tags = (TagPtr*)tag->GetValue();
		// ��������һ��Ԫ�أ�wchar_t* name = tags[1]->Name();
		//virtual void* Value() const override {
		//	return (void*)m_Values;
		//};

		// ��ȡ���ݣ����������������Ҫ�Ŀռ䣬���ض�ȡ���ַ�����
		// ����	TagPtr* tags = new TagPtr[tag->GetSize()];
		//		int count = tag->GetValue((void*)tags);
		// ע��ָ�������е�������ԭ��ǩ���棬�����и��ơ�
		//virtual int GetValue(void* value) const {
		//	return super::GetValue(value);
		//	//if (nullptr == m_Values) {
		//	//	return 0;
		//	//}
		//	//memcpy(value, m_Values, m_Size * sizeof(TagPtr));
		//	//return m_Size * sizeof(TagPtr);
		//}

		// �������ֲ��ұ�ǩ
		TagPtr FindByName(const wchar_t* name)const {
			if (nullptr == name) {
				return nullptr;
			}
			size_t findSize = wcslen(name);
			if (0 == findSize) {
				return nullptr;
			}
			for (size_t i = 0; i < m_Size; i++) {
				if (nullptr == m_Values[i]->Name()) {
					continue;
				}
				size_t thisSize = wcslen(m_Values[i]->Name());
				if (thisSize != findSize) {
					continue;
				}
				if (_wcsnicmp(m_Values[i]->Name(), name, m_Values[i]->Size()) == 0) {
					return m_Values[i];
				}
			}
			return nullptr;
		}

		//// ����һ����ǩ���������֡�����
		//// ע����������顢�б����������ͣ�ֵӦΪָ��ĵ�ַ
		//TagPtr Add(NbtTagType type, const wchar_t* name, void* value) {
		//	if (m_Size + 1 > m_Capcity) {
		//		Expand(1);
		//	}
		//	TagPtr tag = NbtTag::FromType(type, name);
		//	tag->SetValue(value);
		//	m_Values[m_Size++] = tag;

		//	return tag;
		//}

		friend std::wostream& operator<<(std::wostream& out, const CompoundTag& tag) {
			return tag.OutString(out);
		}

		// operator=������ʵ��
		virtual std::wostream& OutValueString(std::wostream& out) const override {
			for (int i = 0; i < this->Size(); i++) {
				out << "\t";
				this->m_Values[i]->OutString(out);
				out << " ";
			}

			return out;
		}
	};

	using CompoundTagPtr = CompoundTag * ;
}