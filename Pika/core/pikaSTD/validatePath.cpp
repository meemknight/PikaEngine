#pragma once
#include <stringManipulation/stringManipulation.h>

namespace pika
{


	bool isFileNameValid(const char *p, size_t s)
	{
		if (s == 0 || p[0] == 0) { return false; }

		for (size_t i = 0; i < s; i++)
		{

			if (pika::findChar("#<$+%>!`&*'|{}?/\\=+ \t\v\r\"", p[i]))
			{
				return 0;
			}
		}
		
		return 1;

	}




}