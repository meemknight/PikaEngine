#include "log.h"
#include <fstream>
#include <chrono>
#include <iomanip>

void pika::LogManager::init(std::string name)
{

	this->name = name;
	bool firstLog = 0;
	
}

void pika::LogManager::log(const char *l, int type)
{

#ifdef PIKA_DEVELOPMENT
	logInternally(l, type);
	logToFile(l, type);
	logToPushNotification(l, type);
#endif

#ifdef PIKA_PRODUCTION

	#if !PIKA_REMOVE_LOGS_TO_FILE_IN_PRODUCTION
	logToFile(l, type);
	#endif

	#if !PIKA_REMOVE_LOGS_TO_NOTIFICATIONS_IN_PRODUCTION
	logToPushNotification(l, type);
	#endif

#endif

}

std::stringstream formatLog(const char *l, int type)
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
		std::ofstream file(name);	//no need to check here
		file.close(); //clear the file content
	}

	pika::logToFile(name.c_str(), l, type);
}

void pika::LogManager::logInternally(const char *l, int type)
{
	if (internalLogs.size() >= maxInternalLogCount)
	{
		internalLogs.pop_front();
	}

	internalLogs.push_back(formatLog(l, type).str());
}

void pika::LogManager::logToPushNotification(const char *l, int type)
{
	if (pushNotificationManager)
	{
		pushNotificationManager->pushNotification(l);
	}

}

void pika::logToFile(const char *fileName, const char *l, int type)
{
	std::ofstream file(fileName, std::ofstream::app);
	if (!file.is_open()) { return; }

	file << formatLog(l, type).rdbuf();
	file.close();
}
