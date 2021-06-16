#pragma once

#include <tinysplinecxx.h>
#include <vector>
#include <glm/vec3.hpp>
#include <mesh.hpp>

namespace util {
	const glm::vec3 up = glm::vec3(0,1,0);
	glm::vec3 std2glm(std::vector<double>);
	std::vector<geometry> load_scene_full_mesh(const char* filename, bool smooth);
	glm::vec3 gs1(glm::vec3 a, glm::vec3 b);
}
