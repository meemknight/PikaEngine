#include "pushNotification.h"
#include <imgui.h>
#include <iostream>


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
		const float PADY = 20.0f;
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
	
	ImGui::SetNextWindowBgAlpha(0.30f); // Transparent background
	if (ImGui::Begin("Example: Simple overlay", &open, window_flags))
	{

		for (auto i = 0; i < notificationQue.size(); i++)
		{
			if (i!=0)
			{
				ImGui::Separator();
			}
			
			//ImGui::PushID(i);
			ImGui::Text(notificationQue[i].content.c_str(), i);
			//ImGui::PopID();
		}

		//ImGui::Text("adfgh");
		//ImGui::Separator();
		//ImGui::Text("adfgh");
		//ImGui::Separator();
		//ImGui::Text("adfgh");



		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (ImGui::MenuItem("Close")) open = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();

	
	while (!notificationQue.empty() && notificationQue.front().startTime +
		std::chrono::seconds(5) < std::chrono::steady_clock::now())
	{
		notificationQue.pop_front();
	}


}

void pika::PushNotificationManager::pushNotification(const char *content)
{
	notificationQue.push_back
		(Notification(std::string(content), std::chrono::steady_clock::now()));
}
