///////////////////////////////////////////
//do not remove this notice
//(c) Luta Vlad
// 
// safeSave 1.0.3
// 
// 
// logs:
// safeSave 1.0.2
// added the temp file thing and also small updates like license
// 
// safeSave 1.0.3
// added the uint64 and int64 types to key value pair
// 
// safeSave 1.0.4
// added the ability to easily store a key value pare inside itself, easy nesting data!
//  + initializer list constructor!
// new data types like vector types!
///////////////////////////////////////////

#include <safeSave/safeSave.h>
#include <filesystem>
#include <string>

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__
#define NOMINMAX 
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#elif defined __linux__
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#endif


namespace sfs
{

	constexpr const char *errorsString[] =
	{
		"noError",
		"couldNotOpenFinle",
		"fileSizeDitNotMatch",
		"checkSumFailed",
		"couldNotMakeBackup",
		"readBackup",
		"warningEntryAlreadyExists",
		"entryNotFound",
		"entryHasDifferentDataType",
		"couldNotParseData",
		"fileSizeNotBigEnough",
	};


	const char *getErrorString(Errors e)
	{
		if (e >= 0 && e < sizeof(errorsString) / sizeof(errorsString[0]))
		{
			return errorsString[e];
		}
		else
		{
			return "unknown error";
		}
	}

