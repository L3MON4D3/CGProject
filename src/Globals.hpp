#pragma once

#include "mesh.hpp"
#include <memory>
#include <glm/gtx/transform.hpp>

namespace Globals {

const unsigned int shader_Obj = 0;
const unsigned int shader_Curve = 1;
const unsigned int shader_Quad = 2;
const unsigned int shader_Glass = 3;
const unsigned int shader_Exhaust = 4;
const unsigned int shader_Light = 5;
const unsigned int shader_Particle = 6;
const unsigned int shader_Skybox = 7;
const unsigned int shader_Asteroids = 8;
const unsigned int shader_Turbine = 9;

const unsigned int shaders_sz = 10;
extern unsigned int shaders[shaders_sz];

const unsigned int cargo_A_shaders[6] {shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Glass, shader_Exhaust};
const unsigned int pirate_shaders[6] {shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Glass, shader_Exhaust};
const unsigned int station_shaders[7] {shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Obj, shader_Obj};
const unsigned int asteroid_shaders[1] {shader_Obj};

const size_t light_size = 1;
extern std::pair<unsigned int, std::pair<int, int>> shader_lights[light_size];

extern std::shared_ptr<std::vector<geometry>> cargo_A;
extern std::shared_ptr<std::vector<geometry>> pirate;
extern std::shared_ptr<std::vector<geometry>> station;
extern std::shared_ptr<geometry> laser_missile;
extern std::shared_ptr<geometry> sphere;

extern std::shared_ptr<std::vector<geometry>> asteroids;

extern glm::mat4 proj;

const unsigned int material_binding = 1;
const unsigned int transforms_binding = 2;
const unsigned int lighting_binding = 3;

extern unsigned int transform_ubo;
extern unsigned int lighting_ubo;
extern unsigned int particle_texture;
extern unsigned int skybox;

const size_t mat_sz = 7;
// ubo-indices.
const unsigned int mat_metal = 0;
const unsigned int mat_orange = 1;
const unsigned int mat_metal_dark = 2;
const unsigned int mat_asteroid = 3;
const unsigned int mat_glass = 4;
const unsigned int mat_exhaust = 5;
const unsigned int mat_dark = 6;
const std::string mat_names[7] = {"metal", "orange", "metal2", "asteroid", "glass", "exhaust", "dark"};

// roughness and refractionIndex of material.
const float material_values[mat_sz][2] = {
	{.485, .472},
	{.819, .713},
	{1, .884},
	{.356, 1},
	{0,1},
	{0,1},
	{.870, .232}
};

// mat_metal-ubo is stored at mat_ubos[mat_metal]...
extern unsigned int mat2ubo[mat_sz];

const unsigned int cargo_A_ubos[6] {mat_metal, mat_metal_dark, mat_orange, mat_dark, mat_glass, mat_exhaust};
const unsigned int pirate_ubos[6] {mat_metal, mat_metal_dark, mat_orange, mat_dark, mat_glass, mat_exhaust};
const unsigned int station_ubos[4] {mat_orange, mat_metal, mat_metal_dark, mat_dark};
const unsigned int asteroid_ubos[1] {mat_asteroid};

const glm::mat4 cargo_A_laser_origin_left = glm::translate(glm::vec3(-0.604, 0.297, -0.399));;
const glm::mat4 cargo_A_laser_origin_right = glm::translate(glm::vec3(0.604, 0.297, -0.399));
const glm::mat4 pirate_laser_origin_left = glm::translate(glm::vec3(-0.48, 0.25, 0));
const glm::mat4 pirate_laser_origin_right = glm::translate(glm::vec3(0.48, 0.25, 0));

const glm::vec3 cargo_A_turbine_right[4] = {
	glm::vec3{.75, .239, -1.125},
	glm::vec3{.75, .36, -1.125},
	glm::vec3{.45, .436, -1.125},
	glm::vec3{.45, .164, -1.125},
};

const glm::vec3 cargo_A_turbine_left[4] = {
	glm::vec3{-.45, .164, -1.125},
	glm::vec3{-.45, .436, -1.125},
	glm::vec3{-.75, .36, -1.125},
	glm::vec3{-.75, .239, -1.125},
};

}
