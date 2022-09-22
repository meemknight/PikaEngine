#pragma once
#include <string>
#include <deque>
#include <sstream>
#include <pushNotification/pushNotification.h>

namespace pika
{


	struct LogManager
	{

		static constexpr const char *DefaultLogFile = "logs.txt";

		//a null name will just log to a internal structure
		void init(std::string name);

		enum : int
		{
			logNormal = 0,
			logWarning,
			logError
		};

		//this will be dependent on the configuration of the project. 
		void log(const char *l, int type = logNormal);
		

		std::string name = "";
		bool firstLog = 0;

		std::deque<std::string> internalLogs;
		static constexpr int maxInternalLogCount = 200;

		
		PushNotificationManager *pushNotificationManager = 0;
	private:
		//used only interally.
		void logToFile(const char *l, int type = logNormal);
		void logInternally(const char *l, int type = logNormal);
		void logToPushNotification(const char *l, int type = logNormal);

	};


	void logToFile(const char *fileName, const char *l, int type = LogManager::logNormal);


}