#pragma once
#include <IconsForkAwesome.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>
#include <unordered_map>

namespace pika
{


	struct OpenglLogsWindow
	{

		void init(pika::pikaImgui::ImGuiIdsManager &idManager);

		void update(bool &open);

		static constexpr char *ICON = ICON_FK_EXCLAMATION_TRIANGLE;
		static constexpr char *NAME = "opengl errors";
		static constexpr char *ICON_NAME = ICON_FK_EXCLAMATION_TRIANGLE " opengl errors";

		std::unordered_map<unsigned int, unsigned int> errorsReported;

		int imguiId = 0;
	};

}
