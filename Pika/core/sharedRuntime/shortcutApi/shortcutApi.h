#pragma once
#include <windowSystemm/input.h>
#include <unordered_map>

namespace pika
{

	struct ShortcutManager
	{

		void update(const pika::Input &input);

		std::unordered_map<std::string, bool *> registeredShortcuts;

		void registerShortcut(const char *s, bool *toggle);
	};


bool shortcut(const pika::Input &input, const char *shortcut);

bool MenuItem(const pika::Input &input, const char *label, const char *shortcut, bool *p_selected, bool enabled = true);


}
