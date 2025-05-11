#pragma once
#include "EditorApp.h"
#include "imgui.h"

namespace Editor
{
	enum class RenderSizePresetType
	{
		AspectRatio,
		FixedResolution,
	};
	struct RenderSizePreset
	{
		std::string name;
		RenderSizePresetType type;
		//float x, y;
		ImVec2 size;
	};

	class GameWindow
	{
	public:
		GameWindow();

		void draw(const EditorContext& context);


	private:
		void drawWindow(const EditorContext& context);

		std::unique_ptr<CustomFramebuffer> gameFramebuffer;
		RenderSizePreset* renderSizePreset = nullptr;
		ImVec2 lastSize = ImVec2(0, 0);
	};
}
