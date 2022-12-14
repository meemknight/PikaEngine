#include "fileManipulation.h"
#include <fstream>

namespace pika
{



	size_t getFileSize(const char *name)
	{
		std::ifstream f(name, std::ios::binary);
		if (!f.is_open()) { return 0; }

		f.seekg(0, std::ios_base::end);
		size_t size = f.tellg();

		f.close();

		return size;
	}

	size_t readEntireFile(const char *name, void *buffer, size_t s, size_t from)
	{
		std::ifstream f(name, std::ios::binary);
		if (!f.is_open()) { return 0; }

		f.seekg(from);

		f.read((char*)buffer, s);

		size_t pos = f.tellg();

		f.close();

		return pos;
	}

	bool writeEntireFile(const char *name, void *buffer, size_t s)
	{
		std::ofstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.write((char *)buffer, s);
			f.close();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool appendToFile(const char *name, void *buffer, size_t s)
	{
		std::ofstream f(name, std::ios::binary | std::ios::app);

		if (f.is_open())
		{
			f.write((char *)buffer, s);
			f.close();
			return true;
		}
		else
		{
			return false;
		}
	}

	void deleteFile(const char *f)
	{
	}





};