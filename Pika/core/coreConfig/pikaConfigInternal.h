#pragma once
//////////////////////////////////////////
//pikaConfigInternal.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#pragma region Platform



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) || defined(__NT__)

#define PIKA_WINDOWS 1

#elif defined(__linux__)

#define PIKA_LINUX 1

#else

#error "Pika supports only windows and linux"

#endif

#pragma endregion

#ifdef PIKA_DEVELOPMENT
#define PIKA_API extern "C" __declspec(dllexport)
#elif defined(PIKA_PRODUCTION)
#define PIKA_API
#endif
