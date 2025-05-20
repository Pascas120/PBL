#include "GameWindow.h"
#include "imgui_internal.h"

#include <array>

namespace Editor
{
	static auto renderSizePresets = std::to_array<RenderSizePreset>({
		{ "16:9 Aspect", RenderSizePresetType::AspectRatio, {16.0f, 9.0f} },
		{ "Full HD (1920x1080)", RenderSizePresetType::FixedResolution, {1920.0f, 1080.0f} },
	});

	static ImVec2 fitViewInContentRegion(const ImVec2& contentRegion, const ImVec2& framebufferSize)
	{
		float framebufferRatio = framebufferSize.x / framebufferSize.y;
		float regionRatio = contentRegion.x / contentRegion.y;
		if (regionRatio > framebufferRatio)
		{
			return ImVec2(contentRegion.y * framebufferRatio, contentRegion.y);
		}
		else
		{
			return ImVec2(contentRegion.x, contentRegion.x / framebufferRatio);
		}
	}

	GameWindow::GameWindow(EditorApp* editor) : editor(editor)
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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImVec4 bgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		bgColor.w = 1.0f;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);

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

			if (ImGui::IsWindowFocused() && editor->playMode == PlayMode::PLAY)
			{
				editor->input();
			}

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();
			ImVec2 framebufferSize, imageSize;

			if (renderSizePreset)
			{
				if (renderSizePreset->type == RenderSizePresetType::AspectRatio)
				{
					framebufferSize = fitViewInContentRegion(contentRegion, renderSizePreset->size);
					imageSize = framebufferSize;
				}
				else
				{
					framebufferSize = renderSizePreset->size;
					imageSize = fitViewInContentRegion(contentRegion, framebufferSize);
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
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
}
