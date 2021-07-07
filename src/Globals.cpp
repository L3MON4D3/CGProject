#include "Globals.hpp"

namespace Globals {

unsigned int shaders[shaders_sz];

std::pair<unsigned int, std::pair<int, int>> shader_lights[light_size];

std::shared_ptr<std::vector<geometry>> cargo_A;
std::shared_ptr<geometry> laser_missile;
std::shared_ptr<geometry> sphere;

unsigned int transform_ubo;
unsigned int lighting_ubo;

unsigned int mat2ubo[mat_sz];

glm::mat4 cargo_A_laser_origin_left = glm::translate(glm::vec3(-0.604, 0.297, -0.399));
glm::mat4 cargo_A_laser_origin_right = glm::translate(glm::vec3(0.604, 0.297, -0.399));

}
