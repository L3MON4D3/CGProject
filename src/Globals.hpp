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

const unsigned int material_binding = 0;
const size_t mat_sz = 4;
// ubo-indices.
const unsigned int mat_metal = 0;
const unsigned int mat_orange = 1;
const unsigned int mat_glass = 2;
const unsigned int mat_asteroid = 3;

// roughness and refractionIndex of material.
const float material_values[mat_sz][2] = {
	{.322, .092},
	{.032, .885},
	{.058, 0},
	{.356, 1}
};

// mat_metal-ubo is stored at mat_ubos[mat_metal]...
extern unsigned int mat2ubo[mat_sz];

const unsigned int cargo_A_ubos[4] {mat_metal, mat_glass, mat_orange, mat_metal};
const unsigned int asteroid_ubos[1] {mat_asteroid};

extern glm::mat4 cargo_A_laser_origin_left;
extern glm::mat4 cargo_A_laser_origin_right;

}
