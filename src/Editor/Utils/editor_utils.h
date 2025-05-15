#pragma once
#include <string>
#include "imgui.h"

class Scene;

namespace Editor::Utils
{
	void wrapCursor(ImVec2 pos, ImVec2 size);
	bool isActiveInAnotherWindow();
	void setUniqueName(int id, Scene& scene, std::string name = "");

	void openFolder(const std::string& path);
}