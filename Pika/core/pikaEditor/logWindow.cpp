#include "logWindow.h"
#include <imgui.h>

void pika::LogWindow::init()
{


}

void pika::LogWindow::update(pika::LogManager &logManager)
{
	//todo push pop id


	if (ImGui::Begin("logs"))
	{


		ImGui::Text("log");



	}
	ImGui::End();


}
