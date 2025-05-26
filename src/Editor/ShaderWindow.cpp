#include "ShaderWindow.h"
#include "EditorApp.h"

#include "imgui.h"
#include "imgui_internal.h"


namespace Editor
{
	void ShaderWindow::draw(const EditorContext& context)
	{
		drawWindow(context);
	}


	void ShaderWindow::drawWindow(const EditorContext& context)
	{
        auto& shaders = context.shaders;

        ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
        ImGui::Begin("Shaders");

        if (selectedShader == nullptr)
        {
            if (shaders.size() > 0)
            {
                selectedShader = shaders[0];
            }
            else
            {
                ImGui::End();
                return;
            }
        }

        if (ImGui::BeginCombo("Shader##Combo", selectedShader->getName().c_str()))
        {
            for (int i = 0; i < shaders.size(); i++)
            {
                if (ImGui::Selectable(shaders[i]->getName().c_str(), selectedShader == shaders[i]))
                {
                    selectedShader = shaders[i];
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Dummy(ImVec2(0, 15));
        selectedShader->use();

        drawUniformDescendants(selectedShader->getUniforms(), "");

        ImGui::End();
	}

    void ShaderWindow::drawUniformDescendants(const std::vector<UniformInfo>& uniforms, const std::string& fullName)
    {
        for (auto& uniform : uniforms)
        {
            if (uniform.size > 1)
                drawUniformArray(uniform, fullName + uniform.name);
            else if (uniform.members.size() > 0)
                drawUniformStruct(uniform, fullName + uniform.name, -1);
            else
                drawUniformLeaf(uniform, fullName + uniform.name, -1);

        }
        ImGui::Dummy(ImVec2(0, 10));
    }

    void ShaderWindow::drawUniformArray(const UniformInfo& uniform, const std::string& fullName)
    {

        std::string displayString = uniform.name + "[]";
        ImGui::PushID(fullName.c_str());
        if (ImGui::TreeNodeEx(displayString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_FramePadding))
        {
            for (int i = 0; i < uniform.size; i++)
            {
                std::string uniformName = fullName + "[" + std::to_string(i) + "]";

                if (uniform.members.empty())
                    drawUniformLeaf(uniform, uniformName, i);
                else
                    drawUniformStruct(uniform, uniformName, i);

                ImGui::Spacing();
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void ShaderWindow::drawUniformStruct(const UniformInfo& uniform, const std::string& fullName, int index)
    {
        std::string displayString = uniform.name;
        if (index >= 0)
        {
            displayString += "[" + std::to_string(index) + "]";
        }
        ImGui::PushID(fullName.c_str());

        if (ImGui::TreeNodeEx(displayString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_FramePadding))
        {
            drawUniformDescendants(uniform.members, fullName + '.');
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void ShaderWindow::drawUniformLeaf(const UniformInfo& uniform, const std::string& fullName, int index)
    {
        ImGui::PushID(fullName.c_str());

        GLuint location = glGetUniformLocation(selectedShader->ID, fullName.c_str());
        if (location == -1)
            ImGui::BeginDisabled();

        std::string label;
        bool treeOpen = false;
        if (index >= 0)
        {
            label = std::to_string(index);
        }
        else
        {
            label = uniform.name;
        }

        bool changed = false;
        switch (uniform.type)
        {
        case GL_FLOAT:
            GLfloat floatValue;
            glGetUniformfv(selectedShader->ID, location, &floatValue);
            if (ImGui::DragFloat(label.c_str(), &floatValue, 0.01f))
            {
                selectedShader->setFloat(fullName, floatValue);
            }
            break;
        case GL_INT:
            GLint intValue;
            glGetUniformiv(selectedShader->ID, location, &intValue);
            if (ImGui::InputInt(label.c_str(), &intValue))
            {
                selectedShader->setInt(fullName, intValue);
            }
            break;
        case GL_BOOL:
            GLint boolValue;
            glGetUniformiv(selectedShader->ID, location, &boolValue);
            if (ImGui::Checkbox(label.c_str(), (bool*)&boolValue))
            {
                selectedShader->setBool(fullName, boolValue);
            }
            break;
        case GL_FLOAT_VEC2:
            glm::vec2 vec2Value;
            glGetUniformfv(selectedShader->ID, location, &vec2Value[0]);
            if (ImGui::DragFloat2(label.c_str(), &vec2Value[0], 0.01f))
            {
                selectedShader->setVec2(fullName, vec2Value);
            }
            break;
        case GL_FLOAT_VEC3:
            glm::vec3 vec3Value;
            glGetUniformfv(selectedShader->ID, location, &vec3Value[0]);
            if (ImGui::DragFloat3(label.c_str(), &vec3Value[0], 0.01f))
            {
                selectedShader->setVec3(fullName, vec3Value);
            }
            break;
        case GL_FLOAT_VEC4:
            glm::vec4 vec4Value;
            glGetUniformfv(selectedShader->ID, location, &vec4Value[0]);
            if (ImGui::DragFloat4(label.c_str(), &vec4Value[0], 0.01f))
            {
                selectedShader->setVec4(fullName, vec4Value);
            }
            break;
        case GL_FLOAT_MAT2:
            glm::mat2 mat2Value;
            glGetUniformfv(selectedShader->ID, location, &mat2Value[0][0]);

            changed = false;
            for (int i = 0; i < 2; ++i)
            {
                changed |= ImGui::DragFloat2((label + "##" + std::to_string(i)).c_str(), &mat2Value[i][0], 0.01f);
                label.clear();
            }
            if (changed)
            {
                selectedShader->setMat2(fullName, mat2Value);
            }
            break;
        case GL_FLOAT_MAT3:
            glm::mat3 mat3Value;
            glGetUniformfv(selectedShader->ID, location, &mat3Value[0][0]);

            changed = false;
            for (int i = 0; i < 3; ++i)
            {
                changed |= ImGui::DragFloat3((label + "##" + std::to_string(i)).c_str(), &mat3Value[i][0], 0.01f);
                label.clear();
            }
            if (changed)
            {
                selectedShader->setMat3(fullName, mat3Value);
            }
            break;
        case GL_FLOAT_MAT4:
            glm::mat4 mat4Value;
            glGetUniformfv(selectedShader->ID, location, &mat4Value[0][0]);

            changed = false;
            for (int i = 0; i < 4; ++i)
            {
                changed |= ImGui::DragFloat4((label + "##" + std::to_string(i)).c_str(), &mat4Value[i][0], 0.01f);
                label.clear();
            }
            if (changed)
            {
                selectedShader->setMat4(fullName, mat4Value);
            }
            break;
        default:
            ImGui::TreeNodeEx(uniform.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
                | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            break;
        }

        if (location == -1)
            ImGui::EndDisabled();
        ImGui::PopID();
    }

}