	Errors readEntireFile(std::vector<char> &data, const char *name)
	{
		data.clear();
		std::ifstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t size = f.tellg();
			f.seekg(0, std::ios_base::beg);

			data.resize(size);

			f.read(&data[0], size);

			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors readEntireFile(void *data, size_t size, const char *name, bool shouldMatchSize, int *bytesRead)
	{
		if (bytesRead)
		{
			*bytesRead = 0;
		}

		std::ifstream f(name, std::ios::binary);
		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.seekg(0, std::ios_base::beg);

			if (shouldMatchSize)
			{
				if (readSize != size)
				{
					return fileSizeDitNotMatch;
				}
				else
				{
					f.read((char *)data, readSize);
					if (bytesRead)
					{
						*bytesRead = readSize;
					}

					return noError;
				}
			}
			else
			{
				f.read((char *)data, std::min(size, readSize));

				if (bytesRead)
				{
					*bytesRead = std::min(size, readSize);
				}

				return noError;
			}
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors getFileSize(const char *name, size_t &size)
	{
		size = 0;

		std::ifstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.close();
			size = readSize;

			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	using HashType = unsigned long long;

	//https://stackoverflow.com/questions/34595/what-is-a-good-hash-function
	unsigned long long fnv_hash_1a_64(const void *key, int len)
	{
		const unsigned char *p = (const unsigned char *)key;
		unsigned long long h = 0xcbf29ce484222325ULL;

		//for (int i = 0; i < len; i++)
		//{
		//	h = (h ^ p[i]) * 0x100000001b3ULL;
		//}

		if (len >= 4)
			for (int i = 0; i < len - 3; i += 4)
			{
				h = (h ^ p[i + 0]) * 0x100000001b3ULL;
				h = (h ^ p[i + 1]) * 0x100000001b3ULL;
				h = (h ^ p[i + 2]) * 0x100000001b3ULL;
				h = (h ^ p[i + 3]) * 0x100000001b3ULL;
			}

		for (int i = len - (len % 4); i < len; i++)
		{
			h = (h ^ p[i]) * 0x100000001b3ULL;
		}

		return h;
	}

	Errors readEntireFileWithCheckSum(void *data, size_t size, const char *name)
	{
		std::ifstream f(name, std::ios::binary);
		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.seekg(0, std::ios_base::beg);

			size_t sizeWithCheckSum = size + sizeof(HashType);

			if (readSize != sizeWithCheckSum)
			{
				return fileSizeDitNotMatch;
			}
			else
			{
				f.read((char *)data, size);
				HashType checkSum = 0;
				f.read((char *)&checkSum, sizeof(HashType));

				auto testCheck = fnv_hash_1a_64(data, size);

				if (testCheck != checkSum)
				{
					return checkSumFailed;
				}
				else
				{
					return noError;
				}
			}

		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors readEntireFileWithCheckSum(std::vector<char> &data, const char *name)
	{
		data.clear();

		std::ifstream f(name, std::ios::binary);
		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.seekg(0, std::ios_base::beg);

			if (readSize > sizeof(HashType))
			{
				data.resize(readSize - sizeof(HashType));
				f.read(&data[0], readSize - sizeof(HashType));

				HashType checkSum = 0;
				f.read((char *)&checkSum, sizeof(HashType));

				auto testCheck = fnv_hash_1a_64(&data[0], data.size());

				if (testCheck != checkSum)
				{
					return checkSumFailed;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return fileSizeNotBigEnough;
			}

		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors writeEntireFileWithCheckSum(const void *data, size_t size, const char *name)
	{
		std::ofstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.write((char *)data, size);
			auto testCheck = fnv_hash_1a_64(data, size);

			f.write((char *)&testCheck, sizeof(testCheck));

			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors writeEntireFile(const std::vector<char> &data, const char *name)
	{
		return writeEntireFile(data.data(), data.size(), name);
	}

	Errors writeEntireFile(const void *data, size_t size, const char *name)
	{
		std::ofstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.write((char *)data, size);
			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors safeSave(const void *data, size_t size, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError)
	{
		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string file2 = nameWithoutExtension; file2 += "2.bin";
		std::string tmpFile = std::string(nameWithoutExtension) + "1.tmp";

		auto err = writeEntireFileWithCheckSum((char *)data, size, tmpFile.c_str());

		if (err == couldNotOpenFinle)
		{
			return couldNotOpenFinle;
		}
		else
		{
			std::error_code renameError;
			std::filesystem::rename(tmpFile, file1, renameError);
			if (renameError)
				return couldNotOpenFinle;

			std::error_code errorCode;
			std::filesystem::copy_file(file1, file2,
				std::filesystem::copy_options::overwrite_existing, errorCode);
			//auto err2 = writeEntireFileWithCheckSum((char*)data, size, file2.c_str());	

			if (errorCode.value())
			{
				if (reportnotMakingBackupAsAnError)
				{
					return couldNotMakeBackup;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return noError;
			}
		}
	}

	Errors safeSaveNoBackup(const void *data, size_t size, const char *nameWithoutExtension)
	{
		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string tmpFile = std::string(nameWithoutExtension) + "1.tmp";

		auto err = writeEntireFileWithCheckSum((char *)data, size, tmpFile.c_str());

		if (err == couldNotOpenFinle)
		{
			return couldNotOpenFinle;
		}
		else
		{
			std::error_code renameError;
			std::filesystem::rename(tmpFile, file1, renameError);
			if (renameError)
				return couldNotOpenFinle;

			return noError;
		}

	}

	Errors safeLoad(void *data, size_t size, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError)
	{
		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		auto err = readEntireFileWithCheckSum((char *)data, size, file1.c_str());

		if (err == noError)
		{
			return noError;
		}
		else
		{
			//load backup
			auto err2 = readEntireFileWithCheckSum((char *)data, size, file2.c_str());

			if (err2 == noError)
			{
				if (reportLoadingBackupAsAnError)
				{
					return readBackup;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return err2;
			}
		}
	}

	Errors safeLoad(std::vector<char> &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError)
	{
		data.clear();

		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		auto err = readEntireFileWithCheckSum(data, file1.c_str());

		if (err == noError)
		{
			return noError;
		}
		else
		{
			//load backup
			auto err2 = readEntireFileWithCheckSum(data, file2.c_str());

			if (err2 == noError)
			{
				if (reportLoadingBackupAsAnError)
				{
					return readBackup;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return err2;
			}
		}
	}

	Errors safeLoadBackup(void *data, size_t size, const char *nameWithoutExtension)
	{
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		//load backup
		auto err2 = readEntireFileWithCheckSum((char *)data, size, file2.c_str());
		return err2;
	}

	Errors safeSave(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError)
	{
		auto rez = data.formatIntoFileDataBinary();

		return safeSave(rez.data(), rez.size(), nameWithoutExtension, reportnotMakingBackupAsAnError);
	}

	Errors safeSaveNoBackup(SafeSafeKeyValueData &data, const char *nameWithoutExtension)
	{
		auto rez = data.formatIntoFileDataBinary();

		return safeSaveNoBackup(rez.data(), rez.size(), nameWithoutExtension);
	}

	Errors safeLoad(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError)
	{
		data = {};

		std::vector<char> readData;
		auto errCode = safeLoad(readData, nameWithoutExtension, reportLoadingBackupAsAnError);

		if (errCode == noError || errCode == readBackup)
		{
			return data.loadFromFileData(readData.data(), readData.size());
		}
		else
		{
			return errCode;
		}
	}


#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__

	Errors openFileMapping(FileMapping &fileMapping, const char *name, size_t size, bool createIfNotExisting)
	{
		fileMapping = {};

		DWORD createDisposition = 0;

		if (createIfNotExisting)
		{
			createDisposition = OPEN_ALWAYS;
		}
		else
		{
			createDisposition = OPEN_EXISTING;
		}

		fileMapping.internal.fileHandle = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, createDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileMapping.internal.fileHandle == INVALID_HANDLE_VALUE)
		{
			auto err = GetLastError();
			return Errors::couldNotOpenFinle;
		}

		fileMapping.internal.fileMapping = CreateFileMappingA(fileMapping.internal.fileHandle, NULL, PAGE_READWRITE, 0, size, NULL);

		if (fileMapping.internal.fileMapping == NULL)
		{
			CloseHandle(fileMapping.internal.fileHandle);
			return Errors::couldNotOpenFinle;
		}


		fileMapping.pointer = MapViewOfFile(fileMapping.internal.fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);

		if (fileMapping.pointer == nullptr)
		{
			CloseHandle(fileMapping.internal.fileMapping);
			CloseHandle(fileMapping.internal.fileHandle);
			return Errors::couldNotOpenFinle;
		}

		fileMapping.size = size;


		return Errors::noError;
	}

	void closeFileMapping(FileMapping &fileMapping)
	{
		UnmapViewOfFile(fileMapping.pointer);
		CloseHandle(fileMapping.internal.fileMapping);
		CloseHandle(fileMapping.internal.fileHandle);
		fileMapping = {};
	}

#elif defined __linux__

	Errors openFileMapping(FileMapping &fileMapping, const char *name, size_t size, bool createIfNotExisting)
	{
		int createDisposition = 0;
		if (createIfNotExisting)
		{
			createDisposition = O_CREAT;
		}

		fileMapping.internal.fd = open(name, O_RDWR | createDisposition);

		if (fileMapping.internal.fd == -1)
		{
			return Errors::couldNotOpenFinle;
		}

		if (ftruncate(fileMapping.internal.fd, size) == -1)
		{
			close(fileMapping.internal.fd);
			return Errors::couldNotOpenFinle;
		}

		fileMapping.pointer = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fileMapping.internal.fd, 0);

		if (fileMapping.pointer == MAP_FAILED)
		{
			fileMapping.pointer = 0;
			close(fileMapping.internal.fd);
			return Errors::couldNotOpenFinle;
		}

		fileMapping.size = size;


		return Errors::noError;
	}

	void closeFileMapping(FileMapping &fileMapping)
	{
		fsync(fileMapping.internal.fd);
		msync(fileMapping.pointer, fileMapping.size, MS_SYNC);
		munmap(fileMapping.pointer, fileMapping.size);
		close(fileMapping.internal.fd);

		fileMapping = {};
	}

#endif	

	bool SafeSafeKeyValueData::operator==(const SafeSafeKeyValueData &other) const
	{

		if (entries.size() != other.entries.size()) { return false; }

		for (const auto &e : entries)
		{

			const auto found = other.entries.find(e.first);
			if (found == other.entries.end()) { return false; }

			if (e.second != found->second) { return false; }

		}

		return true;
	}

	bool SafeSafeKeyValueData::entryExists(std::string at)
	{
		return entries.find(at) != entries.end();
	}

	Errors SafeSafeKeyValueData::getEntryType(std::string at, char &type)
	{
		type = 0;

		auto it = entries.find(at);

		if (it == entries.end()) { return Errors::entryNotFound; }

		type = it->second.type;

		return Errors::noError;
	}

	Errors SafeSafeKeyValueData::setRawData(std::string at, void *data, size_t size)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::rawData_type;
			it->second.data.clear();
			it->second.data.resize(size);
			memcpy(it->second.data.data(), data, size);

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};
			e.type = Entry::Types::rawData_type;
			e.data.resize(size);
			memcpy(e.data.data(), data, size);

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setInt(std::string at, int32_t i)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::int_type;
			it->second.data.clear();
			it->second.primitives.intData = i;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::int_type;
			e.primitives.intData = i;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setChar(std::string at, char c)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::char_type;
			it->second.data.clear();
			it->second.primitives.charData = c;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::char_type;
			e.primitives.charData = c;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setUCHar(std::string at, unsigned char c)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::uchar_type;
			it->second.data.clear();
			it->second.primitives.uCharData = c;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::uchar_type;
			e.primitives.uCharData = c;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setUInt(std::string at, uint32_t u)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::uint_type;
			it->second.data.clear();
			it->second.primitives.uintData = u;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::uint_type;
			e.primitives.uintData = u;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setDouble(std::string at, double d)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::double_type;
			it->second.data.clear();
			it->second.primitives.doubleData = d;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::double_type;
			e.primitives.doubleData = d;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setVec2(std::string at, float x, float y)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::vec2_type;
			it->second.data.clear();
			it->second.primitives.vec.x = x;
			it->second.primitives.vec.y = y;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::vec2_type;
			e.primitives.vec.x = x;
			e.primitives.vec.y = y;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setVec3(std::string at, float x, float y, float z)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::vec3_type;
			it->second.data.clear();
			it->second.primitives.vec.x = x;
			it->second.primitives.vec.y = y;
			it->second.primitives.vec.z = z;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::vec3_type;
			e.primitives.vec.x = x;
			e.primitives.vec.y = y;
			e.primitives.vec.z = z;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setVec4(std::string at, float x, float y, float z, float w)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::vec4_type;
			it->second.data.clear();
			it->second.primitives.vec.x = x;
			it->second.primitives.vec.y = y;
			it->second.primitives.vec.z = z;
			it->second.primitives.vec.w = w;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::vec4_type;
			e.primitives.vec.x = x;
			e.primitives.vec.y = y;
			e.primitives.vec.z = z;
			e.primitives.vec.w = w;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setIVec2(std::string at, int x, int y)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::ivec2_type;
			it->second.data.clear();
			it->second.primitives.ivec.x = x;
			it->second.primitives.ivec.y = y;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::ivec2_type;
			e.primitives.ivec.x = x;
			e.primitives.ivec.y = y;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setIVec3(std::string at, int x, int y, int z)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::ivec3_type;
			it->second.data.clear();
			it->second.primitives.ivec.x = x;
			it->second.primitives.ivec.y = y;
			it->second.primitives.ivec.z = z;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::ivec3_type;
			e.primitives.ivec.x = x;
			e.primitives.ivec.y = y;
			e.primitives.ivec.z = z;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setIVec4(std::string at, int x, int y, int z, int w)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::ivec4_type;
			it->second.data.clear();
			it->second.primitives.ivec.x = x;
			it->second.primitives.ivec.y = y;
			it->second.primitives.ivec.z = z;
			it->second.primitives.ivec.w = w;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::ivec4_type;
			e.primitives.ivec.x = x;
			e.primitives.ivec.y = y;
			e.primitives.ivec.z = z;
			e.primitives.ivec.w = w;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::getVec2(std::string at, float &x, float &y)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::vec2_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				x = it->second.primitives.vec.x;
				y = it->second.primitives.vec.y;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getVec3(std::string at, float &x, float &y, float &z)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::vec3_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				x = it->second.primitives.vec.x;
				y = it->second.primitives.vec.y;
				z = it->second.primitives.vec.z;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getVec4(std::string at, float &x, float &y, float &z, float &w)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::vec4_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				x = it->second.primitives.vec.x;
				y = it->second.primitives.vec.y;
				z = it->second.primitives.vec.z;
				w = it->second.primitives.vec.w;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getIVec2(std::string at, int &x, int &y)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::ivec2_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				x = it->second.primitives.ivec.x;
				y = it->second.primitives.ivec.y;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getIVec3(std::string at, int &x, int &y, int &z)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::ivec3_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				x = it->second.primitives.ivec.x;
				y = it->second.primitives.ivec.y;
				z = it->second.primitives.ivec.z;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getIVec4(std::string at, int &x, int &y, int &z, int &w)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::ivec4_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				x = it->second.primitives.ivec.x;
				y = it->second.primitives.ivec.y;
				z = it->second.primitives.ivec.z;
				w = it->second.primitives.ivec.w;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setuInt64(std::string at, uint64_t i)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::uint64_type;
			it->second.data.clear();
			it->second.primitives.uint64Data = i;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::uint64_type;
			e.primitives.uint64Data = i;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::getuInt64(std::string at, uint64_t &i)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::uint64_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				i = it->second.primitives.uint64Data;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setInt64(std::string at, int64_t i)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::int64_type;
			it->second.data.clear();
			it->second.primitives.int64Data = i;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::int64_type;
			e.primitives.int64Data = i;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::getInt64(std::string at, int64_t &i)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::int64_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				i = it->second.primitives.int64Data;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setKeyValueData(std::string at, const SafeSafeKeyValueData &data)
	{

		auto binaryData = data.formatIntoFileDataBinary();

		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::keyValueData_type;
			it->second.data.clear();
			it->second.data.resize(binaryData.size());

			if (binaryData.size())
				memcpy(it->second.data.data(), binaryData.data(), binaryData.size());

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};
			e.type = Entry::Types::keyValueData_type;
			e.data.resize(binaryData.size());

			if (binaryData.size())
				memcpy(e.data.data(), binaryData.data(), binaryData.size());

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::getKeyValueData(std::string at, SafeSafeKeyValueData &data)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::keyValueData_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				size_t size = it->second.data.size();
				void *binaryData = it->second.data.data();

				data = {};
				auto readingData = data.loadFromFileData((char *)binaryData, size);

				if (readingData != Errors::noError)
				{
					return readingData;
				}

				return Errors::noError;
			}

		}

	}

	Errors SafeSafeKeyValueData::getRawDataPointer(std::string at, void *&data, size_t &size)
	{
		data = 0;
		size = 0;

		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::rawData_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				size = it->second.data.size();
				data = it->second.data.data();
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getInt(std::string at, int32_t &i)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::int_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				i = it->second.primitives.intData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setFloat(std::string at, float f)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::float_type;
			it->second.data.clear();
			it->second.primitives.floatData = f;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::float_type;
			e.primitives.floatData = f;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}


	Errors SafeSafeKeyValueData::getFloat(std::string at, float &f)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::float_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				f = it->second.primitives.floatData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getChar(std::string at, char &c)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::char_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				c = it->second.primitives.charData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getUChar(std::string at, unsigned char &c)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::uchar_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				c = it->second.primitives.uCharData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getUInt(std::string at, unsigned int &u)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::uint_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				u = it->second.primitives.uintData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getDobule(std::string at, double &d)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::double_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				d = it->second.primitives.doubleData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getBool(std::string at, bool &b)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::bool_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				b = it->second.primitives.boolData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setBool(std::string at, bool b)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::bool_type;
			it->second.data.clear();
			it->second.data.push_back(b);
			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::bool_type;
			e.primitives.boolData = b;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setString(std::string at, std::string value)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::string_type;
			it->second.data.clear();
			size_t size = value.length();
			it->second.data.resize(size);
			memcpy(it->second.data.data(), value.c_str(), size);

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};
			e.type = Entry::Types::string_type;
			size_t size = value.length();
			e.data.resize(size);
			memcpy(e.data.data(), value.c_str(), size);

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::getString(std::string at, std::string &s)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::string_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				s.clear();
				s.resize(it->second.data.size());
				memcpy(&s[0], it->second.data.data(), it->second.data.size());
				return Errors::noError;
			}
		}
	}

	std::vector<char> sfs::SafeSafeKeyValueData::formatIntoFileDataBinary() const
	{
		std::vector<char> ret;

		size_t size = 0;
		for (auto &e : entries)
		{
			auto &s = e.first;
			auto &d = e.second;

			size += s.size() + 1;
			size += d.data.size();
			size += 1; //type
			size += 4; //data size
		}

		ret.reserve(size);

		for (auto &e : entries)
		{
			auto &s = e.first;
			auto &d = e.second;

			for (auto c : s)
			{
				ret.push_back(c);
			}
			ret.push_back(0);

			ret.push_back(d.type);

			if (d.type == Entry::Types::rawData_type || d.type == Entry::Types::string_type
				|| d.type == Entry::Types::keyValueData_type
				)
			{
				size_t size = d.data.size();
				ret.push_back(((char *)(&size))[0]);
				ret.push_back(((char *)(&size))[1]);
				ret.push_back(((char *)(&size))[2]);
				ret.push_back(((char *)(&size))[3]);

				for (auto d : d.data)
				{
					ret.push_back(d);
				}
			}
			else if (d.type == Entry::Types::bool_type)
			{
				ret.push_back(d.primitives.boolData);
			}
			else if (d.type == Entry::Types::int_type)
			{
				std::int32_t i = d.primitives.intData;
				ret.push_back(((char *)(&i))[0]);
				ret.push_back(((char *)(&i))[1]);
				ret.push_back(((char *)(&i))[2]);
				ret.push_back(((char *)(&i))[3]);
			}
			else if (d.type == Entry::Types::float_type)
			{
				float f = d.primitives.floatData;
				ret.push_back(((char *)(&f))[0]);
				ret.push_back(((char *)(&f))[1]);
				ret.push_back(((char *)(&f))[2]);
				ret.push_back(((char *)(&f))[3]);
			}
			else if (d.type == Entry::Types::uint64_type)
			{
				auto i = d.primitives.uint64Data;
				ret.push_back(((char *)(&i))[0]);
				ret.push_back(((char *)(&i))[1]);
				ret.push_back(((char *)(&i))[2]);
				ret.push_back(((char *)(&i))[3]);
				ret.push_back(((char *)(&i))[4]);
				ret.push_back(((char *)(&i))[5]);
				ret.push_back(((char *)(&i))[6]);
				ret.push_back(((char *)(&i))[7]);
			}
			else if (d.type == Entry::Types::int64_type)
			{
				auto i = d.primitives.int64Data;
				ret.push_back(((char *)(&i))[0]);
				ret.push_back(((char *)(&i))[1]);
				ret.push_back(((char *)(&i))[2]);
				ret.push_back(((char *)(&i))[3]);
				ret.push_back(((char *)(&i))[4]);
				ret.push_back(((char *)(&i))[5]);
				ret.push_back(((char *)(&i))[6]);
				ret.push_back(((char *)(&i))[7]);
			}
			else if (d.type == Entry::Types::double_type)
			{
				auto i = d.primitives.doubleData;
				ret.push_back(((char *)(&i))[0]);
				ret.push_back(((char *)(&i))[1]);
				ret.push_back(((char *)(&i))[2]);
				ret.push_back(((char *)(&i))[3]);
				ret.push_back(((char *)(&i))[4]);
				ret.push_back(((char *)(&i))[5]);
				ret.push_back(((char *)(&i))[6]);
				ret.push_back(((char *)(&i))[7]);
			}
			else if (d.type == Entry::Types::char_type)
			{
				auto i = d.primitives.charData;
				ret.push_back(((char *)(&i))[0]);
			}
			else if (d.type == Entry::Types::uchar_type)
			{
				auto i = d.primitives.uCharData;
				ret.push_back(((char *)(&i))[0]);
			}
			else if (d.type == Entry::Types::uint_type)
			{
				auto i = d.primitives.uintData;
				ret.push_back(((char *)(&i))[0]);
				ret.push_back(((char *)(&i))[1]);
				ret.push_back(((char *)(&i))[2]);
				ret.push_back(((char *)(&i))[3]);
			}
			else if (d.type == Entry::Types::vec2_type)
			{
				auto vec = d.primitives.vec;
				ret.push_back(((char *)(&vec.x))[0]);
				ret.push_back(((char *)(&vec.x))[1]);
				ret.push_back(((char *)(&vec.x))[2]);
				ret.push_back(((char *)(&vec.x))[3]);

				ret.push_back(((char *)(&vec.y))[0]);
				ret.push_back(((char *)(&vec.y))[1]);
				ret.push_back(((char *)(&vec.y))[2]);
				ret.push_back(((char *)(&vec.y))[3]);
			}
			else if (d.type == Entry::Types::vec3_type)
			{
				auto vec = d.primitives.vec;
				ret.push_back(((char *)(&vec.x))[0]);
				ret.push_back(((char *)(&vec.x))[1]);
				ret.push_back(((char *)(&vec.x))[2]);
				ret.push_back(((char *)(&vec.x))[3]);

				ret.push_back(((char *)(&vec.y))[0]);
				ret.push_back(((char *)(&vec.y))[1]);
				ret.push_back(((char *)(&vec.y))[2]);
				ret.push_back(((char *)(&vec.y))[3]);

				ret.push_back(((char *)(&vec.z))[0]);
				ret.push_back(((char *)(&vec.z))[1]);
				ret.push_back(((char *)(&vec.z))[2]);
				ret.push_back(((char *)(&vec.z))[3]);
			}
			else if (d.type == Entry::Types::vec4_type)
			{
				auto vec = d.primitives.vec;
				ret.push_back(((char *)(&vec.x))[0]);
				ret.push_back(((char *)(&vec.x))[1]);
				ret.push_back(((char *)(&vec.x))[2]);
				ret.push_back(((char *)(&vec.x))[3]);

				ret.push_back(((char *)(&vec.y))[0]);
				ret.push_back(((char *)(&vec.y))[1]);
				ret.push_back(((char *)(&vec.y))[2]);
				ret.push_back(((char *)(&vec.y))[3]);

				ret.push_back(((char *)(&vec.z))[0]);
				ret.push_back(((char *)(&vec.z))[1]);
				ret.push_back(((char *)(&vec.z))[2]);
				ret.push_back(((char *)(&vec.z))[3]);

				ret.push_back(((char *)(&vec.w))[0]);
				ret.push_back(((char *)(&vec.w))[1]);
				ret.push_back(((char *)(&vec.w))[2]);
				ret.push_back(((char *)(&vec.w))[3]);
			}
			else if (d.type == Entry::Types::ivec2_type)
			{
				auto vec = d.primitives.ivec;
				ret.push_back(((char *)(&vec.x))[0]);
				ret.push_back(((char *)(&vec.x))[1]);
				ret.push_back(((char *)(&vec.x))[2]);
				ret.push_back(((char *)(&vec.x))[3]);

				ret.push_back(((char *)(&vec.y))[0]);
				ret.push_back(((char *)(&vec.y))[1]);
				ret.push_back(((char *)(&vec.y))[2]);
				ret.push_back(((char *)(&vec.y))[3]);
			}
			else if (d.type == Entry::Types::ivec3_type)
			{
				auto vec = d.primitives.ivec;
				ret.push_back(((char *)(&vec.x))[0]);
				ret.push_back(((char *)(&vec.x))[1]);
				ret.push_back(((char *)(&vec.x))[2]);
				ret.push_back(((char *)(&vec.x))[3]);

				ret.push_back(((char *)(&vec.y))[0]);
				ret.push_back(((char *)(&vec.y))[1]);
				ret.push_back(((char *)(&vec.y))[2]);
				ret.push_back(((char *)(&vec.y))[3]);

				ret.push_back(((char *)(&vec.z))[0]);
				ret.push_back(((char *)(&vec.z))[1]);
				ret.push_back(((char *)(&vec.z))[2]);
				ret.push_back(((char *)(&vec.z))[3]);
			}
			else if (d.type == Entry::Types::ivec4_type)
			{
				auto vec = d.primitives.ivec;
				ret.push_back(((char *)(&vec.x))[0]);
				ret.push_back(((char *)(&vec.x))[1]);
				ret.push_back(((char *)(&vec.x))[2]);
				ret.push_back(((char *)(&vec.x))[3]);

				ret.push_back(((char *)(&vec.y))[0]);
				ret.push_back(((char *)(&vec.y))[1]);
				ret.push_back(((char *)(&vec.y))[2]);
				ret.push_back(((char *)(&vec.y))[3]);

				ret.push_back(((char *)(&vec.z))[0]);
				ret.push_back(((char *)(&vec.z))[1]);
				ret.push_back(((char *)(&vec.z))[2]);
				ret.push_back(((char *)(&vec.z))[3]);

				ret.push_back(((char *)(&vec.w))[0]);
				ret.push_back(((char *)(&vec.w))[1]);
				ret.push_back(((char *)(&vec.w))[2]);
				ret.push_back(((char *)(&vec.w))[3]);
			}


		}

		return ret;
	}

	/*
	std::vector<char> SafeSafeKeyValueData::formatIntoFileDataTextBased()
	{

		std::vector<char> ret;

		size_t size = 0;
		for (auto &e : entries)
		{
			auto &s = e.first;
			auto &d = e.second;

			size += s.size() + 1; //name
			size += d.data.size();
			size += 10;
		}

		ret.reserve(size);

		for (auto &e : entries)
		{
			auto &s = e.first;
			auto &d = e.second;

			for (auto c : s)
			{
				ret.push_back(c);
			}
			ret.push_back(':');
			ret.push_back(' ');

			if (d.type == Entry::Types::string_type)
			{

				ret.push_back('\"');

				for (auto d : d.data)
				{
					ret.push_back(d);
				}

				ret.push_back('\"');
				ret.push_back('\n');


			}
			else if (d.type == Entry::Types::rawData_type)
			{
				//todo

			}
			else if (d.type == Entry::Types::bool_type)
			{
				if (d.primitives.boolData)
				{
					ret.push_back('t');
					ret.push_back('r');
					ret.push_back('u');
					ret.push_back('e');
					ret.push_back('\n');
				}
				else
				{
					ret.push_back('f');
					ret.push_back('a');
					ret.push_back('l');
					ret.push_back('s');
					ret.push_back('e');
					ret.push_back('\n');
				}
			}
			else if (d.type == Entry::Types::int_type)
			{
				std::int32_t i = d.primitives.intData;
				std::string str = std::to_string(i);
				for (auto c : str) { ret.push_back(c); }
				ret.push_back('\n');

			}
			else if (d.type == Entry::Types::float_type)
			{
				float f = d.primitives.floatData;
				std::string str = std::to_string(f);
				for (auto c : str) { ret.push_back(c); }
				ret.push_back('\n');
			}

		}

		return ret;
	}
	*/

	Errors SafeSafeKeyValueData::loadFromFileData(char *data, size_t size)
	{
		*this = {};

		std::string currentName = {};

		for (char *c = data; c < data + size; c++)
		{
			bool readingName = 1;
			if (*c == 0)
			{
				readingName = 0;
			}
			else
			{
				currentName.push_back(*c);
			}

			if (!readingName)
			{
				c++; if (c >= data + size) { return Errors::couldNotParseData; }

				char type = *c;

				if (type == Entry::Types::bool_type)
				{
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					bool b = *c;

					Entry e;
					e.type = Entry::Types::bool_type;
					e.primitives.boolData = b;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::int_type)
				{
					std::int32_t i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[3] = *c;

					Entry e;
					e.type = Entry::Types::int_type;
					e.primitives.intData = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::float_type)
				{
					float f = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[3] = *c;

					Entry e;
					e.type = Entry::Types::float_type;
					e.primitives.floatData = f;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::uint64_type)
				{
					std::uint64_t i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[3] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[4] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[5] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[6] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[7] = *c;

					Entry e;
					e.type = Entry::Types::uint64_type;
					e.primitives.uint64Data = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::int64_type)
				{
					std::int64_t i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[3] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[4] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[5] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[6] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[7] = *c;

					Entry e;
					e.type = Entry::Types::int64_type;
					e.primitives.int64Data = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::double_type)
				{
					double i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[3] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[4] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[5] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[6] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[7] = *c;

					Entry e;
					e.type = Entry::Types::double_type;
					e.primitives.doubleData = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::uint_type)
				{
					std::uint32_t i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[3] = *c;

					Entry e;
					e.type = Entry::Types::uint_type;
					e.primitives.uintData = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::char_type)
				{
					char i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;

					Entry e;
					e.type = Entry::Types::char_type;
					e.primitives.charData = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::uchar_type)
				{
					char i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;

					Entry e;
					e.type = Entry::Types::uchar_type;
					e.primitives.uCharData = i;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::rawData_type)
				{
					size_t s = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[3] = *c;

					Entry e;
					e.type = Entry::Types::rawData_type;
					e.data.reserve(s);

					for (int i = 0; i < s; i++)
					{
						c++; if (c >= data + size) { return Errors::couldNotParseData; }
						e.data.push_back(*c);
					}

					entries[currentName] = std::move(e);
				}
				else if (type == Entry::Types::string_type)
				{
					size_t s = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[3] = *c;

					Entry e;
					e.type = Entry::Types::string_type;
					e.data.reserve(s);

					for (int i = 0; i < s; i++)
					{
						c++; if (c >= data + size) { return Errors::couldNotParseData; }
						e.data.push_back(*c);
					}

					entries[currentName] = std::move(e);
				}
				else if (type == Entry::Types::keyValueData_type)
				{
					size_t s = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[3] = *c;

					Entry e;
					e.type = Entry::Types::keyValueData_type;
					e.data.reserve(s);

					for (int i = 0; i < s; i++)
					{
						c++; if (c >= data + size) { return Errors::couldNotParseData; }
						e.data.push_back(*c);
					}

					entries[currentName] = std::move(e);

				}
				else if (type == Entry::Types::vec2_type)
				{
					float x = 0;
					float y = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[3] = *c;

					Entry e;
					e.type = Entry::Types::vec2_type;
					e.primitives.vec.x = x;
					e.primitives.vec.y = y;
					e.primitives.vec.z = 0;
					e.primitives.vec.w = 0;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::vec3_type)
				{
					float x = 0;
					float y = 0;
					float z = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[3] = *c;

					Entry e;
					e.type = Entry::Types::vec3_type;
					e.primitives.vec.x = x;
					e.primitives.vec.y = y;
					e.primitives.vec.z = z;
					e.primitives.vec.w = 0;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::vec4_type)
				{
					float x = 0;
					float y = 0;
					float z = 0;
					float w = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[3] = *c;

					Entry e;
					e.type = Entry::Types::vec4_type;
					e.primitives.vec.x = x;
					e.primitives.vec.y = y;
					e.primitives.vec.z = z;
					e.primitives.vec.w = w;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::ivec2_type)
				{
					int x = 0;
					int y = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[3] = *c;

					Entry e;
					e.type = Entry::Types::ivec2_type;
					e.primitives.ivec.x = x;
					e.primitives.ivec.y = y;
					e.primitives.ivec.z = 0;
					e.primitives.ivec.w = 0;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::ivec3_type)
				{
					int x = 0;
					int y = 0;
					int z = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[3] = *c;

					Entry e;
					e.type = Entry::Types::ivec3_type;
					e.primitives.ivec.x = x;
					e.primitives.ivec.y = y;
					e.primitives.ivec.z = z;
					e.primitives.ivec.w = 0;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::ivec4_type)
				{
					int x = 0;
					int y = 0;
					int z = 0;
					int w = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&x))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&y))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&z))[3] = *c;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&w))[3] = *c;

					Entry e;
					e.type = Entry::Types::ivec4_type;
					e.primitives.ivec.x = x;
					e.primitives.ivec.y = y;
					e.primitives.ivec.z = z;
					e.primitives.ivec.w = w;

					entries[currentName] = e;
				}

				currentName = {};
			}

		}

		if (currentName == "")
		{
			return Errors::noError;
		}
		else
		{
			return Errors::couldNotParseData;
		}

	}


	bool SafeSafeKeyValueData::Entry::operator==(const Entry &other) const
	{
		if (type != other.type) { return false; }

		switch (type)
		{
		case no_type: { break; }
		case rawData_type: { if (data != other.data) { return false; } break; }
		case int_type: { if (primitives.intData != other.primitives.intData) { return false; } break; }
		case float_type: { if (primitives.floatData != other.primitives.floatData) { return false; } break; }
		case bool_type: { if (primitives.boolData != other.primitives.boolData) { return false; } break; }
		case string_type: { if (data != other.data) { return false; } break; }
		case uint64_type: { if (primitives.uint64Data != other.primitives.uint64Data) { return false; } break; }
		case int64_type: { if (primitives.int64Data != other.primitives.int64Data) { return false; } break; }
		case keyValueData_type: { if (data != other.data) { return false; } break; }

		case char_type: { if (primitives.charData != other.primitives.charData) { return false; } break; }
		case uchar_type: { if (primitives.uCharData != other.primitives.uCharData) { return false; } break; }
		case uint_type: { if (primitives.uintData != other.primitives.uintData) { return false; } break; }
		case double_type: { if (primitives.doubleData != other.primitives.doubleData) { return false; } break; }

		case vec2_type: { if (primitives.vec.x != primitives.vec.x || primitives.vec.y != primitives.vec.y) { return false; } break; }
		case vec3_type: { if (primitives.vec.x != primitives.vec.x || primitives.vec.y != primitives.vec.y || primitives.vec.z != primitives.vec.z) { return false; } break; }
		case vec4_type: { if (primitives.vec.x != primitives.vec.x || primitives.vec.y != primitives.vec.y || primitives.vec.z != primitives.vec.z || primitives.vec.w != primitives.vec.w) { return false; } break; }

		case ivec2_type: { if (primitives.ivec.x != primitives.ivec.x || primitives.ivec.y != primitives.ivec.y) { return false; } break; }
		case ivec3_type: { if (primitives.ivec.x != primitives.ivec.x || primitives.ivec.y != primitives.ivec.y || primitives.ivec.z != primitives.ivec.z) { return false; } break; }
		case ivec4_type: { if (primitives.ivec.x != primitives.ivec.x || primitives.ivec.y != primitives.ivec.y || primitives.ivec.z != primitives.ivec.z || primitives.ivec.w != primitives.ivec.w) { return false; } break; }

		}


		return true;
	}

}
