#pragma once

#include "Application.h"


class EditorApp : public Application
{
public:
	EditorApp();
	~EditorApp();
	void run() override;

protected:
	void initImGui();

	void createImGuiDrawData();
	void renderImGui();

	std::unique_ptr<CustomFramebuffer> sceneFramebuffer;
	void imguiScene();
};