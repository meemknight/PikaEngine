#pragma once
#include <string>
#include <vector>

namespace pika
{
	//todo overengineer with restrict and stuff



	//dest can be also source
	void removeCharacters(char *dest, const char *source, const char *charsToRemove, size_t destSize);

	//dest can be also source
	void toLower(char *dest, const char *source, size_t size);

	//dest can be also source
	void toUpper(char *dest, const char *source, size_t size);


	//checks if char is in source
	bool findChar(const char *source, char c);


	std::vector<std::string> split(const char *source, char c);
	

}