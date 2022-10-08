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




#if (defined(PIKA_PRODUCTION) && PIKA_REMOVE_IMGUI_IN_PRODUCTION)
#define PIKA_SHOULD_REMOVE_IMGUI 1
#else
#define PIKA_SHOULD_REMOVE_IMGUI 0
#endif


#if ((defined(PIKA_PRODUCTION) && PIKA_REMOVE_PUSH_NOTIFICATION_IN_PRODUCTION) || PIKA_SHOULD_REMOVE_IMGUI)
#define PIKA_SHOULD_REMOVE_PUSH_NOTIFICATIONS 1
#else
#define PIKA_SHOULD_REMOVE_PUSH_NOTIFICATIONS 0
#endif


#if ((defined(PIKA_PRODUCTION) && PIKA_REMOVE_EDITOR_IN_PRODUCATION) || PIKA_SHOULD_REMOVE_IMGUI)
#define PIKA_SHOULD_REMOVE_EDITOR 1
#else
#define PIKA_SHOULD_REMOVE_EDITOR 0
#endif
