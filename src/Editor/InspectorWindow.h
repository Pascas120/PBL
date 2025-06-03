#pragma once
#include "EditorApp.h"

namespace Editor
{
	class InspectorWindow
	{
	public:
		InspectorWindow(EditorApp* editor) : editor(editor) {}
		void draw(const EditorContext& context);

	private:
		EditorApp* editor = nullptr;

		void drawWindow(const EditorContext& context);
		void drawTransform(const EditorContext& context, EntityID id);
		void drawCollider(const EditorContext& context, EntityID id);
		void drawModel(const EditorContext& context, EntityID id);

		void drawCamera(const EditorContext& context, EntityID id);

		void drawPointLight(const EditorContext& context, EntityID id);
		void drawDirectionalLight(const EditorContext& context, EntityID id);

		void drawFlyAi(const EditorContext& context, EntityID id);
		void drawFreeze(const EditorContext& context, EntityID id);
		void drawHeat(const EditorContext& context, EntityID id);
		void drawVelocity(const EditorContext& context, EntityID id);
		void drawRegen(const EditorContext& context, EntityID id);
		void drawElevator(const EditorContext& context, EntityID id);
		void drawButton(const EditorContext& context, EntityID id);
		void drawCameraController(const EditorContext& context, EntityID id);
	};
}