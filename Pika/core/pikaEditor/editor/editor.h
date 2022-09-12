#pragma once

#include <pikaImgui/pikaImgui.h>
#include <logs/logWindow.h>
#include <windowSystemm/input.h>
#include <shortcutApi/shortcutApi.h>

namespace pika
{

	struct Editor
	{

		void init(pika::ShortcutManager &shortcutManager);

		void update(const pika::Input &input);

		struct
		{
			bool dockMainWindow = 1;
		}optionsFlags;

		struct
		{
			bool logsWindow;
		}windowFlags;

	};



}