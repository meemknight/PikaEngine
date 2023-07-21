#pragma once

#include <safeSave/safeSave.h>

namespace pika
{
	namespace sfs
	{
		
		::sfs::Errors safeLoad(::sfs::SafeSafeKeyValueData &data, 
			const char *nameWithoutExtension, bool reportLoadingBackupAsAnError);

	}
}
