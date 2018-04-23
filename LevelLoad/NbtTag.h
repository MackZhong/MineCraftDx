#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>

#include "NbtReaderWriter.h"

namespace MC {
	enum TAG_TYPE : unsigned __int8 {
		TAG_End = 0,
		TAG_Byte = 1,
		TAG_Short = 2,
		TAG_Int = 3,
		TAG_Long = 4,
		TAG_Float = 5,
		TAG_Double = 6,
		TAG_Byte_Array = 7,
		TAG_String = 8,
		TAG_List = 9,
		TAG_Compound = 10,
		TAG_Int_Array = 11,
		TAG_Long_Array = 12,
		TAG_Max
	};

	using NbtTagPtr = std::shared_ptr<class NbtTag>;

	class NbtTag
	{
	private:
		std::wstring m_Name;

	public:
		NbtTag(const std::wstring& name) : m_Name(name) {};
		~NbtTag() {};

		virtual void Write(NbtWriter* pdos) const = 0;
		virtual void Load(NbtReader* pdis) = 0;

		static NbtTag* createTag(TAG_TYPE type, const std::wstring& name);
		virtual TAG_TYPE getId() const = 0;
		NbtTag* setName(const std::wstring& name) { this->m_Name = name;  return this; }
		const std::wstring& getName() const { return m_Name; }

		inline bool operator==(const NbtTag& oth) const {
			if (this->getId() != oth.getId())
				return false;
			return this->m_Name == oth.m_Name;
		}

		static NbtTag* readNamedTag(NbtReader* pdis);

		static void writeNamedTag(const NbtTag* tag, NbtWriter* dos);

		static const char* getTagName(__int8 type);
	};

	class EndTag :public NbtTag {
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {};
		virtual void Load(NbtReader* pdis) override {};

	public:
		EndTag() :super(L"") {};
		virtual TAG_TYPE getId() const override { return TAG_End; };
		friend std::ostream& operator<<(std::ostream& stm, const EndTag& tag) {
			stm << "END";
			return stm;
		}
	};

	class ByteTag :public NbtTag {
		friend class CompoundTag;
		__int8 m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override { pdos->writeByte(m_Data); };
		virtual void Load(NbtReader* pdis) override { m_Data = pdis->readByte(); };

	public:
		ByteTag(const std::wstring& name) : super(name), m_Data(0) {};
		ByteTag(const std::wstring& name, __int8 data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Byte; };
		friend std::ostream& operator<<(std::ostream& stm, const ByteTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const ByteTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}

