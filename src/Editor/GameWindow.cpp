#include "GameWindow.h"
#include "imgui_internal.h"

#include <array>

namespace Editor
{
	static auto renderSizePresets = std::to_array<RenderSizePreset>({
		{ "16:9 Aspect", RenderSizePresetType::AspectRatio, 16.0f, 9.0f },
		{ "Full HD (1920x1080)", RenderSizePresetType::FixedResolution, 1920.0f, 1080.0f },
	});

	GameWindow::GameWindow()
	{
		gameFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ 0, 0 });
		renderSizePreset = &renderSizePresets[0];
	}

	void GameWindow::draw(const EditorContext& context)
	{
		drawWindow(context);
	}

	void GameWindow::drawWindow(const EditorContext& context)
	{
		auto& editor = context.editor;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Game", NULL, ImGuiWindowFlags_MenuBar))
		{
			ImGui::SetNextItemWidth(200);
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginCombo("##BufferSizeCombo", renderSizePreset ? renderSizePreset->name.c_str() : "Free"))
				{
					if (ImGui::Selectable("Free", renderSizePreset == nullptr))
					{
						renderSizePreset = nullptr;
					}
					for (auto& size : renderSizePresets)
					{
						if (ImGui::Selectable(size.name.c_str(), renderSizePreset == &size))
						{
							renderSizePreset = &size;
						}
					}
					
					ImGui::EndCombo();
				}


				ImGui::EndMenuBar();
			}


			if (ImGui::IsWindowHovered() &&
				(ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
			{
				ImGui::SetWindowFocus();
			}

			if (ImGui::IsWindowFocused() && editor->playMode)
			{
				editor->input();
			}

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();
			ImVec2 framebufferSize, imageSize;

			if (renderSizePreset)
			{
				if (renderSizePreset->type == RenderSizePresetType::AspectRatio)
				{
					float framebufferRatio = renderSizePreset->x / renderSizePreset->y;
					float regionRatio = contentRegion.x / contentRegion.y;
					if (regionRatio > framebufferRatio)
					{
						framebufferSize.x = contentRegion.y * framebufferRatio;
						framebufferSize.y = contentRegion.y;
					}
					else
					{
						framebufferSize.x = contentRegion.x;
						framebufferSize.y = contentRegion.x / framebufferRatio;
					}
					imageSize = framebufferSize;
				}
				else
				{
					framebufferSize.x = renderSizePreset->x;
					framebufferSize.y = renderSizePreset->y;

					float regionRatio = contentRegion.x / contentRegion.y;
					float framebufferRatio = framebufferSize.x / framebufferSize.y;
					if (regionRatio > framebufferRatio)
					{
						imageSize.x = contentRegion.y * framebufferRatio;
						imageSize.y = contentRegion.y;
					}
					else
					{
						imageSize.x = contentRegion.x;
						imageSize.y = contentRegion.x / framebufferRatio;
					}
				}
			}
			else
			{
				framebufferSize = imageSize = contentRegion;
			}

			if (framebufferSize.x != lastSize.x || framebufferSize.y != lastSize.y)
			{
				gameFramebuffer->Resize(framebufferSize.x, framebufferSize.y);
				lastSize = framebufferSize;
			}
			editor->render(context.camera, *gameFramebuffer);
			GLuint texture = gameFramebuffer->GetColorTexture();

			if (renderSizePreset)
			{
				ImVec2 pos = ImGui::GetCursorPos();
				ImVec2 offset = ImVec2((contentRegion.x - imageSize.x) / 2.0f, (contentRegion.y - imageSize.y) / 2.0f);
				pos.x += offset.x;
				pos.y += offset.y;
				ImGui::SetCursorPos(pos);
			}
			ImGui::Image(texture, imageSize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}
