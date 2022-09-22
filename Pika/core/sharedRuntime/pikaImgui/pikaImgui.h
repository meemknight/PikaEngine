#pragma once
#include <pikaConfig.h>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"


#include <pikaContext.h>

namespace pika
{


	void initImgui(PikaContext &pikaContext);
	void setImguiContext(PikaContext pikaContext);
	void imguiStartFrame(PikaContext pikaContext);
	void imguiEndFrame(PikaContext pikaContext);

	namespace EditorImguiIds
	{
		enum
		{
			mainEditorWindow = 100,
			editShortcutWindow = 200,
			
			
			idsCount = 4000
		};
	}

};
