#pragma once
#include "mc.h"
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

	extern char DataConversionBuffer[_CVTBUFSIZE];

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

		friend std::wostream& operator<<(std::wostream& wos, const NbtTag* tag);

	};

	class EndTag :public NbtTag {
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {};
		virtual void Load(NbtReader* pdis) override {};

	public:
		EndTag() :super(L"") {};
		virtual TAG_TYPE getId() const override { return TAG_End; };
		friend std::wostream& operator<<(std::wostream& wos, const EndTag& tag) {
			wos << "END";
			return wos;
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
		friend std::wostream& operator<<(std::wostream& wos, const ByteTag& tag) {
			_itoa_s(tag.m_Data, DataConversionBuffer, _CVTBUFSIZE, 10);
			wos << DataConversionBuffer;
			return wos;
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
		friend std::wostream& operator<<(std::wostream& wos, const ShortTag& tag) {
			_itoa_s(tag.m_Data, DataConversionBuffer, _CVTBUFSIZE, 10);
			wos << DataConversionBuffer;
			return wos;
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
		friend std::wostream& operator<<(std::wostream& wos, const IntTag& tag) {
			_itoa_s(tag.m_Data, DataConversionBuffer, _CVTBUFSIZE, 10);
			wos << DataConversionBuffer;
			return wos;
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
		friend std::wostream& operator<<(std::wostream& wos, const LongTag& tag) {
			_i64toa_s(tag.m_Data, DataConversionBuffer, _CVTBUFSIZE, 10);
			wos << DataConversionBuffer;
			return wos;
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
		friend std::wostream& operator<<(std::wostream& wos, const FloatTag& tag) {
			_gcvt_s(DataConversionBuffer, _CVTBUFSIZE, tag.m_Data, 64);
			wos << DataConversionBuffer;
			return wos;
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
		double getData()const { return m_Data; }
		DoubleTag(const std::wstring& name) : super(name), m_Data(0) {};
		DoubleTag(const std::wstring& name, double data) : super(name), m_Data(data) {};
		virtual TAG_TYPE getId() const override { return TAG_Double; };
		friend std::wostream& operator<<(std::wostream& wos, const DoubleTag& tag) {
			_gcvt_s(DataConversionBuffer, _CVTBUFSIZE, tag.m_Data, 64);
			wos << DataConversionBuffer;
			return wos;
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
		std::shared_ptr<const char> m_Data;
		__int32 m_Size;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeInt((__int32)m_Size);
			pdos->write(m_Data.get(), m_Size);
		};
		virtual void Load(NbtReader* pdis) override {
			m_Size = pdis->readInt();
			m_Data = pdis->read(m_Size);
		};

	public:
		ByteArrayTag(const std::wstring& name) : super(name) {
		};
		ByteArrayTag(const std::wstring& name, const char* data) : super(name), m_Data(data) {
		};
		virtual TAG_TYPE getId() const override { return TAG_Byte_Array; };
		friend std::wostream& operator<<(std::wostream& wos, const ByteArrayTag& tag) {
			wos << "[" << tag.m_Size << " bytes]";
			return wos;
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
		SharedPtr32 m_Data;
		__int32 m_Length;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeInt(m_Length);
			for (int i = 0; i < m_Length; i++) {
				pdos->writeInt(m_Data.get()[i]);
			}
		};
		virtual void Load(NbtReader* pdis) override {
			m_Length = pdis->readInt();
			m_Data = SharedPtr32(new __int32[m_Length]);
			for (int i = 0; i < m_Length; i++) {
				m_Data.get()[i] = pdis->readInt();
			}
		};

	public:
		IntArrayTag(const std::wstring& name) : super(name) {
		};
		IntArrayTag(const std::wstring& name, __int32* data, __int32 size)
			: super(name),
			m_Length(size),
			m_Data(data)
		{
		};
		virtual TAG_TYPE getId() const override { return TAG_Int_Array; };
		friend std::wostream& operator<<(std::wostream& wos, const IntArrayTag& tag) {
			wos << "[" << tag.m_Length << " integers]";
			return wos;
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
		SharedPtr64  m_Data;
		__int32 m_Length;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			pdos->writeInt(m_Length);
			for (int i = 0; i < m_Length; i++) {
				pdos->writeLong(m_Data.get()[i]);
			}
		};
		virtual void Load(NbtReader* pdis) override {
			m_Length = pdis->readInt();
			m_Data = SharedPtr64(new __int64[m_Length]);
			for (int i = 0; i < m_Length; i++) {
				m_Data.get()[i] = pdis->readInt();
			}
		};

	public:
		LongArrayTag(const std::wstring& name) : super(name) {
		};
		LongArrayTag(const std::wstring& name, _int64* data, __int32 size)
			: super(name)
			, m_Data(data)
			, m_Length(size) { };
		virtual TAG_TYPE getId() const override { return TAG_Long_Array; };
		friend std::wostream& operator<<(std::wostream& wos, const LongArrayTag& tag) {
			wos << "[" << tag.m_Length << " longlong]";
			return wos;
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
		friend std::wostream& operator<<(std::wostream& wos, const StringTag& tag) {
			wos << "\"" << tag.m_Data << "\"";
			return wos;
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

	class ListTag : public NbtTag {
		friend class CompoundTag;
		TagArray m_TagsList;
		TAG_TYPE m_ElemType;
		using super = NbtTag;

		// 通过 NbtTag 继承
		virtual void Write(NbtWriter* pdos) const override {
			__int8  type;
			if (m_TagsList.empty()) {
				type = TAG_Byte;
			}
			else {
				type = m_TagsList[0]->getId();
			}

			if (type < TAG_End || type >= TAG_Max) {
				throw "Invalid type";
			}
			pdos->writeByte(type);
			pdos->writeInt((__int32)m_TagsList.size());
			for (auto t = m_TagsList.begin(); t != m_TagsList.end(); t++) {
				(*t)->Write(pdos);
			}
		};
		virtual void Load(NbtReader* pdis) override {
			__int8 type = pdis->readByte();
			if (type < TAG_End || type >= TAG_Max) {
				throw "Invalid type";
			}
			m_ElemType = (TAG_TYPE)type;
			int size = pdis->readInt();
			//m_TagList.reserve(size);
			m_TagsList.clear();
			for (int i = 0; i < size; i++) {
				NbtTag* tag = NbtTag::createTag(m_ElemType, L"");
				tag->Load(pdis);
				m_TagsList.push_back(std::shared_ptr<NbtTag>(tag));
			}
		};

	public:
		ListTag(const std::wstring& name) : super(name) {
		};
		virtual TAG_TYPE getId() const override { return TAG_List; };
		friend std::wostream& operator<<(std::wostream& wos, const ListTag& tag) {
			wos << tag.m_TagsList.size() << " entries of type " << NbtTag::getTagName(tag.m_ElemType) << std::endl;
			for (int i = 0; i < tag.m_TagsList.size(); i++) {
				wos << tag.m_TagsList[i];
			}
			return wos;
		}
		void add(NbtTag* tag) {
			m_ElemType = tag->getId();
			m_TagsList.push_back(std::shared_ptr< NbtTag>(tag));
		}
		const NbtTagPtr get(int index) {
			return m_TagsList[index];
		}
		__int32 size() const {
			return (__int32)m_TagsList.size();
		}
		inline bool operator==(const ListTag& oth) const {
			if (!super::operator==(oth)) {
				return false;
			}
			return this->m_TagsList == oth.m_TagsList;
		}
		inline ListTag& operator=(const ListTag& oth) {
			this->setName(oth.getName());
			this->m_TagsList = oth.m_TagsList;
			return *this;
		}
	};

	class CompoundTag :public NbtTag {
		using CompoundTagMap = std::unordered_map<std::wstring, NbtTagPtr>;
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
		void Output(std::wostream& out) {
			out << m_Tags.size() << " tags:" << std::endl;
		}
		void Output(std::wostringstream& out) {
			out << m_Tags.size() << " tags:" << std::endl;
		}
		friend std::wostream& operator<<(std::wostream& wos, const CompoundTag& tag) {
			wos << "CompoundTag have " << tag.m_Tags.size() << " entries:" << std::endl;
			//for (int i = 0; i < tag.m_Tags.size(); i++)
			for (auto t = tag.m_Tags.begin(); t != tag.m_Tags.end(); t++) {
				wos << t->second;
			}
			return wos;
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
		void putByteArray(const std::wstring& name, char* value) {
			m_Tags.emplace(name, new ByteArrayTag(name, value));
		}
		void putIntArray(const std::wstring& name, __int32* value, __int32 size) {
			m_Tags.emplace(name, new IntArrayTag(name, value, size));
		}
		void putLongArray(const std::wstring& name, __int64* value, __int32 size) {
			m_Tags.emplace(name, new LongArrayTag(name, value, size));
		}
		void putCompound(const std::wstring& name, CompoundTag value) {
			m_Tags.emplace(name, value.setName(name));
		}
		void putBoolean(const std::wstring& name, bool val) {
			putByte(name, val ? (__int8)1 : 0);
		}
		//NbtTagPtr get(const std::wstring& name) const {
		//	return m_Tags.at(name);
		//}
		bool contains(const std::wstring& name) const {
			return m_Tags.find(name) != m_Tags.end();
		}
		__int8 getByte(const std::wstring& name) const {
			if (!contains(name)) return (__int8)0;
			ByteTag* tag = reinterpret_cast<ByteTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		__int16 getShort(const std::wstring& name) const {
			if (!contains(name)) return (__int16)0;
			ShortTag* tag = reinterpret_cast<ShortTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		__int32 getInt(const std::wstring& name)  const {
			if (!contains(name)) return (__int32)0;
			IntTag* tag = reinterpret_cast<IntTag *> (m_Tags.at(name).get());
			return tag->m_Data;
		}
		__int64 getLong(const std::wstring& name) const {
			if (!contains(name)) return (__int64)0;
			LongTag* tag = reinterpret_cast<LongTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		float getFloat(const std::wstring& name) const {
			if (!contains(name)) return (float)0;
			FloatTag* tag = reinterpret_cast<FloatTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		double getDouble(const std::wstring& name) const {
			if (!contains(name)) return (double)0;
			DoubleTag* tag = reinterpret_cast<DoubleTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		const std::wstring& getString(const std::wstring& name) const {
			if (!contains(name)) return L"";
			StringTag* tag = reinterpret_cast<StringTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		const std::shared_ptr<const char> getByteArray(const std::wstring& name) const {
			if (!contains(name)) return nullptr;
			ByteArrayTag* tag = reinterpret_cast<ByteArrayTag*>(m_Tags.at(name).get());
			return tag->m_Data;
		}
		SharedPtr32 getIntArray(const std::wstring& name, __int32& size) const {
			if (!contains(name)) return nullptr;
			IntArrayTag* tag = reinterpret_cast<IntArrayTag*>(m_Tags.at(name).get());
			size = tag->m_Length;
			return tag->m_Data;
		}
		SharedPtr64 getLongArray(const std::wstring& name, __int32& size) const {
			if (!contains(name)) return nullptr;
			LongArrayTag* tag = reinterpret_cast<LongArrayTag*>(m_Tags.at(name).get());
			size = tag->m_Length;
			return tag->m_Data;
		}
		CompoundTag* getCompound(const std::wstring& name) const {
			if (!contains(name)) return new CompoundTag(name);
			CompoundTag* tag = reinterpret_cast<CompoundTag*>(m_Tags.at(name).get());
			return tag;
		}
		ListTag* getList(const std::wstring& name)  const {
			if (!contains(name)) return new ListTag(name);
			ListTag* tag = reinterpret_cast<ListTag*>(m_Tags.at(name).get());
			return tag;
			//TagArray tags;
			//for (auto t = m_Tags.begin(); t != m_Tags.end(); t++) {
			//	if (t->second->getName() == name) {
			//		tags.push_back(t->second);
			//	}
			//}
			//return tags;
		}
		bool getBoolean(const std::wstring& string) const {
			return getByte(string) != 0;
		}
	};
	//using CompoundTagPtr = std::shared_ptr<CompoundTag>;
}