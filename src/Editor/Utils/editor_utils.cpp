#include "editor_utils.h"

#include "imgui.h"
#include "imgui_internal.h"

#ifdef _WIN32
#include <ShlObj.h>
#include <Shlwapi.h>
#endif

#include "Scene.h"

#include <filesystem>

namespace Editor::Utils
{

	void wrapCursor(ImVec2 pos, ImVec2 size)
	{
		ImVec2 mousePos = ImGui::GetMousePos();

		float relX = mousePos.x - pos.x;
		float relY = mousePos.y - pos.y;

		if (relX < 0.0f || relX >= size.x || relY < 0.0f || relY >= size.y)
		{
			relX = fmodf(relX + size.x, size.x);
			relY = fmodf(relY + size.y, size.y);


			mousePos.x = pos.x + relX;
			mousePos.y = pos.y + relY;

			ImGui::TeleportMousePos(mousePos);
		}
	}

	bool isActiveInAnotherWindow()
	{
		return ImGui::IsAnyItemActive() && !ImGui::IsWindowFocused();
	}

	void setUniqueName(int id, Scene& scene, std::string name)
	{
		std::string* objName;

		bool newName = !name.empty();
		if (newName)
			objName = &name;
		else
			objName = &scene.getComponent<ObjectInfoComponent>(id).name;



		std::string baseName = *objName;
		auto leftParenthesisPos = baseName.rfind(" (");
		auto rightParenthesisPos = baseName.rfind(')');
		if (leftParenthesisPos != std::string::npos && rightParenthesisPos != std::string::npos)
		{
			std::string indexStr = baseName.substr(leftParenthesisPos + 2, rightParenthesisPos - leftParenthesisPos - 2);

			if (std::all_of(indexStr.begin(), indexStr.end(), ::isdigit))
			{
				baseName = baseName.substr(0, leftParenthesisPos);
			}
		}

		std::unordered_set<int> indices;

		auto& parentTransform = scene.getComponent<Transform>(scene.getComponent<Transform>(id).parent);
		for (auto& child : parentTransform.children)
		{
			if (child == id) continue;

			auto& childName = scene.getComponent<ObjectInfoComponent>(child).name;
			if (childName.find(baseName) == 0)
			{
				if (childName.length() == baseName.length())
				{
					indices.insert(0);
				}
				else if (childName.find(baseName + " (") == 0)
				{
					auto leftParenthesisPos = childName.rfind(" (");
					auto rightParenthesisPos = childName.rfind(')');
					if (leftParenthesisPos == baseName.length() && rightParenthesisPos != std::string::npos) {
						std::string indexStr = childName.substr(leftParenthesisPos + 2, rightParenthesisPos - leftParenthesisPos - 2);
						if (std::all_of(indexStr.begin(), indexStr.end(), ::isdigit))
						{
							indices.insert(std::stoi(indexStr));
						}
					}
				}
			}
		}

		int lowestAvailableIndex = 0;
		while (indices.find(lowestAvailableIndex) != indices.end())
		{
			lowestAvailableIndex++;
		}

		if (lowestAvailableIndex > 0)
		{
			*objName = baseName + " (" + std::to_string(lowestAvailableIndex) + ")";
		}
		else
		{
			*objName = baseName;
		}

		if (newName)
		{
			scene.getComponent<ObjectInfoComponent>(id).name = *objName;
		}
	}

	void openFolder(const std::string& path) {
		#if defined(_WIN32)
			ShellExecute(nullptr, "open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		#elif defined(__APPLE__)
			std::string cmd = "open \"" + path + "\"";
			system(cmd.c_str());
		#elif defined(__linux__)
			std::string cmd = "xdg-open \"" + path + "\"";
			system(cmd.c_str());
		#endif
	}

	std::optional<std::string> openSaveDialog(const std::vector<nfdu8filteritem_t>& filters,
		const std::string& defaultPath)
	{
		nfdu8char_t* outPath;
		nfdsavedialogu8args_t args = { 0 };
		args.filterList = filters.data();
		args.filterCount = filters.size();
		std::string absoluteDefaultPath = std::filesystem::absolute(defaultPath).string();
		args.defaultPath = absoluteDefaultPath.c_str();
		nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
		if (result == NFD_OKAY)
		{
			std::string path = outPath;
			NFD_FreePathU8(outPath);
			return path;
		}
		else
		{
			return std::nullopt;
		}
	}

	std::optional<std::string> openLoadDialog(const std::vector<nfdu8filteritem_t>& filters,
		const std::string& defaultPath)
	{
		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };
		args.filterList = filters.data();
		args.filterCount = filters.size();
		std::string absoluteDefaultPath = std::filesystem::absolute(defaultPath).string();
		args.defaultPath = absoluteDefaultPath.c_str();
		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
		if (result == NFD_OKAY)
		{
			std::string path = outPath;
			NFD_FreePathU8(outPath);
			return path;
		}
		else
		{
			return std::nullopt;
		}
	}
}