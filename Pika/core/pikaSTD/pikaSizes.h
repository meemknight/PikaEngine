#pragma once
//////////////////////////////////////////
//pikaSizes.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

namespace pika
{
	inline size_t constexpr KB(size_t x) { return x * 1024ull; }
	inline size_t constexpr MB(size_t x) { return KB(x) * 1024ull; }
	inline size_t constexpr GB(size_t x) { return MB(x) * 1024ull; }
	inline size_t constexpr TB(size_t x) { return GB(x) * 1024ull; }

	inline float constexpr BYTES_TO_KB(size_t x) { return x / 1024.f; }
	inline float constexpr BYTES_TO_MB(size_t x) { return BYTES_TO_KB(x) / 1024.f; }
	inline float constexpr BYTES_TO_GB(size_t x) { return BYTES_TO_MB(x) / 1024.f; }

	inline void align64(size_t &val)
	{
		val = val | 0b111;
	};

	inline void align64(char *&val)
	{
		val = (char *)((size_t)val | 0b111);
	};

};
