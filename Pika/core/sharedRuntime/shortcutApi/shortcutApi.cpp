#include "shortcutApi.h"
#include <stringManipulation/stringManipulation.h>
#include <iostream>
#include <unordered_map>
#include <set>
#include <imgui.h>

std::unordered_map<std::string, int> buttonMapping =
{
	{"a", pika::Button::A},
	{ "b", pika::Button::B },
	{ "c", pika::Button::C },
	{ "d", pika::Button::D },
	{ "e", pika::Button::E },
	{ "f", pika::Button::F },
	{ "g", pika::Button::G },
	{ "h", pika::Button::H },
	{ "i", pika::Button::I },
	{ "j", pika::Button::J },
	{ "k", pika::Button::K },
	{ "l", pika::Button::L },
	{ "m", pika::Button::M },
	{ "n", pika::Button::N },
	{ "o", pika::Button::O },
	{ "p", pika::Button::P },
	{ "q", pika::Button::Q },
	{ "r", pika::Button::R },
	{ "s", pika::Button::S },
	{ "t", pika::Button::T },
	{ "u", pika::Button::U },
	{ "v", pika::Button::V },
	{ "w", pika::Button::W },
	{ "x", pika::Button::X },
	{ "y", pika::Button::Y },
	{ "z", pika::Button::Z },
	{ "0", pika::Button::NR0 }, { "1", pika::Button::NR1 }, { "2", pika::Button::NR2 }, { "3", pika::Button::NR3 },
	{ "4", pika::Button::NR4 }, { "5", pika::Button::NR5 }, { "6", pika::Button::NR6 }, { "7", pika::Button::NR7 },
	{ "8", pika::Button::NR8 }, { "9", pika::Button::NR9 },
	{ "space", pika::Button::Space },
	{ "enter", pika::Button::Enter },
	{ "escape", pika::Button::Escape },
	{ "up", pika::Button::Up },
	{ "down", pika::Button::Down },
	{ "left", pika::Button::Left },
	{ "right", pika::Button::Right },
	{ "ctrl", pika::Button::LeftCtrl },
	{ "tab", pika::Button::Tab },

};




namespace pika
{

std::vector<std::string> tokenizeShortcutSimple(const char *shortcut)
{
	char data[256] = {};
	pika::removeCharacters(data, shortcut, "\n \t\r\v", sizeof(data));
	pika::toLower(data, data, sizeof(data));

	auto token = pika::split(data, '+');

	return token;
};


std::vector<std::string> tokenizeShortcutNormalized(const char *shortcut)
{
	
	auto token = tokenizeShortcutSimple(shortcut);

	std::set<std::string> tokenSet;

	for (auto &i : token)
	{

		auto it = buttonMapping.find(i);

		if (it != buttonMapping.end())
		{
			tokenSet.insert(it->first);
		}
	}

	std::vector<std::string> ret;
	ret.reserve(tokenSet.size());

	for (auto &i : tokenSet)
	{
		ret.push_back(std::move(i));
	}

	return ret;

}

std::string normalizeShortcutName(const char *shortcut)
{
	auto t = tokenizeShortcutNormalized(shortcut);

	std::string ret = "";
	for (int i = 0; i < t.size(); i++)
	{
		ret += t[i];

		if (i < t.size())
		{
			ret += "+";
		}
	}
	
	return ret;
}

bool shortcut(const pika::Input &input, const char *shortcut)
{
	
	auto token = tokenizeShortcutSimple(shortcut);

	bool pressed = false;

	for (auto &t : token)
	{

		auto it = buttonMapping.find(t);

		if (it != buttonMapping.end())
		{
			if (input.buttons[it->second].pressed())
			{
				pressed = true;
			}
			else if (!input.buttons[it->second].held())
			{
				return false;
			}
		}
	}

	return pressed;
}

bool MenuItem(const pika::Input &input, const char *label, const char *shortcut, bool *p_selected, bool enabled)
{
	if (pika::shortcut(input, shortcut))
	{
		*p_selected = !*p_selected;
	}

	return ImGui::MenuItem(label, shortcut, p_selected, enabled);
}



void ShortcutManager::update(const pika::Input &input)
{
	for (auto &i : registeredShortcuts)
	{
		if (shortcut(input, i.first.c_str()))
		{
			*i.second = !*i.second;
		}
	}


}

void ShortcutManager::registerShortcut(const char *s, bool *toggle)
{

	auto normalized = normalizeShortcutName(s);

	registeredShortcuts[normalized] = toggle;

}

}
