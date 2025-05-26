#include "InspectorWindow.h"
#include "EditorApp.h"

#include "imgui.h"
#include "imgui_internal.h"

#define ADD_COMPONENT(T, name) \
    ImGui::BeginDisabled(scene->hasComponent<T>(editor->selectedObject)); \
    if (ImGui::Selectable(name)) { \
        scene->addComponent<T>(editor->selectedObject, {}); \
    } \
    ImGui::EndDisabled();

namespace Editor
{
    template<typename T>
    static bool componentContextMenu(const EditorContext& context, EntityID id)
    {
        if (ImGui::BeginPopupContextItem("Component Context Menu"))
        {
            if (ImGui::MenuItem("Delete"))
            {
                context.scene->removeComponent<T>(id);
                ImGui::EndPopup();
                ImGui::PopID();
                return true;
            }
            ImGui::EndPopup();
        }

        return false;
    }


    void InspectorWindow::draw(const EditorContext& context)
    {
        drawWindow(context);
    }

    void InspectorWindow::drawWindow(const EditorContext& context)
    {
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
                if (editor->selectedObject == scene->getSceneRootEntity())
                {
                    ImGui::End();
                    return;
                }

                ImGui::Separator();

                drawTransform(context, editor->selectedObject);
                drawCollider(context, editor->selectedObject);
                drawModel(context, editor->selectedObject);
                drawCamera(context, editor->selectedObject);
				drawPointLight(context, editor->selectedObject);
				drawDirectionalLight(context, editor->selectedObject);

                ImGui::Dummy(ImVec2(0, 10));

                if (ImGui::BeginCombo("##AddComponentCombo", "Add Component",
                    ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
                {
                    ImGui::BeginDisabled(scene->hasComponent<ModelComponent>(editor->selectedObject));
                    if (ImGui::Selectable("Model"))
                    {
                        scene->addComponent<ModelComponent>(editor->selectedObject, {
                            .shader = context.shaders[0],
                            .model = context.models[0]
                            });
                    }
                    ImGui::EndDisabled();

                    ImGui::BeginDisabled(scene->hasComponent<ColliderComponent>(editor->selectedObject));
                    if (ImGui::Selectable("Box Collider"))
                    {
                        scene->addComponent<ColliderComponent>(editor->selectedObject, ColliderComponent(ColliderType::BOX));
                    }
                    if (ImGui::Selectable("Sphere Collider"))
                    {
                        scene->addComponent<ColliderComponent>(editor->selectedObject, ColliderComponent(ColliderType::SPHERE));
                    }
                    ImGui::EndDisabled();

                    ADD_COMPONENT(CameraComponent, "Camera");
                    ADD_COMPONENT(PointLightComponent, "Point Light");
					ADD_COMPONENT(DirectionalLightComponent, "Directional Light");


                    ImGui::EndCombo();
                }
            }
        }
        ImGui::End();
    }


    void InspectorWindow::drawTransform(const EditorContext& context, EntityID id)
    {
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

        bool open = ImGui::CollapsingHeader(tabName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<ColliderComponent>(context, id)) return;


        if (open)
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
        auto& scene = context.scene;
        auto& shaders = context.shaders;

        if (!scene->hasComponent<ModelComponent>(id))
            return;

        auto& modelComponent = scene->getComponent<ModelComponent>(id);
        ImGui::PushID(&modelComponent);

        bool open = ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<ModelComponent>(context, id)) return;

