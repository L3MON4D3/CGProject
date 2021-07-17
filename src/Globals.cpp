#include "Globals.hpp"

namespace Globals {

unsigned int shaders[shaders_sz];

std::pair<unsigned int, std::pair<int, int>> shader_lights[light_size];

std::shared_ptr<std::vector<geometry>> cargo_A;
std::shared_ptr<std::vector<geometry>> pirate;
std::shared_ptr<std::vector<geometry>> station;
std::shared_ptr<geometry> laser_missile;
std::shared_ptr<geometry> sphere;
std::shared_ptr<std::vector<geometry>> asteroids;

unsigned int transform_ubo;
unsigned int lighting_ubo;
unsigned int particle_texture;
unsigned int skybox;

glm::mat4 proj;

unsigned int mat2ubo[mat_sz];
}
