#include "pikaConsoleWindow.h"
#include <logs/assert.h>
#include <iostream>

namespace pika
{


	void ConsoleWindow::init(pika::pikaImgui::ImGuiIdsManager &idManager, std::streambuf *buffer)
	{
		this->buffer = buffer;
		imguiId = idManager.getImguiIds();
	}

	void ConsoleWindow::update(pika::LogManager &logManager, bool &open)
	{
		PIKA_DEVELOPMENT_ONLY_ASSERT(buffer, "invalid buffer");

		//s += buffer->get;
		//*buffer = std::streambuf();

		s += std::string{(std::istreambuf_iterator<char>(buffer)),
			std::istreambuf_iterator<char>()};

		constexpr int SIZE = 3000; //todo
		
		if (s.size() > SIZE)
		{
			s.erase(0, s.size() - SIZE);
		}

		ImGui::PushID(imguiId);

		if (!ImGui::Begin(ICON_NAME, &open))
		{
			ImGui::End();
			ImGui::PopID();
			return;
		}

		
		ImGui::BeginChild("##console scrolling", ImVec2(0, 0), false);


		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		
		ImGui::TextWrapped(s.c_str());
		//ImGui::TextUnformatted(s.c_str());

		ImGui::PopStyleVar();

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();


		ImGui::End();
		ImGui::PopID();

	}

};

