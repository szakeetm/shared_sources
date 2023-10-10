#include "ImguiShortcutManager.h"
#include <memory>
#include "imgui.h"

void ShortcutManager::RegisterShortcut(std::vector<int> keys, std::function<void()> function, short id)
{
	shortcuts.push_back(std::make_shared<Shortcut>(keys, function, id));
}

void ShortcutManager::UnregisterShortcut(short id)
{
	if (id == 0) return;
	for (int i = 0; i < shortcuts.size();i++) {
		if (shortcuts.at(i)->id == id) {
			shortcuts.erase(std::find(shortcuts.begin(), shortcuts.end(), shortcuts.at(i)));
		}
	}
}

void ShortcutManager::DoShortcuts()
{
	for (auto shortcut : this->shortcuts) {
		if (shortcut->IsPressed())
			shortcut->Execute();
	}
}

ShortcutManager::Shortcut::Shortcut(std::vector<int> keys, std::function<void()> function, short id)
{
	this->keys = keys;
	this->function = function;
	this->id = id;
}

bool ShortcutManager::Shortcut::IsPressed()
{
	ImGuiIO& io = ImGui::GetIO();
	bool pressed = true;
	for (auto key : keys) {
		pressed &= io.KeysDown[key];
	}
	return pressed;
}

void ShortcutManager::Shortcut::Execute()
{
	function();
}
