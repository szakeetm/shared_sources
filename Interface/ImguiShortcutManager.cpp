#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "ImguiShortcutManager.h"
#include <memory>

void ShortcutManager::RegisterShortcut(const std::vector<int>& keys_, const std::function<void()>& function_, short id_)
{
	shortcuts.emplace_back(Shortcut(keys_, function_, id_));
}

void ShortcutManager::UnregisterShortcut(const short id_)
{
	for (int i = static_cast<int>(shortcuts.size())-1; i >= 0; --i) {
		if (shortcuts[i].id == id_) {
			shortcuts.erase(shortcuts.begin()+i);
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

ShortcutManager::Shortcut::Shortcut(const std::vector<int>& keys_, const std::function<void()>& function_, short id_)
{
	this->keys = keys_;
	this->function = function_;
	this->id = id_;
}

const bool ShortcutManager::Shortcut::IsPressed()
{
	ImGuiIO& io = ImGui::GetIO();
	bool pressed = true;
	for (const auto& key : keys) {
		pressed &= io.KeysDown[key];
	}
	return pressed;
}

void ShortcutManager::Shortcut::Execute()
{
	function();
}
