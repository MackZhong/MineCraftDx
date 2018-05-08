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

		// 读取标签名字
		const wchar_t* Name() const { return m_Name; };
		// 读取标签类型
		const NbtTagType& Type() const { return m_Type; }
		// 读取标签类型字符串
		const char* TypeName() const { return MineCraft::TypeName(m_Type); };
		// 获取数据指针，根据数据类型转换
		virtual void* GetValue() const = 0;
		// 读取数据，传入参数需分配好需要的空间，返回读取的字符数。
		virtual int GetValue(void* value) const = 0;
		// 设置标签的值，要保证指针指向的内存所含数据数量和传入的数量一致。
		// value：指向数据的指针
		// size：数据数量（非字节数）
		virtual void SetValue(void* value, int size = 1) = 0;
		// 返回存储的数据个数，简单类型为1。
		int GetSize() const { return m_Size; }

		// 从内存读入数据到标签中，返回读取的字节数。
		virtual int Read(ByteBuffer* buffer) = 0;

		// 根据类型创建带名字的NBT标签
		static TagPtr FromType(NbtTagType type, const wchar_t* name = nullptr);
		//// 根据类型创建不带名字的NBT标签数组
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

		// 从内存读入数据到标签中，返回读取的字符数。
		virtual int Read(ByteBuffer* buffer) override {
			int readed = buffer->ReadData<T>(&m_Value);
			assert(readed == m_Size * sizeof(T));
			return readed;
		}

		// 获取数据指针，根据数据类型转换。
		// 例：	Int32 data = *(tag->GetValue());
		virtual void* GetValue() const override {
			return (void*)&m_Value;
		};

		// 读取数据，传入参数为数据的地址，返回读取的字符数。
		// 例：	Int32 data;
		//		int size = tag->GetValue(&data);
		virtual int GetValue(void* value) const override {
			memcpy(value, &m_Value, sizeof(T));
			return m_Size * sizeof(T);
		};

		// 设置标签的值，要保证指针指向的内存所含数据数量和传入的数量一致。
		// value：指向数据的指针
		// size：不使用
		// 例：	Int32 data;
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

		// 从内存读入数据到标签中，返回读取的字节数。
		virtual int Read(ByteBuffer* buffer) override {
			ClearValues();
			m_Size = buffer->ReadInt();
			if (0 == m_Size) {
				return m_Size;
			}
			m_Values = new T[m_Size];

			return buffer->ReadData<T>(m_Values, m_Size);
		}

		// 获取数据指针，根据数据类型转换。
		// 例：Int32* data = (Int32*)tag->GetValue();
		virtual void* GetValue() const {
			return (void*)m_Values;
		};

		// 读取数据，传入参数需分配好需要的空间，返回读取的字符数。
		// 例：	Int32* data = new Int32[tag->GetSize()];
		//		int size = tag->GetValue((void*)data);
		virtual int GetValue(void* value) const {
			if (nullptr == m_Values) {
				return 0;
			}
			memcpy(value, m_Values, m_Size * sizeof(T));
			return m_Size * sizeof(T);
		}

		// 设置标签的值，要保证指针指向的内存所含数据数量和传入的数量一致。
		// value：指向数据的指针
		// size：数据的数量
		// 例：	Int32* data = new Int32[6];
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

		// 从内存读入数据到标签中。
		virtual int Read(ByteBuffer* buffer) override {
			m_Size = buffer->ReadString(&m_Values, buffer->ReadShort());
			return m_Size;
		}

		// 获取数据指针，根据数据类型转换。
		// 例：	wchar_t* str = (wchar_t*)tag->GetValue();
		virtual void* GetValue() const {
			return (void*)m_Values;
		};

		// 读取数据，传入参数需分配好需要的空间，返回读取的字符数。
		// 注意NbtTag::GetSize()返回的是字符数，分配空间要加字符串结尾符。
		// 例：	wchar_t* str = new wchar_t[tag->GetSize() + 1];
		//		int length = tag->GetValue((void*)str);
		virtual int GetValue(void* value) const {
			if (nullptr == m_Values) {
				return 0;
			}
			wcscpy_s((wchar_t*)value, m_Size, m_Values);
			return m_Size;
		}

		// 设置标签的值，要保证指针指向的内存所含数据数量和传入的数量一致。
		// value：指向字符串的指针
		// size：字符数
		// 例：	wchar_t* str = L"Test";
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

		// 设置标签的值，要保证指针指向的内存所含数据数量和传入的数量一致。
		// value：指向数据的指针
		// size：不使用
		// 例：	Int32* data = new Int32[6];
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

		// 从内存读入数据到标签中。
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

		// 获取数据指针，根据数据类型转换。
		// 返回的是TagPtr（也即NbtTag*）类型的指针数组。
		// 例：	TagPtr* tags = (TagPtr*)tag->GetValue();
		// 访问其中一个元素：wchar_t* name = tags[1]->Name();
		virtual void* GetValue() const {
			return (void*)m_Values;
		};

		// 读取数据，传入参数需分配好需要的空间，返回读取的字符数。
		// 例：	TagPtr* tags = new TagPtr[tag->GetSize()];
		//		int count = tag->GetValue((void*)tags);
		// 注意指针数组中的数据由原标签保存，不进行复制。
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

		// 从内存读入数据到标签中。
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

		// 设置标签的值，要保证指针指向的内存所含数据数量和传入的数量一致。
		// value：指向数据的指针
		// size：不使用
		// 例：	Int32* data = new Int32[6];
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

		// 获取数据指针，根据数据类型转换。
		// 返回的是TagPtr（也即NbtTag*）类型的指针数组。
		// 例：	TagPtr* tags = (TagPtr*)tag->GetValue();
		// 访问其中一个元素：wchar_t* name = tags[1]->Name();
		virtual void* GetValue() const {
			return (void*)m_Entries;
		};

		// 读取数据，传入参数需分配好需要的空间，返回读取的字符数。
		// 例：	TagPtr* tags = new TagPtr[tag->GetSize()];
		//		int count = tag->GetValue((void*)tags);
		// 注意指针数组中的数据由原标签保存，不进行复制。
		virtual int GetValue(void* value) const {
			if (nullptr == m_Entries) {
				return 0;
			}
			memcpy(value, m_Entries, m_Size * sizeof(TagPtr));
			return m_Size * sizeof(TagPtr);
		}

		// 根据名字查找标签
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

		// 插入一个标签并设置名字、数据
		NbtTag* Add(NbtTagType type, const wchar_t* name, void* value) {
			NbtTag* tag = NbtTag::FromType(type, name);
			tag->SetValue(value);

			return tag;
		}
	};

}