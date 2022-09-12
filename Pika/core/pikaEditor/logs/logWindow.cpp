#include "logWindow.h"
#include <imgui.h>
#include <IconsForkAwesome.h>

void pika::LogWindow::init()
{


}

void pika::LogWindow::update(pika::LogManager &logManager, bool &open)
{
	//todo push pop id


	if (ImGui::Begin(ICON_NAME, &open))
	{


		ImGui::Text("log");



	}
	ImGui::End();


}
