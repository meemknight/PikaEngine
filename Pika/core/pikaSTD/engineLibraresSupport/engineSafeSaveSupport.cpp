#include "engineSafeSaveSupport.h"
#include <globalAllocator/globalAllocator.h>

namespace pika
{
	namespace sfs
	{

		::sfs::Errors safeLoad(::sfs::SafeSafeKeyValueData &data,
			const char *nameWithoutExtension, bool reportLoadingBackupAsAnError)
		{
			data = {};

			std::vector<char> readData;
			readData.reserve(4000);

			pika::memory::pushCustomAllocatorsToStandard();
			auto errCode = ::sfs::safeLoad(readData, nameWithoutExtension, reportLoadingBackupAsAnError);
			pika::memory::popCustomAllocatorsToStandard();

			if (errCode == ::sfs::noError || errCode == ::sfs::readBackup)
			{
				return data.loadFromFileData(readData.data(), readData.size());
			}
			else
			{
				return errCode;
			}

		}

	}
}
