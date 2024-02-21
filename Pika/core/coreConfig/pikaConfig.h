#pragma once
//////////////////////////////////////////
//pikaConfig.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////



//////////////////////////////////////////
//
//constants
//
//PIKA_WINDOWS
//PIKA_LINUX
//PIKA_DEVELOPMENT
//PIKA_PRODUCTION
// 
//  
//////////////////////////////////////////




///chose the assert function to be run on perma assert in production code

//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::assertFunctionProduction
#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::assertFunctionDevelopment
//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::assertFunctionToLog
//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::terminate
//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION //remove all asserts in production (I don't recommand this)




#define PIKA_REMOVE_PUSH_NOTIFICATIONS_IN_PRODUCTION 0


#define PIKA_REMOVE_LOGS_TO_FILE_IN_PRODUCTION 0
#define PIKA_REMOVE_LOGS_TO_NOTIFICATIONS_IN_PRODUCTION 0


#define PIKA_REMOVE_OPTIONAL_NOVALUE_CHECKS_IN_PRODUCTION 1


#define PIKA_REMOVE_PUSH_NOTIFICATION_IN_PRODUCTION 0



#define PIKA_REMOVE_IMGUI_IN_PRODUCTION 1


 #define PIKA_ENGINE_SAVES_PATH PIKA_ENGINE_RESOURCES_PATH "/engineSaves/"


#include <pikaConfigInternal.h>

