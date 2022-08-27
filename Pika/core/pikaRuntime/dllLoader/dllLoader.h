#pragma once
#include <filesystem>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>

#define TESTSTART(x) void x(pika::PikaContext pikaContext)
typedef TESTSTART(testStart_t);
//extern "C" __declspec(dllexport) TESTPRINT(gameLogic);
#undef TESTSTART

#define TESTUPDATE(x) void x(pika::PikaContext pikaContext)
typedef TESTUPDATE(testUpdate_t);
#undef TESTUPDATE



namespace pika
{

bool loadDll(std::filesystem::path path, 
	testStart_t** testPrint, testUpdate_t** testUpdate);

};
