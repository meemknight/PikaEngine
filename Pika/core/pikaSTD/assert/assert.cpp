#include <pikaConfig.h>
#include "assert.h"
#include <cstdlib>
#include <cstdio>


namespace pika
{

	namespace assert
	{

		void terminate()
		{
			std::exit(1);
		}


	#ifdef PIKA_WINDOWS

	#include <Windows.h>

		void assertFunctionInternal(const char *expression, const char *file,
			int line, const char *comment)
		{
			char buffer[1024] = {};

			std::snprintf(buffer, sizeof(buffer),
				"Assertion failed\n\n"

				"Expression: \n%s\n\n"

				"File: %s\n"
				"Line: %d\n\n"

				"Comment: \n%s\n\n"

				"Press retry to debug."
				, expression
				, file
				, line
				, comment
			);

			int const action = MessageBoxA(0, buffer, "Pika error", MB_TASKMODAL
				| MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);

			switch (action)
			{
				case IDABORT: //Abort the program
				{
					terminate();
					return;
				}
				case IDRETRY: //Break execution (debug)
				{
				#ifdef _MSC_VER
					__debugbreak();
				#endif
					terminate();

					return;
				}
				case IDIGNORE: //Ignore assert
				{
					return;
				}
			
			}

		};


		void assertFunctionProduction(const char *expression, const char *file,
			int line, const char *comment)
		{

			char buffer[1024] = {};

			std::snprintf(buffer, sizeof(buffer),
				"Assertion failed\n\n"

				"Expression: \n%s\n\n"

				"File: %s\n"
				"Line: %d\n\n"

				"Comment: \n%s\n\n"

				"Please report this error to the developer."
				, expression
				, file
				, line
				, comment
			);


			int const action = MessageBoxA(0,
				buffer, "Pika error", MB_TASKMODAL
				| MB_ICONHAND | MB_OK | MB_SETFOREGROUND);

			terminate();
		}


	#elif defined(PIKA_LINUX)


		void assertFunctionProduction(const char *expression, const char *file,
			int line, const char *comment)
		{
			terminate();
		}


	#endif


	}


};
