#pragma once

#include "mesh.hpp"
#include <memory>
#include <glm/gtx/transform.hpp>

namespace Globals {

extern unsigned int shaderProgramObj;
extern unsigned int shaderProgramCurve;

const size_t light_size = 1;
extern std::pair<unsigned int, std::pair<int, int>> shader_lights[light_size];

extern std::shared_ptr<std::vector<geometry>> cargo_A;
extern std::shared_ptr<geometry> laser_missile;
extern std::shared_ptr<geometry> sphere;

extern glm::mat4 cargo_A_laser_origin_left;
extern glm::mat4 cargo_A_laser_origin_right;

}
