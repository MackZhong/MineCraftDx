#include "EnginePCH.h"
#include "mc.h"
#include "NbtReaderWriter.h"
//#include "NbtIo.h"
#include "NbtTag.h"

namespace MC {
	char DataConversionBuffer[_CVTBUFSIZE];

	const int MCREGION_VERSION_ID = 0x4abc;
	const int ANVIL_VERSION_ID = 0x4abd;
	const wchar_t* NETHER_FOLDER = L"DIM-1";
	const wchar_t* ENDER_FOLDER = L"DIM1";

	std::wostream& operator<<(std::wostream& ostm, const NbtTag* tag) {
		ostm << NbtTag::getTagName(tag->getId()) << ":" << tag->m_Name << L"=";

		switch (tag->getId()) {
		case TAG_End:
		{
			EndTag * ptag = (EndTag*)tag;
			ostm << *ptag;
			break;
		}
		case TAG_Byte:
		{
			ByteTag * ptag = (ByteTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Short:
		{
			ShortTag * ptag = (ShortTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Int:
		{
			IntTag * ptag = (IntTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Long:
		{
			LongTag * ptag = (LongTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Float:
		{
			FloatTag * ptag = (FloatTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Double:
		{
			DoubleTag * ptag = (DoubleTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Byte_Array:
		{
			ByteArrayTag * ptag = (ByteArrayTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_String:
		{
			StringTag * ptag = (StringTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_List:
		{
			ListTag * ptag = (ListTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Compound:
		{
			CompoundTag * ptag = (CompoundTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Int_Array:
		{
			IntArrayTag * ptag = (IntArrayTag*)(tag);
			ostm << *ptag;
			break;
		}
		case TAG_Long_Array: {
			LongArrayTag * ptag = (LongArrayTag*)(tag);
			ostm << *ptag;

			break;
		}
		}
		ostm << std::endl;
		ostm.flush();
		return ostm;
	}

	NbtTag* NbtTag::createTag(int type, const std::wstring& name) {
		switch (type) {
		case TAG_End:
			return new EndTag;
		case TAG_Byte:
			return new ByteTag(name);
		case TAG_Short:
			return new ShortTag(name);
		case TAG_Int:
			return new IntTag(name);
		case TAG_Long:
			return new LongTag(name);
		case TAG_Float:
			return new FloatTag(name);
		case TAG_Double:
			return new DoubleTag(name);
		case TAG_Byte_Array:
			return new ByteArrayTag(name);
		case TAG_String:
			return new StringTag(name);
		case TAG_List:
			return new ListTag(name);
		case TAG_Compound:
			return new CompoundTag(name);
		case TAG_Int_Array:
			return new IntArrayTag(name);
		case TAG_Long_Array:
			return new LongArrayTag(name);
		}
		return nullptr;
	}

	NbtTag* NbtTag::readNamedTag(NbtReader* pdis) {
		__int8 type = pdis->readByte();
		if (type < TAG_End || type >= TAG_Max) {
			throw "Invalid tag type";
			return nullptr;
		}

		if (TAG_End == type) {
			return new EndTag;
		}

		const std::wstring& name = pdis->readUTF();
		int pos = (int)pdis->position();
		NbtTag* newTag = createTag((TAG_TYPE)type, name);
		newTag->Load(pdis);

		return newTag;
	}

	void NbtTag::writeNamedTag(const NbtTag* tag, NbtWriter* pdos) {
		pdos->writeByte(tag->getId());
		if (TAG_End == tag->getId()) {
			return;
		}

		pdos->writeUTF(tag->getName());
		tag->Write(pdos);
	}

	const char* NbtTag::getTagName(__int8 type) {
		if (type < TAG_End || type >= TAG_Max) {
			throw "Invalid tag type";
			return nullptr;
		}

		switch ((TAG_TYPE)type) {
		case TAG_End:
			return "TAG_End";
		case TAG_Byte:
			return "TAG_Byte";
		case TAG_Short:
			return "TAG_Short";
		case TAG_Int:
			return "TAG_Int";
		case TAG_Long:
			return "TAG_Long";
		case TAG_Float:
			return "TAG_Float";
		case TAG_Double:
			return "TAG_Double";
		case TAG_Byte_Array:
			return "TAG_Byte_Array";
		case TAG_String:
			return "TAG_String";
		case TAG_List:
			return "TAG_List";
		case TAG_Compound:
			return "TAG_Compound";
		case TAG_Int_Array:
			return "TAG_Int_Array";
		case TAG_Long_Array:
			return "TAG_Long_Array";
		}
		return "UNKNOWN";
	}

	//CompoundTag* NbtIo::readCompressed(FS::ifstream& in) {
		//IFilteringStream sbin;
		//sbin.set_auto_close(true);
		//sbin.push(boost::iostreams::gzip_decompressor());
		//sbin.push(in);

		//NbtReader dis(sbin);
		//try {
		//	CompoundTag* tag = NbtIo::readFrom(&dis);
		//	return tag;
		//}
		//catch (std::exception& err) {
		//	std::cerr << "Error: " << err.what() << std::endl;
		//	throw err;
		//}
	//	return nullptr;
	//}

	//void NbtIo::writeCompressed(const CompoundTag* tag, FS::ofstream& out) {
		//boost::iostreams::filtering_ostream sbout;
		//sbout.push(out);

		//NbtWriter dos(sbout);
		//try {
		//	NbtIo::writeTo(tag, &dos);
		//}
		//catch (std::exception& err) {
		//	throw err;
		//}
	//}

	//CompoundTag* NbtIo::decompress(const char* buffer, size_t size, COMPRESSION_SCHEME scheme) {
		//IFilteringStream sbin;
		//if (COMPRESSION_SCHEME_GZIP == scheme)
		//	sbin.push(boost::iostreams::gzip_decompressor());
		//else
		//	sbin.push(boost::iostreams::zlib_decompressor());
		//boost::iostreams::array_source data(buffer, size);
		//sbin.push(data);
		//std::ofstream decomped("decomp.bin");
		//boost::iostreams::copy(sbin, decomped);
		//decomped.flush();
		//decomped.close();
		//std::ifstream ins("decomp.bin");
		//NbtReader dis(ins);

		//try {
		//	CompoundTag* tag = readFrom(&dis);
		//	return tag;
		//}
		//catch (std::exception& err) {
		//	throw err;
		//}

	//	return nullptr;
	//}

	//UniquePtr NbtIo::compress(CompoundTag* tag) {
	//	OFilteringStream sbout;
	//	NbtWriter dos(sbout);
	//	try {
	//		NbtIo::writeTo(tag, &dos);
	//	}
	//	catch (std::exception& err) {
	//		throw err;
	//	}

	//	size_t size = sbout.size();
	//	auto buf = std::make_unique<__int8[]>(size);
	//	sbout.rdbuf()->sgetn(buf.get(), size);

	//	return buf;
	//}

	//void NbtIo::write(const CompoundTag* tag, FS::path& file) {
	//	FS::ofstream ofs(file);
	//	NbtWriter dos(ofs);
	//	try {
	//		NbtIo::writeTo(tag, &dos);
	//	}
	//	catch (std::exception err) {

	//	}
	//	//finally{
	//	//	dos.close();
	//	//}
	//}

	//void NbtIo::safeWrite(const CompoundTag* tag, FS::path file) {
	//	FS::path file2(file.filename().wstring() + L"_tmp");
	//	if (FS::exists(file2)) {
	//		FS::remove(file2);
	//	}

	//	NbtIo::write(tag, file2);


	//	if (FS::exists(file)) {
	//		FS::remove(file);
	//	}
	//	if (FS::exists(file)) {
	//		wchar_t buf[128];
	//		swprintf_s(buf, L"Failed to delete %s ", file.filename().wstring().c_str());
	//		throw buf;
	//	}

	//	FS::rename(file2, file);
	//}

	//CompoundTag* NbtIo::readFrom(NbtReader* pdis) {
	//	NbtTag* tag1 = NbtTag::readNamedTag(pdis);
	//	CompoundTag* tag = reinterpret_cast<CompoundTag*>(tag1);

	//	if (tag)
	//		return tag;

	//	throw "Root tag must be a named compound tag";
	//}

	//CompoundTag* NbtIo::read(FS::path& file) {
	//	if (!FS::exists(file)) return nullptr;
	//	FS::ifstream ifs(file);
	//	NbtReader dis(ifs);
	//	try {
	//		return NbtIo::readFrom(&dis);
	//	}
	//	catch (std::exception err) {

	//	}
	//	//finally{
	//	//	dis.close();
	//	//}

	//	return nullptr;
	//}
}