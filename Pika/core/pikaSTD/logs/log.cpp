#include "log.h"
#include <fstream>
#include <chrono>
#include <iomanip>

void pika::LogManager::init(std::string name)
{
	if (name == "")
	{
		internalLogs.reserve(10);
	}

	this->name = name;
	bool firstLog = 0;
	//this might do some other things in the future
}

void pika::LogManager::log(const char *l, int type)
{

#ifdef PIKA_DEVELOPMENT
	logInternally(l, type);
	logToFile(l, type);
#endif

#ifdef PIKA_PRODUCTION
	logToFile(l, type); //todo enum settings

#endif

}

std::stringstream pika::LogManager::formatLog(const char *l, int type)
{
	auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream s;
	s << "#" << std::put_time(std::localtime(&time), "%Y-%m-%d %X") << ": ";
	s << l << "\n";
	return s;
}

void pika::LogManager::logToFile(const char *l, int type)
{
	//todo unlickely atribute
	if (!firstLog)
	{
		firstLog = 1;
		std::ofstream file(name);
		file.close(); //clear the file content
	}

	std::ofstream file(name, std::ofstream::app);
	file << formatLog(l, type).rdbuf();
	file.close();
}

void pika::LogManager::logInternally(const char *l, int type)
{
	internalLogs.push_back(formatLog(l, type).str());
}
