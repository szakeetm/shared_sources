#include "ImguiShortcutManager.h"
#include <memory>
#include "imgui.h"

void ShortcutManager::RegisterShortcut(std::vector<int> keys, std::function<void()>& function, short id)
{
	shortcuts.emplace_back(Shortcut(keys, function, id));
}

void ShortcutManager::UnregisterShortcut(short id)
{
	if (id == 0) return;
	//Iterating from back
	for (int i = shortcuts.size() - 1; i >= 0; --i) {
		if (shortcuts[i].id == id) {
			shortcuts.erase(shortcuts.begin() + i);
		}
	}
}

void ShortcutManager::DoShortcuts()
{
	for (auto& shortcut : this->shortcuts) {
		if (shortcut.IsPressed())
			shortcut.Execute();
	}
}

ShortcutManager::Shortcut::Shortcut(std::vector<int> keys, std::function<void()>& function, short id)
{
	this->keys = keys;
	this->function = function;
	this->id = id;
}

bool ShortcutManager::Shortcut::IsPressed()
{
	ImGuiIO& io = ImGui::GetIO();
	bool pressed = true;
	for (auto& key : keys) {
		pressed &= io.KeysDown[key];
	}
	return pressed;
}

void ShortcutManager::Shortcut::Execute()
{
	function();
}
