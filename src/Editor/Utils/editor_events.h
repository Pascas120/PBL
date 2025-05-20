#pragma once

#include "ECS/EventSystem.h"
#include "glm/glm.hpp"

namespace Editor::Events
{
	struct CameraFocus final : public Event
	{
		glm::vec3 position;
	};
}