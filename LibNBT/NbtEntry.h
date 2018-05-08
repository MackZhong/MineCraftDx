#pragma once
#include "nbt.h"
#include <vector>
#include <string>

namespace MineCraft {
	class NbtEntry
	{
	protected:
		std::string m_Name;

	public:
		NbtEntry() {};
	};
}