#include "InspectorWindow.h"
#include "EditorApp.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace Editor
{
    void InspectorWindow::draw(const EditorContext& context)
    {
        drawWindow(context);
    }

	void InspectorWindow::drawWindow(const EditorContext& context)
	{
        auto& editor = context.editor;
        auto& scene = context.scene;
        ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Inspector"))
        {
            if (editor->selectedObject != (EntityID)-1)
            {
                assert(scene->hasComponent<ObjectInfoComponent>(editor->selectedObject));
                auto& objectInfo = scene->getComponent<ObjectInfoComponent>(editor->selectedObject);
                std::string objName = objectInfo.name;
                std::string displayName = objName + "###objName";
                const char* name = objName.c_str();

                if (ImGui::InputText("Name", (char*)name, 64))
                {
                    objectInfo.name = name;
                }

                drawTransform(context, editor->selectedObject);
                drawCollider(context, editor->selectedObject);
                drawModel(context, editor->selectedObject);
            }
        }
        ImGui::End();
	}


	void InspectorWindow::drawTransform(const EditorContext& context, EntityID id)
	{
        auto& editor = context.editor;
        auto& scene = context.scene;
        if (!scene->hasComponent<Transform>(id))
            return;

        auto& ts = scene->getTransformSystem();
        auto& transform = scene->getComponent<Transform>(id);

        ImGui::PushID(&transform);

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 translation = transform.translation;
            glm::vec3 rotation = transform.eulerRotation;
            glm::vec3 scale = transform.scale;

            if (ImGui::DragFloat3("Position", &translation[0], 0.1f))
            {
                ts.translateEntity(id, translation);
            }
            if (ImGui::DragFloat3("Rotation", &rotation[0], 0.1f))
            {
                ts.rotateEntity(id, rotation);
            }
            if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
            {
                ts.scaleEntity(id, scale);
            }
        }

        ImGui::PopID();
	}


	void InspectorWindow::drawCollider(const EditorContext& context, EntityID id)
	{
        auto& editor = context.editor;
        auto& scene = context.scene;
        if (!scene->hasComponent<ColliderComponent>(id))
            return;

        auto& collider = scene->getComponent<ColliderComponent>(id);

        ImGui::PushID(&collider);

        ColliderShape* shape = collider.GetColliderShape();
        std::string tabName;

        switch (shape->getType())
        {
        case ColliderType::BOX:
            tabName = "Box Collider";
            break;
        case ColliderType::SPHERE:
            tabName = "Sphere Collider";
            break;
        default:
            tabName = "Unknown Collider";
            break;
        }

        if (ImGui::CollapsingHeader(tabName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 center = shape->center;
            if (ImGui::DragFloat3("Center", &center[0], 0.1f))
            {
                shape->center = center;
            }

            switch (shape->getType())
            {
            case ColliderType::BOX:
                ImGui::DragFloat3("Half Size", &((BoxCollider*)shape)->halfSize[0], 0.1f, 0.0f, 1000.0f);
                break;
            case ColliderType::SPHERE:
                ImGui::DragFloat("Radius", &((SphereCollider*)shape)->radius, 0.1f, 0.0f, 1000.0f);
                break;
            default:
                break;
            }
        }


        ImGui::PopID();
	}



	void InspectorWindow::drawModel(const EditorContext& context, EntityID id)
	{
		auto& editor = context.editor;
		auto& scene = context.scene;
		auto& shaders = context.shaders;

        if (!scene->hasComponent<ModelComponent>(id))
            return;

        auto& modelComponent = scene->getComponent<ModelComponent>(id);
        ImGui::PushID(&modelComponent);

        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        {
            Shader* modelShader = modelComponent.shader;
            if (ImGui::BeginCombo("Shader##Combo", modelShader->getName().c_str()))
            {
                for (int i = 0; i < shaders.size(); i++)
                {
                    if (ImGui::Selectable(shaders[i]->getName().c_str(), modelShader == shaders[i]))
                    {
                        modelShader = shaders[i];
                        modelComponent.shader = modelShader;
                    }
                }

                ImGui::EndCombo();
            }
        }


        ImGui::PopID();
	}
}