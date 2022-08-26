#pragma once
//////////////////////////////////////////
//pikaConfig.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////



//////////////////////////////////////////
//All macros
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
//
//functions
//
// PIKA_PERMA_ASSERT
// PIKA_DEVELOPMENT_ONLY_ASSERT
//
//
//////////////////////////////////////////



///chose the assert function to be run on perma assert in production code

//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::assertFunctionProduction
#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::assertFunctionDevelopment
//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::assertFunctionToLog
//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION pika::assert::terminate
//#define PIKA_INTERNAL_CURRENT_ASSERT_FUNCTION //remove all asserts in production





#include <pikaConfigInternal.h>