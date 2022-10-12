#include "pushNotification.h"
#include <imgui.h>
#include <iostream>
#include <pikaConfig.h>
#include <IconsForkAwesome.h>
#include <pikaImgui/pikaImgui.h>

#if PIKA_SHOULD_REMOVE_PUSH_NOTIFICATIONS

#define PIKA_NOT_IMPLEMENT


void pika::PushNotificationManager::init()
{}

void pika::PushNotificationManager::update(bool &open)
{}

void pika::PushNotificationManager::pushNotification(const char *content, int logType)
{}


#endif

#ifndef PIKA_NOT_IMPLEMENT

void pika::PushNotificationManager::init()
{
}

void pika::PushNotificationManager::update(bool &open)
{

	if (notificationQue.empty())
	{
		return;
	}

	static int corner = 0;
	ImGuiIO &io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
		| ImGuiWindowFlags_AlwaysAutoResize;

	//set popup pos
	{
		const float PADX = 10.0f;
		const float PADY = 40.0f;
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PADX) : (work_pos.x + PADX);
		window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PADY) : (work_pos.y + PADY);
		window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
		window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowViewport(viewport->ID);
		window_flags |= ImGuiWindowFlags_NoMove;
	}

	static int lastSize = 0; //todo move

	if (lastSize < notificationQue.size())
	{
		ImGui::SetNextWindowFocus();
	}

	lastSize = notificationQue.size();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
	if (ImGui::Begin("Notification content", &open, window_flags))
	{

		for (auto i = 0; i < notificationQue.size(); i++)
		{
			if (i != 0)
			{
				ImGui::Separator();
			}

			if (notificationQue[i].notificationType == pika::logError)
			{
				pika::pikaImgui::addErrorSymbol();
				ImGui::SameLine();
			}

			if (notificationQue[i].notificationType == pika::logWarning)
			{
				pika::pikaImgui::addWarningSymbol();
				ImGui::SameLine();
			}
			
			ImGui::Text(notificationQue[i].content.c_str(), i);
		

		}



		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (ImGui::MenuItem("Close")) notificationQue.clear();
			ImGui::EndPopup();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();


	while (!notificationQue.empty() && notificationQue.front().startTime +
		std::chrono::seconds(5) < std::chrono::steady_clock::now())
	{
		notificationQue.pop_front();
	}


}

void pika::PushNotificationManager::pushNotification(const char *content, int logType)
{
	notificationQue.push_back
	( Notification(std::string(content), std::chrono::steady_clock::now(), logType) );
}

#endif
 





