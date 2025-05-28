#pragma once
#include "EditorApp.h"

namespace Editor
{
	class HierarchyWindow
	{
	public:
		HierarchyWindow(EditorApp* editor) : editor(editor) {}
		void draw(const EditorContext& context);

	private:
		EditorApp* editor = nullptr;
		EntityID clickedItem = (EntityID)-1;

		void drawWindow(const EditorContext& context);
		void drawNode(const EditorContext& context, EntityID id);
		void drawRearrangeTarget(const EditorContext& context, EntityID id, int targetIndex);
	};
}