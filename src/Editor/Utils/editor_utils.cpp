#include "editor_utils.h"

#include "imgui.h"
#include "imgui_internal.h"

#ifdef _WIN32
#include <ShlObj.h>
#include <Shlwapi.h>
#endif

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
}