#include "ImguiShortcutManager.h"
#include <memory>
#include "imgui.h"

void ShortcutManager::RegisterShortcut(std::vector<int> keys, std::function<void()> function, short id)
{
	shortcuts.emplace_back(std::make_unique<Shortcut>(keys, function, id));
}

void ShortcutManager::UnregisterShortcut(short id)
{
	if (id == 0) return;
	for (int i = 0; i < shortcuts.size();i++) {
		if (shortcuts[i]->id == id) {
			shortcuts.erase(std::find(shortcuts.begin(), shortcuts.end(), shortcuts[i]));
			i = 0;
		}
	}
}

void ShortcutManager::DoShortcuts()
{
	for (int i = 0; i < shortcuts.size(); i++) {
		if (shortcuts[i]->IsPressed())
			shortcuts[i]->Execute();
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
