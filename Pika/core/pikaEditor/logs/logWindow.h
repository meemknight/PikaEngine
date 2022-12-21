#pragma once
#include <logs/log.h>
#include <IconsForkAwesome.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>


#include <pikaConfig.h>
#if !PIKA_SHOULD_REMOVE_EDITOR


namespace pika
{


	struct LogWindow
	{

		void init(pika::pikaImgui::ImGuiIdsManager &idManager);

		void update(pika::LogManager &logManager, bool &open);

		static constexpr char *ICON = ICON_FK_COMMENT_O;
		static constexpr char *NAME = "logs";
		static constexpr char *ICON_NAME = ICON_FK_COMMENT_O " logs";
		bool autoScroll = true;
		ImGuiTextFilter filter;

		int imguiId = 0;
	};

}


#endif