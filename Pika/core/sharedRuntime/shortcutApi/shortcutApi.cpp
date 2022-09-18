#include "shortcutApi.h"
#include <stringManipulation/stringManipulation.h>
#include <iostream>
#include <unordered_map>
#include <set>
#include <imgui.h>
#include <imgui.h>

struct Mapping
{
	short normal = 0;
	short imgui = 0;
};

//todo remove global things that allocate memory
std::unordered_map<std::string, Mapping> buttonMapping;

void pika::initShortcutApi()
{
	buttonMapping =
	{
		{"a", {pika::Button::A, ImGuiKey_A}},
		{ "b", {pika::Button::B, ImGuiKey_B} },
		{ "c", {pika::Button::C, ImGuiKey_C} },
		{ "d", {pika::Button::D, ImGuiKey_D} },
		{ "e", {pika::Button::E, ImGuiKey_E} },
		{ "f", {pika::Button::F, ImGuiKey_F} },
		{ "g", {pika::Button::G, ImGuiKey_G} },
		{ "h", {pika::Button::H, ImGuiKey_H} },
		{ "i", {pika::Button::I, ImGuiKey_I} },
		{ "j", {pika::Button::J, ImGuiKey_J} },
		{ "k", {pika::Button::K, ImGuiKey_K} },
		{ "l", {pika::Button::L, ImGuiKey_L} },
		{ "m", {pika::Button::M, ImGuiKey_M} },
		{ "n", {pika::Button::N, ImGuiKey_N} },
		{ "o", {pika::Button::O, ImGuiKey_O} },
		{ "p", {pika::Button::P, ImGuiKey_P} },
		{ "q", {pika::Button::Q, ImGuiKey_Q} },
		{ "r", {pika::Button::R, ImGuiKey_R} },
		{ "s", {pika::Button::S, ImGuiKey_S} },
		{ "t", {pika::Button::T, ImGuiKey_T} },
		{ "u", {pika::Button::U, ImGuiKey_U} },
		{ "v", {pika::Button::V, ImGuiKey_V} },
		{ "w", {pika::Button::W, ImGuiKey_W} },
		{ "x", {pika::Button::X, ImGuiKey_X} },
		{ "y", {pika::Button::Y, ImGuiKey_Y} },
		{ "z", {pika::Button::Z, ImGuiKey_Z} },
		{ "0", {pika::Button::NR0, ImGuiKey_0}}, { "1", {pika::Button::NR1, ImGuiKey_1} }, { "2", {pika::Button::NR2, ImGuiKey_2} }, { "3", {pika::Button::NR3, ImGuiKey_3} },
		{ "4", {pika::Button::NR4, ImGuiKey_0}}, { "5", {pika::Button::NR5, ImGuiKey_5} }, { "6", {pika::Button::NR6, ImGuiKey_6} }, { "7", {pika::Button::NR7, ImGuiKey_7} },
		{ "8", {pika::Button::NR8, ImGuiKey_8}}, { "9", {pika::Button::NR9, ImGuiKey_9} },
		{ "space", {pika::Button::Space , ImGuiKey_Space}},
		{ "enter", {pika::Button::Enter, ImGuiKey_Enter} },
		{ "escape", {pika::Button::Escape, ImGuiKey_Escape} },
		{ "up", {pika::Button::Up, ImGuiKey_UpArrow} },
		{ "down", {pika::Button::Down , ImGuiKey_DownArrow}},
		{ "left", {pika::Button::Left , ImGuiKey_LeftArrow}},
		{ "right", {pika::Button::Right , ImGuiKey_RightArrow}},
		{ "ctrl", {pika::Button::LeftCtrl , ImGuiKey_LeftCtrl}},
		{ "tab", {pika::Button::Tab , ImGuiKey_Tab}},
		{ "alt", {pika::Button::LeftAlt , ImGuiKey_LeftAlt}},

	};

}



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
		t[i][0] = std::toupper(t[i][0]);

		ret += t[i];

		if (i < t.size()-1)
		{
			ret += "+";
		}
	}
	
	return ret;
}



//todo shortcut should rely on glfw backend when imgui is disabeled in production build
bool shortcut(const pika::Input &input, const char *shortcut)
{
	auto token = tokenizeShortcutSimple(shortcut);
	
	if (token.empty()) { return 0; }


	bool pressed = false;

	if (0)
	{	//noraml implementation
		for (auto &t : token)
		{
			auto it = buttonMapping.find(t);
			if (it != buttonMapping.end())
			{
				if (input.buttons[it->second.normal].pressed())
				{
					pressed = true;
				}
				else if (!input.buttons[it->second.normal].held())
				{
					return false;
				}
			}
		}
	}
	else
	{	//imgui backend
		for (auto &t : token)
		{
			auto it = buttonMapping.find(t);
			if (it != buttonMapping.end())
			{
				if (ImGui::IsKeyPressed(it->second.imgui, false))
				{
					pressed = true;
				}
				else if (!ImGui::IsKeyDown(it->second.imgui))
				{
					return false;
				}
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
		if (shortcut(input, i.second.shortcut.c_str()))
		{
			*i.second.toggle = !*i.second.toggle;
		}
	}

}

bool ShortcutManager::registerShortcut(const char *name, const char *s, bool *toggle)
{

	if (registeredShortcuts.find(name) != registeredShortcuts.end())
	{
		//todo log error
		return 0;
	}
	else
	{
		registeredShortcuts[name] 
			= Shortcut{std::move(normalizeShortcutName(s)), toggle};
		return 1;
	}

}

const char *ShortcutManager::getShortcut(const char *name)
{
	auto it = registeredShortcuts.find(name);

	if (it == registeredShortcuts.end()) { return ""; }
	else 
	{ return it->second.shortcut.c_str(); };

}

}
