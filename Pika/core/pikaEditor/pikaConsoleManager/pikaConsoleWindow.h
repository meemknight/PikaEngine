#pragma once
#include <pikaImgui/pikaImgui.h>
#include <logs/log.h>

namespace pika
{

	struct ConsoleWindow
	{

		void init(pika::pikaImgui::ImGuiIdsManager &idManager);

		void update(pika::LogManager &logManager, bool &open);

		static constexpr char *ICON = ICON_FK_TERMINAL;
		static constexpr char *NAME = "console";
		static constexpr char *ICON_NAME = ICON_FK_TERMINAL " console";

		int imguiId = 0;

		std::string s;
	};


};