#include "glm_serialization.h"

using json = nlohmann::json;

namespace glm
{
	void to_json(nlohmann::json& j, const glm::vec2& v)
	{
		j = { v.x, v.y };
	}

	void from_json(const nlohmann::json& j, glm::vec2& v)
	{
		j.at(0).get_to(v.x);
		j.at(1).get_to(v.y);
	}

	void to_json(nlohmann::json& j, const glm::vec3& v)
	{
		j = { v.x, v.y, v.z };
	}

	void from_json(const nlohmann::json& j, glm::vec3& v)
	{
		j.at(0).get_to(v.x);
		j.at(1).get_to(v.y);
		j.at(2).get_to(v.z);
	}

	void to_json(nlohmann::json& j, const glm::vec4& v)
	{
		j = { v.x, v.y, v.z, v.w };
	}

	void from_json(const nlohmann::json& j, glm::vec4& v)
	{
		j.at(0).get_to(v.x);
		j.at(1).get_to(v.y);
		j.at(2).get_to(v.z);
		j.at(3).get_to(v.w);
	}

	void to_json(nlohmann::json& j, const glm::quat& q)
	{
		j = { q.x, q.y, q.z, q.w };
	}

	void from_json(const nlohmann::json& j, glm::quat& q)
	{
		j.at(0).get_to(q.x);
		j.at(1).get_to(q.y);
		j.at(2).get_to(q.z);
		j.at(3).get_to(q.w);
	}

	void to_json(nlohmann::json& j, const glm::mat2& m)
	{
		j = json{};
		for (int col = 0; col < 2; ++col)
			for (int row = 0; row < 2; ++row)
				j.push_back(m[col][row]);
	}

	void from_json(const nlohmann::json& j, glm::mat2& m)
	{
		for (int col = 0; col < 2; ++col)
			for (int row = 0; row < 2; ++row)
				j.at(col * 2 + row).get_to(m[col][row]);
	}

	void to_json(nlohmann::json& j, const glm::mat3& m)
	{
		j = json{};
		for (int col = 0; col < 3; ++col)
			for (int row = 0; row < 3; ++row)
				j.push_back(m[col][row]);
	}

	void from_json(const nlohmann::json& j, glm::mat3& m)
	{
		for (int col = 0; col < 3; ++col)
			for (int row = 0; row < 3; ++row)
				j.at(col * 3 + row).get_to(m[col][row]);
	}

	void to_json(nlohmann::json& j, const glm::mat4& m)
	{
		j = json{};
		for (int col = 0; col < 4; ++col)
			for (int row = 0; row < 4; ++row)
				j.push_back(m[col][row]);
	}

	void from_json(const nlohmann::json& j, glm::mat4& m)
	{
		for (int col = 0; col < 4; ++col)
			for (int row = 0; row < 4; ++row)
				j.at(col * 4 + row).get_to(m[col][row]);
	}
}