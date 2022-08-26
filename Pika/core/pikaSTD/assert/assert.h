#pragma once
#include <pikaConfig.h>


namespace pika
{

	namespace assert
	{

		//arguments don't do anything here
		void terminate(...);

		void assertFunctionDevelopment(
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

		void assertFunctionToLog(
			const char *expression,
			const char *file,
			int line,
			const char *comment = nullptr);

	}

}



#define PIKA_PERMA_ASSERT(expression, comment) (void)(			\
			(!!(expression)) ||									\
			(PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION(#expression,			\
				__FILE__, __LINE__, comment), 0)				\
)


#ifdef PIKA_DEVELOPMENT

#define PIKA_DEVELOPMENT_ONLY_ASSERT(expression, comment) (void)(			\
			(!!(expression)) ||												\
			(pika::assert::assertFunctionDevelopment(#expression,			\
				__FILE__, __LINE__, comment), 0)							\
)

#elif defined(PIKA_PRODUCTION)

#define PIKA_DEVELOPMENT_ONLY_ASSERT(expression, comment)

#endif
