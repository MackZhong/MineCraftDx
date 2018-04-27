#pragma once
#include "IComparable.h"
#include <string>

namespace MC{
	enum TagType
	{
		TAG_END = 0,
		TAG_BYTE = 1,
		TAG_SHORT = 2,
		TAG_INT = 3,
		TAG_LONG = 4,
		TAG_FLOAT = 5,
		TAG_DOUBLE = 6,
		TAG_BYTE_ARRAY = 7,
		TAG_STRING = 8,
		TAG_LIST = 9,
		TAG_COMPOUND = 10,
		TAG_INT_ARRAY = 11,
		TAG_LONG_ARRAY = 12,
		TAG_SHORT_ARRAY = 13,
		TAG_MAX
	};

class TagKey : public IComparable<TagKey>
{
protected:
	std::string m_Name;
	TagType m_Type;

public:
	TagKey(const char* name, TagType type) : m_Name(name), m_Type(type) {};
	std::string Name() const { return m_Name; }
	TagType Type() const { return m_Type; }

	// Í¨¹ý IComparable ¼Ì³Ð
	virtual bool CompareTo(const TagKey & other) override {
		if (this->m_Type != other.m_Type) {
			return false;
		}
		return this->m_Name == other.m_Name;
	};
};
}
