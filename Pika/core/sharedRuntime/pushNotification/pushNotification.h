#pragma once
#include <string>
#include <chrono>
#include <deque>

namespace pika
{

	struct Notification
	{
		std::string content = "";
		std::chrono::steady_clock::time_point startTime = {};

		Notification(std::string content, std::chrono::steady_clock::time_point startTime):
			content(content), startTime(startTime){};

		Notification() {};
	};


	struct PushNotificationManager
	{

		void init();

		void update(bool &open);

		void pushNotification(const char *content);

		std::deque<Notification> notificationQue;
	};




}