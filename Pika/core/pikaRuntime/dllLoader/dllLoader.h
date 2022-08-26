#pragma once
#include <filesystem>

#define TESTPRINT(x) void x()
typedef TESTPRINT(testPrint_t);
//extern "C" __declspec(dllexport) TESTPRINT(gameLogic);
#undef TESTPRINT


namespace pika
{


bool loadDll(std::filesystem::path path, testPrint_t** testPrint);

};
