#pragma once
#include <vector>
#include <functional>
#include <memory>

class ShortcutManager {
public:
	void RegisterShortcut(std::vector<int> keys, std::function<void()> function, short id = 0);
	void UnregisterShortcut(short id);
	void DoShortcuts();
protected:
	class Shortcut {
	public:
		Shortcut(std::vector<int> keys, std::function<void()> function, short id);
		bool IsPressed();
		void Execute();
		short id = 0;
	protected:
		std::vector<int> keys;
		std::function<void()> function;
	};
	std::vector<std::shared_ptr<Shortcut>> shortcuts;
};