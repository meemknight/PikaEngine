#include "openglLogs.h"

#include <glad/glad.h>

//https://learnopengl.com/In-Practice/Debugging
void GLAPIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char *message,
	const void *userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204
		|| id == 131222
		|| id == 131140 //that dittering thing
		) return;
	
	pika::OpenglLogsWindow *openglLogsWindow = (pika::OpenglLogsWindow*)userParam;

	if (type == GL_DEBUG_TYPE_PERFORMANCE) return;

	openglLogsWindow->errorsReported[id] ++;


	__noop();

}

void pika::OpenglLogsWindow::init(pika::pikaImgui::ImGuiIdsManager &idManager)
{
	imguiId = idManager.getImguiIds();

	//glEnable(GL_DEBUG_OUTPUT);
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageCallback(glDebugOutput, this);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

void pika::OpenglLogsWindow::update(bool &open)
{

	ImGui::PushID(imguiId);


	if (!ImGui::Begin(ICON_NAME, &open))
	{
		ImGui::End();
		ImGui::PopID();
		return;
	}
	ImGui::SetWindowSize({300,100}, ImGuiCond_FirstUseEver);

	for (auto &i : errorsReported)
	{
		ImGui::Text("Message id %iu, count %iu", i.first, i.second);
	}

	
	ImGui::End();
	ImGui::PopID();

}
