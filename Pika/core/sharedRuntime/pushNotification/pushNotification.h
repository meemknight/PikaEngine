#pragma once
#include <string>
#include <chrono>
#include <deque>

namespace pika
{
	enum: int
	{
		logNormal = 0,
		logWarning,
		logError
	};

	struct Notification
	{
		std::string content = "";
		std::chrono::steady_clock::time_point startTime = {};
		int notificationType = pika::logNormal;

		Notification(std::string content, std::chrono::steady_clock::time_point startTime, int notificationType):
			content(content), startTime(startTime), notificationType(notificationType)
		{
		};

		Notification() {};
	};


	struct PushNotificationManager
	{

		void init();

		void update(bool &open);

		void pushNotification(const char *content, int logType = pika::logNormal);

		std::deque<Notification> notificationQue;
	};




};