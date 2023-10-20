#pragma once
#include <vector>
#include <functional>
#include <memory>

class ShortcutManager {
public:
	void RegisterShortcut(std::vector<int> keys, std::function<void()>& function, short id = 0);
	void UnregisterShortcut(short id);
	void DoShortcuts();
protected:
	class Shortcut {
	public:
		Shortcut(std::vector<int> keys, std::function<void()>& function, short id);
		bool IsPressed();
		void Execute();
		// id allows for removing all shortcuts with a given id. Useful when recomputing shortcuts for memorized selections, views, structures etc.
		short id = 0;
	protected:
		std::vector<int> keys;
		std::function<void()> function;
	};
	std::vector<Shortcut> shortcuts;
};