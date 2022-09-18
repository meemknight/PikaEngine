#pragma once

#include <pikaImgui/pikaImgui.h>
#include <logs/logWindow.h>
#include <windowSystemm/input.h>
#include <shortcutApi/shortcutApi.h>
#include <editShortcuts/editShortcuts.h>
#include <pushNotification/pushNotification.h>

namespace pika
{

	struct Editor
	{

		void init(pika::ShortcutManager &shortcutManager);

		void update(const pika::Input &input, pika::ShortcutManager &shortcutManager
			,pika::LogManager &logs);

		struct
		{
			bool dockMainWindow = 1;
		}optionsFlags;

		struct
		{
			bool logsWindow = 0;
			bool editShortcutsWindow = 0;
		}windowFlags;

		pika::LogWindow logWindow;
		pika::EditShortcutsWindow editShortcutsWindow;
		pika::PushNotificationManager pushNotificationManager;
	};



}