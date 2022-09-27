#pragma once
#include <pikaConfig.h>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"
#include <IconsForkAwesome.h>


#include <pikaContext.h>

#ifdef PIKA_WINDOWS
#define IM_PRId64   "I64d"
#define IM_PRIu64   "I64u"
#define IM_PRIx64   "I64X"
#else
#define IM_PRId64   "lld"
#define IM_PRIu64   "llu"
#define IM_PRIx64   "llX"
#endif

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
			logWindow = 300,
			containersWindow = 400,
			
			idsCount = 4000
		};
	}

	struct ImGuiIdsManager
	{
		int counter = EditorImguiIds::idsCount + 1;

		//returns the first id. (count) ids will be reserved.
		//if you want 5 ids and the function returns 10, then ids 10 11 12 13 14 will be reserved.
		int getImguiIds(unsigned int count = 1)
		{
			return counter + count;
		}
	};

	void addErrorSymbol();
	void addWarningSymbol();

};
