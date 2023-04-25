#pragma once
#include <intrin.h>
#include <Windows.h>

///////////////////////////////////////////
//https://github.com/meemknight/profilerLib
//do not remove this notice
//(c) Luta Vlad
///////////////////////////////////////////


//set this to true to remove the implementation
//usefull to quickly remove debug and profile cod and to port to
//other platforms
#define PROFILER_LIB_REMOVE_IMPLEMENTATION 0


namespace PL 
{

	struct ProfileRezults
	{
		float timeSeconds;
		unsigned int cpuClocks;
	};

#if !PROFILER_LIB_REMOVE_IMPLEMENTATION

	struct PerfFreqvency
	{
		PerfFreqvency()
		{
			QueryPerformanceFrequency(&perfFreq);
		}

		LARGE_INTEGER perfFreq;
	};
	const static PerfFreqvency freq;


	struct Profiler
	{

		LARGE_INTEGER startTime = {};
		__int64 cycleCount = {};

		void start()
		{
			QueryPerformanceCounter(&startTime);
			cycleCount = __rdtsc();
		}

		ProfileRezults end()
		{
			__int64 endCycleCount = __rdtsc();
			LARGE_INTEGER endTime;
			QueryPerformanceCounter(&endTime);

			cycleCount = endCycleCount - cycleCount;
			startTime.QuadPart = endTime.QuadPart - startTime.QuadPart;


			ProfileRezults r = {};

			r.timeSeconds = (float)startTime.QuadPart / (float)freq.perfFreq.QuadPart;
			r.cpuClocks = cycleCount;

			return r;
		}

	};

	const int AverageProfilerMaxTests = 200;

	struct AverageProfiler
	{
		ProfileRezults rezults[AverageProfilerMaxTests];
		int index = 0;

		Profiler profiler;

		void start()
		{
			profiler.start();
		}

		ProfileRezults end()
		{
			auto r = profiler.end();
			
			if(index < AverageProfilerMaxTests)
			{
				rezults[index] = r;
				index++;
			}

			return r;
		}

		ProfileRezults getAverageNoResetData()
		{
			if (index == 0)
			{
				return { 0,0 };
			}

			long double time = 0;
			unsigned long cpuTime = 0;

			for(int i=0;i<index;i++)
			{
				time += rezults[i].timeSeconds;
				cpuTime += rezults[i].cpuClocks;
			}


			return { (float)(time / index), cpuTime /index };
		}
		
		void resetData()
		{
			index = 0;
		}

		ProfileRezults getAverageAndResetData()
		{
			auto r = getAverageNoResetData();
			resetData();
			return r;
		}


	};

#else

	struct Profiler
	{
	
		
		void start()
		{
			
		}
	
		ProfileRezults end()
		{	
			return {};
		}
	
	};
	
	
	struct AverageProfiler
	{
		
		void start()
		{
		}
	
		ProfileRezults end()
		{
			return {};
		}
	
		ProfileRezults getAverageNoResetData()
		{
			
			return {};
		}
	
		void resetData()
		{
		}
	
		ProfileRezults getAverageAndResetData()
		{
			return {};
		}
	
	};



#endif

};