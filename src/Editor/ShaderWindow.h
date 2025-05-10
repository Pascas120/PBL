#pragma once
#include "EditorApp.h"

namespace Editor
{
	class ShaderWindow
	{
	public:
		void draw(const EditorContext& context);

	private:
		void drawWindow(const EditorContext& context);
		void drawUniformDescendants(const std::vector<UniformInfo>& uniforms, const std::string& fullName);
		void drawUniformArray(const UniformInfo& uniform, const std::string& fullName);
		void drawUniformStruct(const UniformInfo& uniform, const std::string& fullName, int index);
		void drawUniformLeaf(const UniformInfo& uniform, const std::string& fullName, int index);

		Shader* selectedShader = nullptr;
	};
}