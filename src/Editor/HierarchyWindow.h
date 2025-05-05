#include "EditorApp.h"

namespace Editor
{
	class HierarchyWindow
	{
	public:
		void draw(const EditorContext& context);

	private:
		void drawWindow(const EditorContext& context);
		void drawNode(const EditorContext& context, EntityID id);
		void drawRearrangeTarget(const EditorContext& context, EntityID id, int targetIndex);
	};
}