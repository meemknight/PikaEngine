#pragma once
#include <pikaConfig.h>



namespace pika
{

	namespace assert
	{


		void terminate();

		void assertFunctionInternal(
			const char *expression, 
			const char *file,
			int line, 
			const char *comment = nullptr);

		void assertFunctionProduction
		(
			const char *expression,
			const char *file,
			int line,
			const char *comment = nullptr
		);


	}

}