			return this->m_Data == oth.m_Data;
		}
		inline ByteTag& operator=(const ByteTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class ShortTag :public NbtTag {
		friend class CompoundTag;
		__int16 m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override { pdos->writeShort(m_Data); };
		virtual void Load(NbtReader* pdis) override { m_Data = pdis->readShort(); };

	public:
		ShortTag(const std::wstring& name) : super(name), m_Data(0) {};
		ShortTag(const std::wstring& name, __int16 data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Short; };
		friend std::ostream& operator<<(std::ostream& stm, const ShortTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const ShortTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline ShortTag& operator=(const ShortTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class IntTag :public NbtTag {
		friend class CompoundTag;
		__int32 m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override { pdos->writeInt(m_Data); };
		virtual void Load(NbtReader* pdis) override { m_Data = pdis->readInt(); };

	public:
		IntTag(const std::wstring& name) : super(name), m_Data(0) {};
		IntTag(const std::wstring& name, __int32 data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Int; };
		friend std::ostream& operator<<(std::ostream& stm, const IntTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const IntTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline IntTag& operator=(const IntTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class LongTag :public NbtTag {
		friend class CompoundTag;
		__int64 m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override { pdos->writeLong(m_Data); };
		virtual void Load(NbtReader* pdis) override { m_Data = pdis->readLong(); };

	public:
		LongTag(const std::wstring& name) : super(name), m_Data(0) {};
		LongTag(const std::wstring& name, __int64 data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Long; };
		friend std::ostream& operator<<(std::ostream& stm, const LongTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const LongTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline LongTag& operator=(const LongTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class FloatTag :public NbtTag {
		friend class CompoundTag;
		float m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override { pdos->writeFloat(m_Data); };
		virtual void Load(NbtReader* pdis) override { m_Data = pdis->readFloat(); };

	public:
		FloatTag(const std::wstring& name) : super(name), m_Data(0) {};
		FloatTag(const std::wstring& name, float data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Float; };
		friend std::ostream& operator<<(std::ostream& stm, const FloatTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const FloatTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline FloatTag& operator=(const FloatTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class DoubleTag :public NbtTag {
		friend class CompoundTag;
		double m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override { pdos->writeDouble(m_Data); };
		virtual void Load(NbtReader* pdis) override { m_Data = pdis->readDouble(); };

	public:
		DoubleTag(const std::wstring& name) : super(name), m_Data(0) {};
		DoubleTag(const std::wstring& name, double data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Double; };
		friend std::ostream& operator<<(std::ostream& stm, const DoubleTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const DoubleTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline DoubleTag& operator=(const DoubleTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class ByteArrayTag :public NbtTag {
		friend class CompoundTag;
		ByteBuffer m_Data;
		__int32 m_Size;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeInt((__int32)m_Size);
			pdos->write(m_Data, m_Size);
		};
		virtual void Load(NbtReader* pdis) override {
			m_Size = pdis->readInt();
			m_Data = pdis->read(m_Size);
		};

	public:
		ByteArrayTag(const std::wstring& name) : super(name) {
		};
		ByteArrayTag(const std::wstring& name, const ByteBuffer& data) : super(name) {
			m_Data = data;
		};
		virtual TAG_TYPE getId() const override { return TAG_Byte_Array; };
		friend std::ostream& operator<<(std::ostream& stm, const ByteArrayTag& tag) {
			stm << "[" << tag.m_Size << " bytes]";
			return stm;
		}
		inline bool operator==(const ByteArrayTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline ByteArrayTag& operator=(const ByteArrayTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class IntArrayTag :public NbtTag {
		friend class CompoundTag;
		IntArray m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeInt((__int32)m_Data.size());
			for (int i = 0; i < (int)m_Data.size(); i++) {
				pdos->writeInt(m_Data[i]);
			}
		};
		virtual void Load(NbtReader* pdis) override {
			__int32 length = pdis->readInt();
			m_Data.reserve(length);
			for (int i = 0; i < (int)m_Data.size(); i++) {
				m_Data[i] = pdis->readInt();
			}
		};

	public:
		IntArrayTag(const std::wstring& name) : super(name) {
		};
		IntArrayTag(const std::wstring& name, const IntArray& data) : super(name) {
			m_Data = data;
		};
		virtual TAG_TYPE getId() const override { return TAG_Int_Array; };
		friend std::ostream& operator<<(std::ostream& stm, const IntArrayTag& tag) {
			stm << "[" << tag.m_Data.size() << " integers]";
			return stm;
		}
		inline bool operator==(const IntArrayTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline IntArrayTag& operator=(const IntArrayTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class LongArrayTag :public NbtTag {
		friend class CompoundTag;
		LongArray m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeInt((__int32)m_Data.size());
			for (int i = 0; i < (int)m_Data.size(); i++) {
				pdos->writeLong(m_Data[i]);
			}
		};
		virtual void Load(NbtReader* pdis) override {
			__int32 length = pdis->readInt();
			m_Data.reserve(length);
			for (int i = 0; i < (int)m_Data.size(); i++) {
				m_Data[i] = pdis->readInt();
			}
		};

	public:
		LongArrayTag(const std::wstring& name) : super(name) {
		};
		LongArrayTag(const std::wstring& name, const LongArray& data) : super(name) {
			m_Data = data;
		};
		virtual TAG_TYPE getId() const override { return TAG_Long_Array; };
		friend std::ostream& operator<<(std::ostream& stm, const LongArrayTag& tag) {
			stm << "[" << tag.m_Data.size() << " integers]";
			return stm;
		}
		inline bool operator==(const LongArrayTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline LongArrayTag& operator=(const LongArrayTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class StringTag :public NbtTag {
		friend class CompoundTag;
		std::wstring m_Data;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeUTF(m_Data);
		};
		virtual void Load(NbtReader* pdis) override {
			m_Data = pdis->readUTF();
		};

	public:
		StringTag(const std::wstring& name) : super(name) {
		};
		StringTag(const std::wstring& name, const std::wstring& data) : super(name) {
			m_Data = data;
		};
		virtual TAG_TYPE getId() const override { return TAG_String; };
		friend std::wostream& operator<<(std::wostream& stm, const StringTag& tag) {
			stm << tag.m_Data;
			return stm;
		}
		inline bool operator==(const StringTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline StringTag& operator=(const StringTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};

	class CompoundTag :public NbtTag {
		using CompoundTagMap = std::map<std::wstring, NbtTagPtr>;
		CompoundTagMap m_Tags;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			for (auto t = m_Tags.begin(); t != m_Tags.end(); t++) {
				NbtTag::writeNamedTag(t->second.get(), pdos);
			}
			pdos->writeByte(TAG_End);
		};
		virtual void Load(NbtReader* pdis) override {
			m_Tags.clear();
			NbtTag* tag;
			while ((tag = NbtTag::readNamedTag(pdis))->getId() != TAG_End) {
				m_Tags.emplace(tag->getName(), tag);
			}
		};

	public:
		CompoundTag(const std::wstring& name) : super(name) {
		};
		virtual TAG_TYPE getId() const override { return TAG_Compound; };
		bool isEmpty() const { return m_Tags.empty(); }
		friend std::ostream& operator<<(std::ostream& stm, const CompoundTag& tag) {
			stm << tag.m_Tags.size() << " entries";
			return stm;
		}
		inline bool operator==(const CompoundTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Tags == oth.m_Tags;
		}
		inline CompoundTag& operator=(const CompoundTag& oth) {
			this->setName(oth.getName());
			this->m_Tags = oth.m_Tags;
			return *this;
		}
		std::vector<NbtTagPtr> getAllTags() const {
			std::vector<NbtTagPtr> tags;
			for (auto t = m_Tags.begin(); t != m_Tags.end(); t++) {
				tags.push_back(t->second);
			}
			return tags;
		}
		void put(const std::wstring& name, NbtTag& tag) {
			m_Tags.emplace(name, tag.setName(name));
		}
		void putByte(const std::wstring& name, __int8 value) {
			m_Tags.emplace(name, new ByteTag(name, value));
		}
		void putShort(const std::wstring& name, __int16 value) {
			m_Tags.emplace(name, new ShortTag(name, value));
		}
		void putInt(const std::wstring& name, __int32 value) {
			m_Tags.emplace(name, new IntTag(name, value));
		}
		void putLong(const std::wstring& name, __int64 value) {
			m_Tags.emplace(name, new LongTag(name, value));
		}
		void putFloat(const std::wstring& name, float value) {
			m_Tags.emplace(name, new FloatTag(name, value));
		}
		void putDouble(const std::wstring& name, double value) {
			m_Tags.emplace(name, new DoubleTag(name, value));
		}
		void putString(const std::wstring& name, const std::wstring& value) {
			m_Tags.emplace(name, new StringTag(name, value));
		}
		void putByteArray(const std::wstring& name, const ByteBuffer& value) {
			m_Tags.emplace(name, new ByteArrayTag(name, value));
		}
		void putIntArray(const std::wstring& name, IntArray value) {
			m_Tags.emplace(name, new IntArrayTag(name, value));
		}
		void putCompound(const std::wstring& name, CompoundTag value) {
			m_Tags.emplace(name, value.setName(name));
		}
		void putBoolean(const std::wstring& name, bool val) {
			putByte(name, val ? (__int8)1 : 0);
		}
		NbtTagPtr get(const std::wstring& name) {
			return m_Tags.at(name);
		}
		bool contains(const std::wstring& name) {
			return m_Tags.find(name) != m_Tags.end();
		}
		__int8 getByte(const std::wstring& name) {
			if (!contains(name)) return (__int8)0;
			ByteTag* tag = reinterpret_cast<ByteTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		__int16 getShort(const std::wstring& name) {
			if (!contains(name)) return (__int16)0;
			ShortTag* tag = reinterpret_cast<ShortTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		__int32 getInt(const std::wstring& name) {
			if (!contains(name)) return (__int32)0;
			IntTag* tag = reinterpret_cast<IntTag *> (m_Tags.at(name).get());
			return tag->m_Data;
		}
		__int64 getLong(const std::wstring& name) {
			if (!contains(name)) return (__int64)0;
			LongTag* tag = reinterpret_cast<LongTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		float getFloat(const std::wstring& name) {
			if (!contains(name)) return (float)0;
			FloatTag* tag = reinterpret_cast<FloatTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		double getDouble(const std::wstring& name) {
			if (!contains(name)) return (double)0;
			DoubleTag* tag = reinterpret_cast<DoubleTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		const std::wstring& getString(const std::wstring& name) {
			if (!contains(name)) return L"";
			StringTag* tag = reinterpret_cast<StringTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		ByteBuffer getByteArray(const std::wstring& name) {
			if (!contains(name)) return nullptr;
			ByteArrayTag* tag = reinterpret_cast<ByteArrayTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		IntArray getIntArray(const std::wstring& name) {
			if (!contains(name)) return IntArray();
			IntArrayTag* tag = reinterpret_cast<IntArrayTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		CompoundTag* getCompound(const std::wstring& name) {
			if (!contains(name)) return new CompoundTag(name);
			CompoundTag* tag = reinterpret_cast<CompoundTag*>(m_Tags.at(name).get());
			return tag;
		}
		using TagList = std::vector<NbtTagPtr>;
		TagList getList(const std::wstring& name) {
			TagList tags;
			for (auto t = m_Tags.begin(); t != m_Tags.end(); t++) {
				if (t->second->getName() == name) {
					tags.push_back(t->second);
				}
			}
			return tags;
		}
		bool getBoolean(const std::wstring& string) {
			return getByte(string) != 0;
		}
	};
	//using CompoundTagPtr = std::shared_ptr<CompoundTag>;

	template<class T> class ListTag : public NbtTag {
		friend class CompoundTag;
		std::vector<T*> m_TagList;
		TAG_TYPE m_Type;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			if (m_TagList.empty()) {
				return;
			}
			__int8 t1 = m_TagList[0]->getId();
			if (t1 < TAG_End || t1 >= TAG_Max) {
				throw "Invalid type";
			}
			pdos->writeByte(t1);
			pdos->writeInt((__int32)m_TagList.size());
			for (auto t = m_TagList.begin(); t != m_TagList.end(); t++) {
				(*t)->Write(pdos);
			}
		};
		virtual void Load(NbtReader* pdis) override {
			__int8 t = pdis->readByte();
			if (t < TAG_End || t >= TAG_Max) {
				throw "Invalid type";
			}
			m_Type = (TAG_TYPE)t;
			int size = pdis->readInt();
			//m_TagList.reserve(size);
			m_TagList.clear();
			for (int i = 0; i < size; i++) {
				T* tag = NbtTag::createTag(m_Type, L"");
				tag->Load(pdis);
				m_TagList.push_back(tag);
			}
		};

	public:
		ListTag(const std::wstring& name) : super(name) {
		};
		virtual TAG_TYPE getId() const override { return TAG_List; };
		friend std::ostream& operator<<(std::ostream& stm, const ListTag& tag) {
			stm << tag.m_TagsList.size() << " entries of type " << NbtTag::getTagName(tag.m_Type);
			return stm;
		}
		void add(const T* tag) {
			m_Type = tag->getId();
			m_TagList.push_back(tag);
		}
		const T* get(int index) {
			return m_TagList[index];
		}
		__int32 size() const {
			return (__int32)m_TagList.size();
		}
		inline bool operator==(const ListTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_Data == oth.m_Data;
		}
		inline ListTag& operator=(const ListTag& oth) {
			this->setName(oth.getName());
			this->m_Data = oth.m_Data;
			return *this;
		}
	};
}