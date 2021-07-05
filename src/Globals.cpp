#include "Globals.hpp"

namespace Globals {

unsigned int shaderProgramObj;
unsigned int shaderProgramCurve;

std::pair<unsigned int, std::pair<int, int>> shader_lights[light_size];

std::shared_ptr<geometry> cargo_A;
std::shared_ptr<geometry> laser_missile;
std::shared_ptr<geometry> sphere;

glm::mat4 cargo_A_laser_origin_left = glm::translate(glm::vec3(-0.604, 0.297, -0.399));
glm::mat4 cargo_A_laser_origin_right = glm::translate(glm::vec3(0.604, 0.297, -0.399));

}
