#include "fileChanged.h"

namespace pika
{


	void FileChanged::setFile(const char *path)
	{
		this->path = path;

		WIN32_FILE_ATTRIBUTE_DATA Data = {};
		if (GetFileAttributesEx(path, GetFileExInfoStandard, &Data))
		{
			time = Data.ftLastWriteTime;
		}
		else
		{
			time = {};
		}

	}
	

	bool FileChanged::changed()
	{
		if (time.dwHighDateTime == 0
			&& time.dwLowDateTime == 0)
		{
			return 0;
		}

		if (path.empty()) { return 0; }

		WIN32_FILE_ATTRIBUTE_DATA Data = {};
		if (GetFileAttributesEx(path.string().c_str(), GetFileExInfoStandard, &Data))
		{
			if (time.dwHighDateTime == Data.ftLastWriteTime.dwHighDateTime
				&& time.dwLowDateTime == Data.ftLastWriteTime.dwLowDateTime)
			{
				return 0;
			}
			else
			{
				time = Data.ftLastWriteTime;
				return 1;
			}
		}

		return 0;
	}


};