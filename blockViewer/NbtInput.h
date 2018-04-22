#pragma once
#include <boost/iostreams/filtering_stream.hpp>
//#include <boost/iostreams/stream_buffer.hpp>
//#include <boost/iostreams/stream.hpp>
#include <streambuf>
#include <memory>
#include <fstream>

//using StreamBuffer = boost::iostreams::stream_buffer;
using IFilteringStream = boost::iostreams::filtering_istream;

class NbtInput
{
	std::shared_ptr<std::streambuf> m_Buffer;

public:
	NbtInput(const IFilteringStream& stream);
	NbtInput(const std::ifstream& stream);
	~NbtInput();
};


NbtInput::NbtInput(const IFilteringStream& stream) : m_Buffer(stream.rdbuf)
{
	//m_Buffer = std::shared_ptr<std::streambuf>(stream.rdbuf());
}

inline NbtInput::NbtInput(const std::ifstream & stream) : m_Buffer(stream.rdbuf)
{
	//m_Buffer = std::shared_ptr < std::streambuf>(stream.rdbuf);
}


NbtInput::~NbtInput()
{
}
