#include "pikaConsoleWindow.h"
#include <logs/assert.h>
#include <iostream>

namespace pika
{


	void ConsoleWindow::init(pika::pikaImgui::ImGuiIdsManager &idManager)
	{
		imguiId = idManager.getImguiIds();
	}

	void ConsoleWindow::update(pika::LogManager &logManager, bool &open)
	{

		//s += buffer->get;
		//*buffer = std::streambuf();

		//s += std::string{(std::istreambuf_iterator<char>(buffer)),
		//	std::istreambuf_iterator<char>()};

		ImGui::PushID(imguiId);

		if (!ImGui::Begin(ICON_NAME, &open))
		{
			ImGui::End();
			ImGui::PopID();
			return;
		}

		
		ImGui::BeginChild("##console scrolling", ImVec2(0, 0), false);


		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		
		if (wrapped)
		{
			if(bufferBeginPos <= BUFFER_SIZE - 1)
			ImGui::TextWrapped(&buffer[bufferBeginPos + 1]);
			//ImGui::SameLine();
			ImGui::TextWrapped(buffer);
		}
		else
		{
			ImGui::TextWrapped(buffer);
		}

		//ImGui::TextUnformatted(s.c_str());

		ImGui::PopStyleVar();

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();


		ImGui::End();
		ImGui::PopID();

	}

	void ConsoleWindow::write(const char *c)
	{
		size_t i = 0;
		while (c[i] != 0)
		{
			buffer[bufferBeginPos] = c[i];
			i++;
			bufferBeginPos++;

			if (bufferBeginPos >= BUFFER_SIZE)
			{
				bufferBeginPos = 0;
				wrapped = true;
			}

			if (wrapped)
			{
				buffer[bufferBeginPos] = 0;
			}
		}

	}

};