        if (open)
        {
            Model* model = modelComponent.model;
            if (ImGui::BeginCombo("Model##Combo", model->directory.c_str()))
            {
                for (int i = 0; i < context.models.size(); i++)
                {
                    if (ImGui::Selectable(context.models[i]->directory.c_str(), model == context.models[i]))
                    {
                        model = context.models[i];
                        modelComponent.model = model;
                    }
                }
                ImGui::EndCombo();
            }

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

    void InspectorWindow::drawCamera(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<CameraComponent>(id))
            return;

        auto& cameraComponent = scene->getComponent<CameraComponent>(id);

        ImGui::PushID(&cameraComponent);

        bool open = ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<CameraComponent>(context, id)) return;
        if (open)
        {
            auto& camera = cameraComponent.camera;
            auto& frustum = camera.getFrustum();

            constexpr ImGuiComboFlags comboFlags = 0;// ImGuiComboFlags_WidthFitPreview;

            if (ImGui::BeginCombo("Projection",
                cameraComponent.projectionType == CameraComponent::ProjectionType::PERSPECTIVE ? "Perspective" : "Orthographic",
                comboFlags))
            {
                if (ImGui::Selectable("Perspective",
                    cameraComponent.projectionType == CameraComponent::ProjectionType::PERSPECTIVE))
                {
                    cameraComponent.projectionType = CameraComponent::ProjectionType::PERSPECTIVE;
                    cameraComponent.dirty = true;
                }
                if (ImGui::Selectable("Orthographic",
                    cameraComponent.projectionType == CameraComponent::ProjectionType::ORTHOGRAPHIC))
                {
                    cameraComponent.projectionType = CameraComponent::ProjectionType::ORTHOGRAPHIC;
                    cameraComponent.dirty = true;
                }
                ImGui::EndCombo();
            }

            ImGui::Indent();
            std::string fovSizeAxisName =
                cameraComponent.projectionType == CameraComponent::ProjectionType::PERSPECTIVE ?
                "Field of View Axis" : "Size Axis";
            if (ImGui::BeginCombo(fovSizeAxisName.c_str(),
                cameraComponent.fovSizeAxis == CameraComponent::FovSizeAxis::VERTICAL ? "Vertical" : "Horizontal",
                comboFlags))
            {
                if (ImGui::Selectable("Vertical",
                    cameraComponent.fovSizeAxis == CameraComponent::FovSizeAxis::VERTICAL))
                {
                    cameraComponent.fovSizeAxis = CameraComponent::FovSizeAxis::VERTICAL;
                    cameraComponent.dirty = true;
                }
                if (ImGui::Selectable("Horizontal",
                    cameraComponent.fovSizeAxis == CameraComponent::FovSizeAxis::HORIZONTAL))
                {
                    cameraComponent.fovSizeAxis = CameraComponent::FovSizeAxis::HORIZONTAL;
                    cameraComponent.dirty = true;
                }
                ImGui::EndCombo();
            }

            if (cameraComponent.projectionType == CameraComponent::ProjectionType::PERSPECTIVE)
            {
                float fovDeg = glm::degrees(cameraComponent.perspective.fov);
                if (ImGui::SliderFloat("Field of View", &fovDeg, 1e-05f, 179.9f, "%.2f"))
                {
                    cameraComponent.perspective.fov = glm::radians(fovDeg);
                    cameraComponent.dirty = true;
                }
            }
            else
            {
                cameraComponent.dirty |= ImGui::DragFloat("Size", &cameraComponent.orthographic.size, 0.1f);
            }

            cameraComponent.dirty |= ImGui::DragFloat2("Screen Offset", &cameraComponent.screenOffset[0],
                0.01f, -1.0f, 1.0f);

            ImGui::SeparatorText("Clipping Planes");
            cameraComponent.dirty |= ImGui::DragFloat("Near", &cameraComponent.nearPlane,
                0.01f, 0.01f, cameraComponent.farPlane - 0.01f);
            cameraComponent.dirty |= ImGui::DragFloat("Far", &cameraComponent.farPlane,
                0.01f, cameraComponent.nearPlane + 0.01f, 10000.0f);

            ImGui::Unindent();
        }

        ImGui::PopID();
    }

    void InspectorWindow::drawPointLight(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<PointLightComponent>(id))
            return;

        auto& light = scene->getComponent<PointLightComponent>(id);

        ImGui::PushID(&light);

        bool open = ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<PointLightComponent>(context, id)) return;
        if (open)
        {
            ImGui::ColorEdit3("Color", &light.color[0]);
			ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
			ImGui::Spacing();
            ImGui::DragFloat("Constant", &light.constant, 0.01f, 0.0f);
			ImGui::DragFloat("Linear", &light.linear, 0.01f, 0.0f);
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.01f, 0.0f);
        }
        ImGui::PopID();
    }

	void InspectorWindow::drawDirectionalLight(const EditorContext& context, EntityID id)
	{
		auto& scene = context.scene;
		if (!scene->hasComponent<DirectionalLightComponent>(id))
			return;

		auto& light = scene->getComponent<DirectionalLightComponent>(id);

		ImGui::PushID(&light);

		bool open = ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen);
		if (componentContextMenu<DirectionalLightComponent>(context, id)) return;
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.color[0]);
			ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
		}
		ImGui::PopID();
	}
}