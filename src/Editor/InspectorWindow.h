#pragma once
#include "EditorApp.h"

namespace Editor
{
	class InspectorWindow
	{
	public:
		void draw(const EditorContext& context);

	private:
		void drawWindow(const EditorContext& context);
		void drawTransform(const EditorContext& context, EntityID id);
		void drawCollider(const EditorContext& context, EntityID id);
		void drawModel(const EditorContext& context, EntityID id);
	};
}