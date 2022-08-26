#pragma once

#pragma region Platform



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) || defined(__NT__)

#define PIKA_WINDOWS 1

#elif defined(__linux__)

#define PIKA_LINUX 1

#else

#error "Pika supports only windows and linux"

#endif

#pragma endregion
