#pragma once

#include "mesh.hpp"
#include <memory>
#include <glm/gtx/transform.hpp>

namespace Globals {

const unsigned int shader_Obj = 0;
const unsigned int shader_Curve = 1;
const unsigned int shader_Emote = 2;
const unsigned int shader_Glass = 3;
const unsigned int shader_Exhaust = 4;

const unsigned int shaders_sz = 5;
extern unsigned int shaders[shaders_sz];

const unsigned int cargo_A_shaders[6] {shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Obj};
const unsigned int asteroid_shaders[1] {shader_Curve};

const size_t light_size = 1;
extern std::pair<unsigned int, std::pair<int, int>> shader_lights[light_size];

extern std::shared_ptr<std::vector<geometry>> cargo_A;
extern std::shared_ptr<geometry> laser_missile;
extern std::shared_ptr<geometry> sphere;

const unsigned int material_binding = 1;
const unsigned int transforms_binding = 2;
const unsigned int lighting_binding = 3;

extern unsigned int transform_ubo;
extern unsigned int lighting_ubo;

const size_t mat_sz = 6;
// ubo-indices.
const unsigned int mat_metal = 0;
const unsigned int mat_orange = 1;
const unsigned int mat_metal2 = 2;
const unsigned int mat_asteroid = 3;
const unsigned int mat_glass = 4;
const unsigned int mat_exhaust = 5;

// roughness and refractionIndex of material.
const float material_values[mat_sz][2] = {
	{.322, .092},
	{.032, .885},
	{.058, 0},
	{.356, 1},
	{0,1},
	{0,1}
};

// mat_metal-ubo is stored at mat_ubos[mat_metal]...
extern unsigned int mat2ubo[mat_sz];

const unsigned int cargo_A_ubos[6] {mat_metal, mat_metal2, mat_orange, mat_metal, mat_glass, mat_exhaust};
const unsigned int asteroid_ubos[1] {mat_asteroid};

extern glm::mat4 cargo_A_laser_origin_left;
extern glm::mat4 cargo_A_laser_origin_right;

}
