///////////////////////////////////////////
//do not remove this notice
//(c) Luta Vlad
// 
// safeSave 1.0.4
// 
///////////////////////////////////////////

#pragma once
#include <fstream>
#include <vector>
#include <unordered_map>
#include <initializer_list>

#ifdef _MSC_VER
#pragma warning( disable : 26812 )
#endif

namespace sfs
{
	enum Errors: int
	{
		noError = 0,
		couldNotOpenFinle,
		fileSizeDitNotMatch,
		checkSumFailed,
		couldNotMakeBackup,
		readBackup,
		warningEntryAlreadyExists,
		entryNotFound,
		entryHasDifferentDataType,
		couldNotParseData,
		fileSizeNotBigEnough,
	};

	const char *getErrorString(Errors e);

	//can return error: couldNotOpenFinle
	Errors readEntireFile(std::vector<char> &data, const char *name);

	//reades the content of a file (size bytes), if shouldMatchSize is false will read the entire fill untill size bytes are read or the entire file was read
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch
	Errors readEntireFile(void *data, size_t size, const char *name, bool shouldMatchSize,
		int *bytesRead = nullptr);

	//gets the file size
	//can return error: couldNotOpenFinle
	Errors getFileSize(const char *name, size_t &size);

	//reades the entire content of the data to a file and uses checkSum
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors readEntireFileWithCheckSum(void *data, size_t size, const char *name);

	//reades the entire content of the data to a file and uses checkSum
	//can return error: couldNotOpenFinle, fileSizeNotBigEnough
	Errors readEntireFileWithCheckSum(std::vector<char> &data, const char *name);

	//writes the entire content of the data to a file and uses checkSum
	//can return error: couldNotOpenFinle
	Errors writeEntireFileWithCheckSum(const void *data, size_t size, const char *name);

	//writes the entire content of the data to a file
	//can return error: couldNotOpenFinle
	Errors writeEntireFile(const std::vector<char> &data, const char *name);

	//writes the entire content of the data to a file
	//can return error: couldNotOpenFinle
	Errors writeEntireFile(const void *data, size_t size, const char *name);

	//saved the data with a check sum and a backup
	// It will also use a temporary file to make sure the saving is safe.
	//can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(const void *data, size_t size, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError);


	//saved the data with a check sum. It will use a temporary file to make sure the saving is safe.
	//can return error: couldNotOpenFinle
	Errors safeSaveNoBackup(const void *data, size_t size, const char *nameWithoutExtension);


