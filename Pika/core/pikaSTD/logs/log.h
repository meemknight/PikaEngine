#pragma once
#include <string>
#include <deque>
#include <sstream>
#include <pushNotification/pushNotification.h>

namespace pika
{


	struct LogManager
	{

		static constexpr const char *DefaultLogFile = PIKA_RESOURCES_PATH "logs.txt";

		//a null name will just log to a internal structure
		void init(std::string name);

		
		//this will be dependent on the configuration of the project. 
		void log(const char *l, int type = pika::logNormal);
		

		std::string name = "";
		bool firstLog = 0;

		static constexpr int maxInternalLogCount = 200;
		std::deque<std::string> internalLogs; //used to print the logs in a internal console

		
		pika::PushNotificationManager *pushNotificationManager = 0;
	private:
		//used only interally.
		void logToFile(const char *l, int type = pika::logNormal);
		void logInternally(const char *l, int type = pika::logNormal);
		void logToPushNotification(const char *l, int type = pika::logNormal);

	};


	void logToFile(const char *fileName, const char *l, int type = pika::logNormal);


}