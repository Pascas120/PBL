#include "editor_utils.h"

#include "imgui.h"

#ifdef _WIN32
#include <ShlObj.h>
#include <Shlwapi.h>
#endif

namespace Editor::Utils
{

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