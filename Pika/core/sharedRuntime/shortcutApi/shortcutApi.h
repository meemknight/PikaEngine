#pragma once
#include <windowSystemm/input.h>
#include <unordered_map>

namespace pika
{

	struct ShortcutManager
	{
		struct Shortcut
		{
			std::string shortcut = "";
			bool *toggle = 0;
		};

		void update(const pika::Input &input);

		std::unordered_map<std::string, Shortcut> registeredShortcuts;

		bool registerShortcut(const char *name, const char *s, bool *toggle);

		const char *getShortcut(const char *name);
	};

bool shortcut(const pika::Input &input, const char *shortcut);

bool MenuItem(const pika::Input &input, const char *label, const char *shortcut, bool *p_selected, bool enabled = true);

std::string normalizeShortcutName(const char *shortcut);

void initShortcutApi();

}
