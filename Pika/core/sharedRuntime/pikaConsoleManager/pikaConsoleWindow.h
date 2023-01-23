#pragma once

#include <pikaConfig.h>

#include <pikaImgui/pikaImgui.h>
#include <logs/log.h>
#include <pikaSizes.h>

namespace pika
{

	struct ConsoleWindow
	{

		void init(pika::pikaImgui::ImGuiIdsManager &idManager);

		void update(bool &open);

		static constexpr char *ICON = ICON_FK_TERMINAL;
		static constexpr char *NAME = "console";
		static constexpr char *ICON_NAME = ICON_FK_TERMINAL " console";

		int imguiId = 0;

		constexpr static const size_t BUFFER_SIZE = 3000;

		char buffer[BUFFER_SIZE+1] = {};
		size_t bufferBeginPos = 0;

		bool wrapped = 0;

		void write(const char *c);
	};


};

