#pragma once

//todo debug release configurations cmake

//compiler
//
#ifdef _MSC_VER
	#define PIKA_MSVC
#endif

#ifdef __GNUC__
	#define PIKA_GCC
#endif

#ifdef __clang__
	#define PIKA_CLANG
#endif
//


//assume
#ifdef PIKA_MSVC
	#define PIKA_ASSUME(x) __assume((x))
#else
	#define PIKA_ASSUME(x)
#endif



//optimize off
#ifdef PIKA_MSVC
	#define PIKA_OPTIMIZE_OFF() __pragma(optimize( "", off ))
#else
	#define PIKA_OPTIMIZE_OFF()
#endif



//optimize on
#ifdef PIKA_MSVC
	#define PIKA_OPTIMIZE_ON() __pragma(optimize( "", on ))
#else
	#define PIKA_OPTIMIZE_ON()
#endif




