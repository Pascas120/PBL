#pragma once
#include <string>
#include "imgui.h"

namespace Editor::Utils
{
	void wrapCursor(ImVec2 pos, ImVec2 size);
	bool isActiveInAnotherWindow();

	void openFolder(const std::string& path);
}