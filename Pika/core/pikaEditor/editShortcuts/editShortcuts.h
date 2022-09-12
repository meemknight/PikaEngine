#pragma once
#include <IconsForkAwesome.h>
#include <shortcutApi/shortcutApi.h>

namespace pika
{

	struct EditShortcutsWindow
	{


		void init();

		void update(pika::ShortcutManager &shortcutManager, bool &open);

		static constexpr char *ICON = ICON_FK_PENCIL_SQUARE;
		static constexpr char *NAME = "Edit Shortcuts...";
		static constexpr char *ICON_NAME = ICON_FK_PENCIL_SQUARE " Edit Shortcuts...";

	};





};