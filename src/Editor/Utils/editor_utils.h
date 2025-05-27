#pragma once
#include <string>
#include <optional>
#include <vector>
#include <nfd.h>
#include "imgui.h"

#include "ECS/EntityManager.h"
class Scene;

namespace Editor::Utils
{
	void wrapCursor(ImVec2 pos, ImVec2 size);
	bool isActiveInAnotherWindow();
	void setUniqueName(int id, Scene& scene, std::string name = "");

	void openFolder(const std::string& path);

	std::optional<std::string> openSaveDialog(const std::vector<nfdu8filteritem_t>& filters,
		const std::string& defaultPath = "");

	std::optional<std::string> openLoadDialog(const std::vector<nfdu8filteritem_t>& filters,
		const std::string& defaultPath = "");

	bool entityRefField(const std::string& label, EntityID& entityId, Scene& scene);
}