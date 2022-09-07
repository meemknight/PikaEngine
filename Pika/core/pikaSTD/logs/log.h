#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace pika
{


	struct LogManager
	{

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

		std::vector<std::string> internalLogs;

	private:
		//used only interally.
		std::stringstream formatLog(const char *l, int type = logNormal);
		void logToFile(const char *l, int type = logNormal);
		void logInternally(const char *l, int type = logNormal);

	};





}