	//loads the data that was saved using safeSave
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed, 
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded with the backup)'
	// is checkSumFailed is returned, the data was still read!
	Errors safeLoad(void *data, size_t size, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError);

	//loads the data that was saved using safeSave and stored as a SafeSafeKeyValueData structure
	//can return error: couldNotOpenFinle, checkSumFailed, fileSizeNotBigEnough
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded with the backup)
	Errors safeLoad(std::vector<char> &data, const char *nameWithoutExtension,
		bool reportLoadingBackupAsAnError);

	//same as safeLoad but only loads the backup file.
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors safeLoadBackup(void *data, size_t size, const char *nameWithoutExtension);


	//used to save data and read it
	struct SafeSafeKeyValueData
	{
		struct Entry
		{
			enum Types
			{
				no_type = 0,
				rawData_type,
				int_type,
				float_type,
				bool_type,
				string_type,
				uint64_type,
				int64_type,
				keyValueData_type,

				char_type,
				uchar_type,
				uint_type,
				double_type,

				vec2_type,
				vec3_type,
				vec4_type,

				ivec2_type,
				ivec3_type,
				ivec4_type,
			};

			std::vector<char> data;
			char type = 0;
			union Primitives
			{
				std::uint64_t uint64Data = 0;
				std::int64_t int64Data;
				std::int32_t intData;
				float floatData;
				bool boolData;
				char charData;
				unsigned char uCharData;
				unsigned int uintData;
				double doubleData;

				struct Vec
				{
					float x;
					float y;
					float z;
					float w;
				}vec;

				struct IVec
				{
					int x;
					int y;
					int z;
					int w;
				}ivec;

			}primitives;

			bool operator== (const Entry &other) const;

			bool operator!= (const Entry &other) const
			{
				return !(*this == other);
			}

			// Implicit constructors for primitive types
			Entry() = default;
			Entry(int v) { type = int_type; primitives.intData = v; }
			Entry(float v) { type = float_type; primitives.floatData = v; }
			Entry(bool v) { type = bool_type; primitives.boolData = v; }
			Entry(std::int64_t v) { type = int64_type; primitives.int64Data = v; }
			Entry(std::uint64_t v) { type = uint64_type; primitives.uint64Data = v; }
			Entry(const std::string &v)
			{
				type = string_type;
				data.assign(v.begin(), v.end());
			}
			Entry(const char *v): Entry(std::string(v)) {}

			Entry(char c) { type = char_type; primitives.charData = c; }
			Entry(unsigned char c) { type = uchar_type; primitives.uCharData = c; }
			Entry(unsigned int i) { type = uint_type; primitives.uintData = i; }
			Entry(double d) { type = double_type; primitives.doubleData = d; }


			Entry(float x, float y) { type = vec2_type; primitives.vec = {}; primitives.vec.x = x; primitives.vec.y = y; }
			Entry(float x, float y, float z) { type = vec3_type; primitives.vec = {}; primitives.vec.x = x; primitives.vec.y = y; primitives.vec.y = z; }
			Entry(float x, float y, float z, float w) { type = vec4_type; primitives.vec = {}; primitives.vec.x = x; primitives.vec.y = y; primitives.vec.y = z; primitives.vec.w = w; }

			Entry(int x, int y) { type = ivec2_type; primitives.ivec = {}; primitives.ivec.x = x; primitives.ivec.y = y; }
			Entry(int x, int y, int z) { type = ivec3_type; primitives.ivec = {}; primitives.ivec.x = x; primitives.ivec.y = y; primitives.ivec.y = z; }
			Entry(int x, int y, int z, int w) { type = ivec4_type; primitives.ivec = {}; primitives.ivec.x = x; primitives.ivec.y = y; primitives.ivec.y = z; primitives.ivec.w = w; }


		};

		SafeSafeKeyValueData() = default;

		SafeSafeKeyValueData(std::initializer_list<std::pair<std::string, Entry>> init)
		{
			for (const auto &pair : init)
			{
				entries[pair.first] = pair.second;
			}
		}

		std::unordered_map<std::string, Entry> entries;

		bool operator== (const SafeSafeKeyValueData &other) const;

		bool operator!= (const SafeSafeKeyValueData &other) const
		{
			return !(*this == other);
		}


		//returns true if entry exists
		bool entryExists(std::string at);

		//can return error: entryNotFound
		Errors getEntryType(std::string at, char &type);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setRawData(std::string at, void *data, size_t size);

		//this can be usefull to hold nested key value data objects in one another!
		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setKeyValueData(std::string at, const SafeSafeKeyValueData &data);

		//won't change data if failed
		//can return error: entryNotFound, entryHasDifferentDataType, couldNotParseData
		Errors getKeyValueData(std::string at, SafeSafeKeyValueData &data);

		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getRawDataPointer(std::string at, void *&data, size_t &size);

		//won't change i if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getInt(std::string at, int32_t &i);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setInt(std::string at, int32_t i);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setChar(std::string at, char c);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setUCHar(std::string at, unsigned char c);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setUInt(std::string at, uint32_t u);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setDouble(std::string at, double d);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setVec2(std::string at, float x, float y);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setVec3(std::string at, float x, float y, float z);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setVec4(std::string at, float x, float y, float z, float w);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setIVec2(std::string at, int x, int y);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setIVec3(std::string at, int x, int y, int z);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setIVec4(std::string at, int x, int y, int z, int w);


		//won't change the variable if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getVec2(std::string at, float &x, float &y);

		//won't change the variable if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getVec3(std::string at, float &x, float &y, float &z);

		//won't change the variable if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getVec4(std::string at, float &x, float &y, float &z, float &w);

		//won't change the variable if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getIVec2(std::string at, int &x, int &y);

		//won't change the variable if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getIVec3(std::string at, int &x, int &y, int &z);

		//won't change the variable if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getIVec4(std::string at, int &x, int &y, int &z, int &w);


		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setuInt64(std::string at, uint64_t i);

		//won't change i if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getuInt64(std::string at, uint64_t &i);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setInt64(std::string at, int64_t i);

		//won't change i if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getInt64(std::string at, int64_t &i);

		//won't change f if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getFloat(std::string at, float &f);


		//won't change c if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getChar(std::string at, char &c);

		//won't change c if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getUChar(std::string at, unsigned char &c);

		//won't change u if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getUInt(std::string at, unsigned int &u);

		//won't change d if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getDobule(std::string at, double &d);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setFloat(std::string at, float f);

		//won't change b if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getBool(std::string at, bool &b);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setBool(std::string at, bool b);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setString(std::string at, std::string value);

		//won't change s if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getString(std::string at, std::string &s);

		std::vector<char> formatIntoFileDataBinary() const;

		//not finished yet
		//std::vector<char> formatIntoFileDataTextBased();


		//can return error: couldNotParseData
		Errors loadFromFileData(char *data, size_t size);
	};

	//saved the data stored as a SafeSafeKeyValueData structure in a binary format with a check sum and a backup
	//Uses a temporary file to make sure the saving is safe
	//can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError);


	//saved the data stored as a SafeSafeKeyValueData structure in a binary format with a check.
	//Uses a temporary file to make sure the saving is safe
	//can return error: couldNotOpenFinle, 
	Errors safeSaveNoBackup(SafeSafeKeyValueData &data, const char *nameWithoutExtension);


	//loads the data that was saved using safeSave and stored as a SafeSafeKeyValueData structure
	//can return error: couldNotOpenFinle, fileSizeNotBigEnough, checkSumFailed, couldNotParseData
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded from the backup)
	Errors safeLoad(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError);



#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__ || defined __APPLE__

	struct FileMapping
	{
		void *pointer = {};
		size_t size = 0;
		struct
		{
			void *fileHandle = 0;
			void *fileMapping = 0;
		}internal = {};
	};

#elif defined __linux__

	struct FileMapping
	{
		void *pointer = {};
		size_t size = 0;
		struct
		{
			int fd = 0;
		}internal = {};
	};

#endif

	//a file mapping maps the content of a file to ram
	//can return error: couldNotOpenFinle
	Errors openFileMapping(FileMapping &fileMapping, const char *name, size_t size, bool createIfNotExisting);

	void closeFileMapping(FileMapping &fileMapping);

};