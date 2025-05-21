#include "HierarchyWindow.h"
#include "EditorApp.h"
#include "Utils/editor_utils.h"
#include "Utils/editor_events.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Serialization.h"

#include "spdlog/spdlog.h"

namespace Editor
{
    void HierarchyWindow::draw(const EditorContext& context)
    {
		drawWindow(context);
    }

	void HierarchyWindow::drawWindow(const EditorContext& context)
	{
        auto& scene = context.scene;

        constexpr float rightMargin = 40.0f;

        ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Hierarchy"))
        {
            if (ImGui::BeginChild("HierarchyScrollArea", ImVec2(0, 0),
                ImGuiChildFlags_None,
                ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings
            ))
            {
                EntityID root = scene->getSceneRootEntity();

                if (ImGui::BeginPopupContextWindow("Hierarchy Context Menu"))
                {
                    bool disabled;
                    if (ImGui::MenuItem("Add Object"))
                    {
                        EntityID newObject = scene->createEntity(root);
                        scene->getComponent<ObjectInfoComponent>(newObject).name = "New Object";
						editor->selectedObject = newObject;
                    }

                    ImGui::EndPopup();
                }

                drawNode(context, root);
            }
            ImGui::EndChild();
        }
        ImGui::End();
	}


    void HierarchyWindow::drawNode(const EditorContext& context, EntityID id)
    {
        auto& scene = context.scene;

        ImGui::PushID(id);

        std::string objName = scene->getComponent<ObjectInfoComponent>(id).name;
        std::string displayName = objName + "###objName";
        Transform& transform = scene->getComponent<Transform>(id);
		auto& ts = scene->getTransformSystem();


        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth 
            | ImGuiTreeNodeFlags_FramePadding;

        nodeFlags |= (transform.children.size() == 0) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen;
        if (id == editor->selectedObject)
            nodeFlags |= ImGuiTreeNodeFlags_Selected;


        // tree node start
        bool opened = ImGui::TreeNodeEx(displayName.c_str(), nodeFlags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            editor->selectedObject = id;
        }
		if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
		{
			//auto& camera = context.camera;
			//glm::vec3& pos = transform.translation;

			//camera.Position = pos - camera.Front * editor->camDistance;
            Events::CameraFocus event;
			event.position = transform.globalMatrix[3];
			editor->getEventSystem().triggerEvent(event);
		}

        // Context Menu
        bool deleteObject = false;
        if (ImGui::BeginPopupContextItem("Context Menu"))
        {
            bool disabled;

            if (ImGui::MenuItem("Add Child"))
            {
                EntityID newObject = scene->createEntity(id);
                //scene->getComponent<ObjectInfoComponent>(newObject).name = "New Object";
				Utils::setUniqueName(newObject, *scene, "New Object");
				editor->selectedObject = newObject;
            }

            if (id != scene->getSceneRootEntity())
            {
                if (ImGui::MenuItem("Copy"))
                {
                    json objectJson = Serialization::serializeObjects({ id }, *scene);
                    editor->clipboard.objectJson = objectJson;
                }

                if (ImGui::BeginMenu("Paste", !editor->clipboard.objectJson.empty()))
                {
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    {
                        if (!editor->clipboard.objectJson.empty())
                        {
                            json objectJson = editor->clipboard.objectJson;
                            auto pastedEntities = Serialization::deserializeObjects(objectJson, *scene,
                                transform.parent, { context.shaders, context.models, false });

                            int index = ts.getChildIndex(id);
                            for (auto& entity : pastedEntities)
                            {
                                if (scene->getComponent<Transform>(entity).parent == transform.parent)
                                {
                                    ts.setChildIndex(entity, index + 1);
                                    Utils::setUniqueName(entity, *scene);
                                }
                            }
                            editor->selectedObject = pastedEntities[0];
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("As Child"))
                    {
                        if (!editor->clipboard.objectJson.empty())
                        {
                            json objectJson = editor->clipboard.objectJson;
                            auto pastedEntities = Serialization::deserializeObjects(objectJson, *scene,
                                id, { context.shaders, context.models, false });

                            for (auto& entity : pastedEntities)
                            {
                                Utils::setUniqueName(entity, *scene);
                            }
                            editor->selectedObject = pastedEntities[0];
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Duplicate"))
                {
                    json objectJson = Serialization::serializeObjects({ id }, *scene);
                    auto pastedEntities = Serialization::deserializeObjects(objectJson, *scene,
                        transform.parent, { context.shaders, context.models, false });

                    for (auto& entity : pastedEntities)
                    {
                        if (scene->getComponent<Transform>(entity).parent == transform.parent)
                        {
                            ts.setChildIndex(entity, ts.getChildIndex(id) + 1);
                            Utils::setUniqueName(entity, *scene);
                        }
                    }
                    editor->selectedObject = pastedEntities[0];
                }

                if (ImGui::MenuItem("Delete"))
                {
                    deleteObject = true;
                }
            }

            ImGui::EndPopup();
        }


        // Drag and Drop
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(Payload::HIERARCHY_NODE, &id, sizeof(EntityID));
            ImGui::Text(objName.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(Payload::HIERARCHY_NODE))
            {
                EntityID draggedObj = *(EntityID*)payload->Data;
                scene->getTransformSystem().addChildKeepTransform(id, draggedObj);
            }
            ImGui::EndDragDropTarget();
        }

        if (opened)
        {
            if (transform.children.size() > 0)
            {
                for (int i = 0; i < transform.children.size(); i++)
                {
                    drawRearrangeTarget(context, id, i);
                    drawNode(context, transform.children[i]);
                }
                drawRearrangeTarget(context, id, transform.children.size());
            }

            ImGui::TreePop();
        }
        ImGui::PopID();

        if (deleteObject)
        {
            scene->destroyEntity(id);
			if (!scene->hasEntity(editor->selectedObject))
			{
				editor->selectedObject = (EntityID)-1;
			}
        }

    }


    void HierarchyWindow::drawRearrangeTarget(const EditorContext& context, EntityID id, int targetIndex)
    {
        auto& scene = context.scene;

        const ImGuiPayload* payload = ImGui::GetDragDropPayload();

        if (payload && payload->IsDataType(Payload::HIERARCHY_NODE))
        {
            float separatorThickness = ImGui::GetStyle().ItemSpacing.y;
            float cursorPosY = ImGui::GetCursorPosY();

            ImGui::SetCursorPosY(cursorPosY - separatorThickness);
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, separatorThickness);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(Payload::HIERARCHY_NODE))
                {
                    EntityID draggedObj = *(EntityID*)payload->Data;
                    auto& ts = scene->getTransformSystem();
                    ts.addChildKeepTransform(id, draggedObj);
                    ts.setChildIndex(draggedObj, targetIndex);

                }
                ImGui::EndDragDropTarget();
            }
            ImGui::SetCursorPosY(cursorPosY);
        }
    }

}
