#include "InspectorWindow.h"
#include "EditorApp.h"
#include "Utils/editor_utils.h"

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
				std::string tag = objectInfo.tag;
                const char* tagCStr = tag.c_str();

                if (ImGui::InputText("Tag", (char*)tagCStr, 64))
                {
                    objectInfo.tag = tagCStr;
                }

                ImGui::Separator();

                drawTransform(context, editor->selectedObject);
                drawCollider(context, editor->selectedObject);
                drawModel(context, editor->selectedObject);
                drawCamera(context, editor->selectedObject);
                drawPointLight(context, editor->selectedObject);
                drawDirectionalLight(context, editor->selectedObject);
				drawFlyAi(context, editor->selectedObject);
				drawVelocity(context, editor->selectedObject);
                drawHeat(context, editor->selectedObject);
                drawFreeze(context, editor->selectedObject);
                drawRegen(context, editor->selectedObject);
                drawElevator(context, editor->selectedObject);
                drawButton(context, editor->selectedObject);


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
					ADD_COMPONENT(FlyAIComponent, "Fly AI");
					ADD_COMPONENT(VelocityComponent, "Velocity");
                    ADD_COMPONENT(HeatComponent, "Heat");
                    ADD_COMPONENT(FreezeComponent, "Freeze");
                    ADD_COMPONENT(RegenComponent, "Regen");
                    ADD_COMPONENT(ElevatorComponent, "Elevator");
                    ADD_COMPONENT(ButtonComponent, "Button");

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

			ImGui::Checkbox("Static", &transform.isStatic);

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
			ImGui::Checkbox("Static", &collider.isStatic);
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
            if (ImGui::BeginCombo("Model##Combo", model->path.c_str()))
            {
                for (int i = 0; i < context.models.size(); i++)
                {
                    if (ImGui::Selectable(context.models[i]->path.c_str(), model == context.models[i]))
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

	void InspectorWindow::drawFlyAi(const EditorContext& context, EntityID id)
	{
		static const std::string stateNames[] = {
			"Patrolling",
			"Diving",
			"Returning"
		};

		auto& scene = context.scene;
		if (!scene->hasComponent<FlyAIComponent>(id))
			return;
		auto& flyAI = scene->getComponent<FlyAIComponent>(id);
		ImGui::PushID(&flyAI);
		bool open = ImGui::CollapsingHeader("Fly AI", ImGuiTreeNodeFlags_DefaultOpen);
		if (componentContextMenu<FlyAIComponent>(context, id)) return;
		if (open)
		{
			Utils::entityRefField("Butter Entity", flyAI.idButter, *scene);
			Utils::entityRefField("Bread Entity", flyAI.idBread, *scene);

            ImGui::DragFloat("Patrol Height Offset", &flyAI.patrolHeightOffset, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Patrol Speed", &flyAI.patrolSpeed, 0.1f, 0.0f);
            ImGui::DragFloat("Patrol Range", &flyAI.patrolRange, 0.1f, 1e-05f);
            ImGui::DragFloat("Patrol Point Reached Threshold", &flyAI.patrolPointReachedThreshold, 0.1f, 0.01f);

            ImGui::Separator();
			ImGui::DragFloat("Dive Speed", &flyAI.diveSpeed, 0.1f, 0.0f);
            ImGui::DragFloat("Detection Radius", &flyAI.detectionRadius, 0.1f, 0.0f);
            ImGui::DragFloat("Dive End Height", &flyAI.diveEndHeight, 0.1f, -100.0f, 100.0f);
            ImGui::DragFloat("Dive Cooldown", &flyAI.diveCooldownTime, 0.1f, 0.0f);

            ImGui::Separator();
			ImGui::DragFloat("Return Speed", &flyAI.returnSpeed, 0.1f, 0.0f);
			ImGui::DragFloat("Ground Y", &flyAI.groundY, 0.1f, -100.0f, 100.0f);
            if (ImGui::BeginCombo("State", stateNames[flyAI.state].c_str()))
            {
				for (int i = 0; i < 3; i++)
				{
					if (ImGui::Selectable(stateNames[i].c_str(), flyAI.state == i))
					{
						flyAI.state = (FlyAIComponent::FlyState)i;
					}
				}
				ImGui::EndCombo();
            }
			ImGui::DragFloat3("Patrol Target", &flyAI.patrolTarget[0], 0.1f, -100.0f, 100.0f);

		}
		ImGui::PopID();
	}

    void InspectorWindow::drawVelocity(const EditorContext& context, EntityID id)
    {
		auto& scene = context.scene;
		if (!scene->hasComponent<VelocityComponent>(id))
			return;
		auto& velocity = scene->getComponent<VelocityComponent>(id);
		ImGui::PushID(&velocity);
		bool open = ImGui::CollapsingHeader("Velocity", ImGuiTreeNodeFlags_DefaultOpen);
		if (componentContextMenu<VelocityComponent>(context, id)) return;
		if (open)
		{
			ImGui::DragFloat3("Linear", &velocity.velocity[0], 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat3("Angular", &velocity.angularVelocity[0], 0.1f, -100.0f, 100.0f);
			ImGui::Checkbox("Use Gravity", &velocity.useGravity);
		}
		ImGui::PopID();
    }
    void InspectorWindow::drawHeat(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<HeatComponent>(id))
            return;

        auto& heat = scene->getComponent<HeatComponent>(id);
        ImGui::PushID(&heat);

        bool open = ImGui::CollapsingHeader("Heat", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<HeatComponent>(context, id))  return;

        if (open)
        {
            ImGui::DragFloat("Trigger Radius", &heat.triggerRadius, 0.1f, 0.0f, 100.0f);
            ImGui::Checkbox("Triggered", &heat.hasTriggered);            
            char buf[64]; std::strncpy(buf, heat.OnEnterMessage.c_str(), sizeof(buf));
            if (ImGui::InputText("Message", buf, sizeof(buf)))
                heat.OnEnterMessage = buf;
        }
        ImGui::PopID();
    }
    void InspectorWindow::drawFreeze(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<FreezeComponent>(id))
            return;

        auto& freeze = scene->getComponent<FreezeComponent>(id);
        ImGui::PushID(&freeze);

        bool open = ImGui::CollapsingHeader("Freeze", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<FreezeComponent>(context, id)) return;

        if (open)
        {
            ImGui::DragFloat("Trigger Radius", &freeze.triggerRadius, 0.1f, 0.0f, 100.0f);
            ImGui::Checkbox("Triggered", &freeze.hasTriggered);             
            char buf[64]; std::strncpy(buf, freeze.OnEnterMessage.c_str(), sizeof(buf));
            if (ImGui::InputText("Message", buf, sizeof(buf)))
                freeze.OnEnterMessage = buf;
        }
        ImGui::PopID();
    }
    void InspectorWindow::drawRegen(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<RegenComponent>(id))
            return;

        auto& regen = scene->getComponent<RegenComponent>(id);
        ImGui::PushID(&regen);

        bool open = ImGui::CollapsingHeader("Regen", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<RegenComponent>(context, id)) return;

        if (open)
        {
            ImGui::DragFloat("Trigger Radius", &regen.triggerRadius, 0.1f, 0.0f, 100.0f);
            ImGui::Checkbox("Triggered", &regen.hasTriggered);            
            char buf[64]; std::strncpy(buf, regen.OnEnterMessage.c_str(), sizeof(buf));
            if (ImGui::InputText("Message", buf, sizeof(buf)))
                regen.OnEnterMessage = buf;
        }
        ImGui::PopID();
    }
    void InspectorWindow::drawElevator(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<ElevatorComponent>(id)) return;
        auto& e = scene->getComponent<ElevatorComponent>(id);

        ImGui::PushID(&e);
        bool open = ImGui::CollapsingHeader("Elevator", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<ElevatorComponent>(context, id)) return;
        if (open)
        {
            ImGui::DragFloat("Open Height", &e.openHeight, 0.1f);
            ImGui::DragFloat("Speed", &e.speed, 0.1f);
            Utils::entityRefField("Button Entity", e.buttonEntity, *scene);
            
            ImGui::Separator();
            ImGui::Checkbox("Door", &e.isDoor);
            if (e.isDoor) {
                const char* dirs[] = { "L", "R" };
                int dir = (int)e.doorDir;
                if (ImGui::Combo("Direction", &dir, dirs, 2))
                     e.doorDir = ElevatorComponent::DoorDir(dir);
                ImGui::Checkbox("Lock", &e.locked);
                
            }

            ImGui::Text("Is Moving: %s", e.isMoving ? "Yes" : "No");


        }
        ImGui::PopID();
    }




    void InspectorWindow::drawButton(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;
        if (!scene->hasComponent<ButtonComponent>(id)) return;
        auto& b = scene->getComponent<ButtonComponent>(id);

        ImGui::PushID(&b);
        bool open = ImGui::CollapsingHeader("Button", ImGuiTreeNodeFlags_DefaultOpen);
        if (componentContextMenu<ButtonComponent>(context, id)) return;
        if (open)
        {
            ImGui::DragFloat("Press Depth", &b.pressDepth, 0.01f);
            ImGui::DragFloat("Press Speed", &b.pressSpeed, 0.1f);

			
			std::string playerTag = b.playerTag;
			const char* playerTagCStr = playerTag.c_str();
			if (ImGui::InputText("Player Tag", (char*)playerTagCStr, 64))
			{
				b.playerTag = playerTagCStr;
			}

            
            Utils::entityRefField("Elevator Entity", b.elevatorEntity, *scene);
        }
        ImGui::PopID();
    }





}